/* Copyright 2010 Nathan Stitt

   This file is part of mod_plates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 3 of the License.
 
   mod_plates is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details. 

   You should have received a copy of the GNU General Public License along
   with mod_plates. If not, see http://www.gnu.org/licenses/.
*/

#include "plates.h"
#include <poppler.h>
#include "chart.h"

Plates *_instance = NULL;


Plates::Plates( const std::string &dir_name, boost::logging::mask_t log_mask ) :
	directory( dir_name ),
	sink(&std::cout, log_mask )
{
	curl_global_init(CURL_GLOBAL_ALL);
	g_type_init ();

	LOG_INIT( (boost::logging::trace >> boost::logging::eol) ); //log format

	sink.attach_qualifier(boost::logging::log);

	LOG_ADD_OUTPUT_STREAM(sink);

	LOG_INFO( "Starting up Plates" );
	current_cycle = Cycle::Latest( directory.string() );
}


Plates::~Plates(){
	delete current_cycle;
}


void
create_instance( const std::string &dir_name ){
	_instance=new Plates( dir_name, LOG_MASK_LEVEL_DEBUG );
}

Plates*
Plates::instance(){
	return _instance;
}



boost::once_flag once = BOOST_ONCE_INIT;

Plates*
Plates::config( const std::string &dir_name ){
        boost::call_once( once, boost::bind(create_instance, dir_name ) );
	return _instance;
}


unsigned int 
Plates::current_timestamp(){
	return time(NULL) - 1000000000;
}

boost::shared_ptr<Booklet>
Plates::get_booklet( booklet_key_t key ){
	boost::mutex::scoped_lock l(mutex);
	running_t::const_iterator bk = running.find( key );
	if ( bk == running.end() ){
		return boost::shared_ptr<Booklet>();
	} else {
		return bk->second;
	}
}

unsigned int
Plates::start_booklet( const std::string &airports, char** charts, int num_charts ){
	boost::mutex::scoped_lock l(mutex);
	if ( running.size() > MAX_RUNNING && ! this->evict() ){
		LOG_ERROR( running.size() << " booklets are running, failed to evict any" );
		return 0;
	}
	LOG_ERROR( "Number of chart types: " << num_charts );
	boost::shared_ptr<Booklet> booklet=this->current_cycle->make_booklet( airports );
	for ( int i = 0; i< num_charts; i++ ){
		LOG_DEBUG( "Chart: " << charts[i] );
		booklet->enable_chart_type( Chart::str_to_type(charts[i]), i );
	}

	booklet->start();
	
	unsigned int now = current_timestamp();
	while ( now && ! running.emplace( now, booklet ).second ){
	 	now -= 1;
	}
	LOG_INFO( "Starting new booklet for " << airports << " key=" << now );

	return now;
}

unsigned int
Plates::evict(){
	unsigned int removed = 0;
	unsigned int now = current_timestamp();
	boost::mutex::scoped_lock l(mutex);
	for ( running_t::const_iterator bk = running.begin(); bk != running.end(); bk++ ){
		if ( bk->first < ( now - KEEP_MINUTES*60 ) ){
			running.erase( bk );
			removed++;
		}
	}
	return removed;
}

void
plates_config( const char *directory ){
	Plates::config( directory );
}

booklet_key_t
plates_to_key( const char *key ){
	return boost::lexical_cast<booklet_key_t>(key);
}

unsigned int
plates_start_booklet( const char *airports, char** charts, int num_charts  ){
	return Plates::instance()->start_booklet( airports,charts,num_charts );
}


const char *
plates_status( booklet_key_t key ){
	Plates::booklet_t bk = Plates::instance()->get_booklet( key );
	if ( bk ){
		return bk->status_msg().c_str();
	} else {
		return "booklet not found";
	}
}

unsigned short int
plates_report_status( booklet_key_t key, plates_callback_t cb, void *cbarg ){
	Plates::booklet_t bk = Plates::instance()->get_booklet( key );
	unsigned short int count = 0;
	if ( bk ){
		Booklet::progress_log_t log = bk->progress();
		for ( Booklet::progress_log_t::const_iterator msg = log.begin(); msg != log.end(); msg++ ){
			cb( count++, msg->c_str(), cbarg );		
		}
	}
	return count;
}

unsigned short int
plates_report_not_found( booklet_key_t key, plates_callback_t cb, void *cbarg ){
	Plates::booklet_t bk = Plates::instance()->get_booklet( key );
	unsigned short int count = 0;
	if ( bk ){
		for ( Booklet::not_found_t::const_iterator nf = bk->not_found.begin(); nf != bk->not_found.end(); nf++ ){
			cb( count++, nf->c_str(), cbarg );
		}
	}
	return count;
}

size_t
plates_booklet_data( booklet_key_t key, const char **data, const char **suggested_filename  ){
	Plates::booklet_t bk = Plates::instance()->get_booklet( key );
	if ( ! bk ){
		return 0;
	}
	*suggested_filename = bk->suggested_filename.c_str();
	return bk->raw_data( data );
}

void
plates_destroy(){
	LOG_INFO( "destroy plates" );
	delete _instance;
	_instance = NULL;

}
