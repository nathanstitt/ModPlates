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

#include "booklet.h"
#include "cycle.h"
#include "airport.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cairo/cairo-pdf.h>
#include "logging.h"
#include "chart.h"

Booklet::Booklet( Cycle *c, double w, double h ) : 
	cycle(c), 
	stop_requested( false ),
	surface( NULL ),
	sort_order( Chart::UNK+1 ),
	width( w ),
	height( h ),
	state( PENDING )
{
}


Booklet::~Booklet() {
	
}

void
Booklet::start(){
	assert( ! thread );
	std::cout << "Rendering " << airports.size() << std::endl;

	thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Booklet::do_work, this)));

}

void
Booklet::stop(){
	assert(thread);
        stop_requested = true;
        thread->join();
}

Booklet::State
Booklet::status(){
	boost::mutex::scoped_lock l(mutex);
	return state;
}

std::string
Booklet::status_msg(){
	switch ( this->status() ){
	case PENDING:
		return "PENDING";
	case FETCHING:
		return "FETCHING";
	case RENDERING:
		return "RENDERING";
	case READY:
		return "READY";
	default:
		return "UNKOWN";
	}
}


static cairo_status_t 
_write_callback(void *closure, const unsigned char *data, unsigned int length){
	Booklet::buffer_t* buffer = (Booklet::buffer_t*) closure;
	buffer->reserve( buffer->size() + length );
	std::copy( data, data+length, std::back_inserter(*buffer) );
	return CAIRO_STATUS_SUCCESS;
}



void
Booklet::do_work(){
	LOG_DEBUG( "Booklet starting do_work()" );

	curl = curl_easy_init();
	if ( ! curl ){
		this->log_msg("Failed to initialize HTTP transport");
		curl_easy_cleanup(curl);
		return;
	}
	curl_easy_setopt( curl, CURLOPT_USERAGENT, "www.stitt.org/plates" );

//	curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L);
//	curl_easy_setopt( curl, CURLOPT_HEADER, 1L);


	{
		boost::mutex::scoped_lock l(mutex);

		assert( ! surface );

		progress_log.clear();

		surface = cairo_pdf_surface_create_for_stream( _write_callback, &buffer, width, height );

		state = RENDERING;
		suggested_filename.clear();
		for ( airports_t::iterator airport = airports.begin(); 
		      airport != airports.end() && ! stop_requested; 
		      ++airport )
		{
			if ( suggested_filename.length() ){
				suggested_filename += "-";
			}
			suggested_filename += (*airport)->identifier;
		}
		suggested_filename += ".pdf";

	}
	

	for ( airports_t::iterator airport = airports.begin(); 
	      airport != airports.end() && ! stop_requested; 
	      ++airport )
	{
		(*airport)->bind( this, surface );
	}

	{
		boost::mutex::scoped_lock l(mutex);
		cairo_surface_finish (surface);
		cairo_status = cairo_surface_status(surface);
		if (cairo_status)
			LOG_ERROR( "Cairo Error: " << cairo_status_to_string (cairo_status) );

		cairo_surface_destroy (surface);
		state = READY;
	}
	curl_easy_cleanup(curl);
	this->log_msg("Completed");
	LOG_DEBUG( "Booklet Done Work" );
}


bool
Booklet::enable_chart_type( ChartType t, int order ){
	boost::mutex::scoped_lock l(mutex);
	sort_order[ t ] = order+1;
	return true;
}

bool
Booklet::is_wanted( ChartType t ) const {
	int ord = this->get_sort_order(t);
	LOG_DEBUG("IS WANTED? " << Chart::type_to_str((Chart::Type)t) << " : " << ord );
	return this->get_sort_order(t);
}

int
Booklet::get_sort_order( ChartType t ) const {
	LOG_DEBUG("Getting sort order for " << t << " sz: " << sort_order.size() );
	return sort_order[ t ];
}



Booklet::progress_log_t
Booklet::progress() {
	boost::mutex::scoped_lock l(mutex);
	return Booklet::progress_log_t( progress_log );
}

void
Booklet::log_msg( const std::string &msg ){
	boost::mutex::scoped_lock l(mutex);
	progress_log.push_back( msg );
}

void
Booklet::log_replace_last_msg( const std::string &msg ){
	boost::mutex::scoped_lock l(mutex);
	progress_log.pop_back();
	progress_log.push_back( msg );
}

size_t
Booklet::raw_data( const char **data ){
	size_t ret = buffer.size();
	if ( ret ){
		*data = &( buffer[ 0 ] );
	}
	return ret;
}

std::ostream &
operator<<(std::ostream &stream, const Booklet &bk) {
	std::copy( bk.buffer.begin(), bk.buffer.end(), std::ostream_iterator<char>( stream ) );
	return stream;
}
