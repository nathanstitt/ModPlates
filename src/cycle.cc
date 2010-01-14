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


#include "cycle.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <functional>
#include <algorithm>
#include <cctype>

#include "airport.h"
#include "chart.h"
#include "booklet.h"
#include "logging.h"

namespace fs = boost::filesystem;

typedef boost::unordered_map<std::string,Airport*> map_t;
bool tracking = false;


Cycle::Cycle( const boost::filesystem::path &dir_name  ) : directory( dir_name ){
	xmlTextReaderPtr reader;
	int ret;
	LOG_INFO("Cycle Using Directory " << ( directory / "cycle.xml" ) );
	reader = xmlNewTextReaderFilename( ( directory / "cycle.xml" ).string().c_str() );
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while ( ret == 1 ) {
			this->process_node(reader);
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
	} else {
		LOG_ERROR("Failed to open or parse cycle.xml") 
	}
}



Cycle*
Cycle::Import( const boost::filesystem::path &dir_name ){
	Cycle *cycle = new Cycle( dir_name );
	if ( ! fs::exists( cycle->directory / cycle->id  ) ) {
		fs::create_directory( cycle->directory / cycle->id );
	}
	if ( fs::exists( cycle->directory / cycle->id / "cycle.xml" ) ) {
		fs::remove( cycle->directory / cycle->id  / "cycle.xml" );
	}
	fs::copy_file( cycle->directory / "cycle.xml", cycle->directory / cycle->id / "cycle.xml" );
	
	*(const_cast<fs::path*>( &(cycle->directory) )) /= cycle->id;
	LOG_INFO( "Imported: " << cycle->directory );
	return cycle;
}

struct LatestDir {
	fs::path path;
	time_t lwrite;
};

Cycle *
Cycle::Latest( const boost::filesystem::path &root_dir ){
	fs::path path(  root_dir );
	fs::directory_iterator end_iter;
	LatestDir ld;
	ld.lwrite = 0;

	for ( fs::directory_iterator dir_itr( path ); dir_itr != end_iter; ++dir_itr ){
		if ( fs::is_directory( *dir_itr ) && fs::is_regular_file( fs::path(*dir_itr) / "cycle.xml" ) ){
			LOG_INFO("Looking in directory " << *dir_itr );
			if ( fs::last_write_time( *dir_itr ) > ld.lwrite ){
				ld.lwrite = fs::last_write_time( *dir_itr );
				ld.path = *dir_itr;
			}
		}
	}
	if ( ld.lwrite ){
		return new Cycle( ld.path );
	} else {
		LOG_ERROR("Failed to find directory containing cycle.xml");
		return NULL;
	}
}

void
Cycle::process_node(xmlTextReaderPtr reader) {
	const xmlChar *name;

	name = xmlTextReaderConstName(reader);
	if ( id.empty() && ! xmlStrcmp( name, BAD_CAST "digital_tpp" ) ) {
		const_cast<std::string*>( &id )->assign( (const char*) xmlTextReaderGetAttribute( reader, BAD_CAST "cycle" ) );
	}
	
	if ( ! xmlStrcmp( name, BAD_CAST "airport_name" ) )  {
		if ( xmlTextReaderNodeType(reader) == 1 ){
			xmlChar *code = xmlTextReaderGetAttribute( reader, BAD_CAST "apt_ident" );
			if ( code ){
				current_airport = new Airport( (const char*) code );
				xmlFree( code );
				map[ current_airport->identifier ] = current_airport;
			}
		}
	}

	if ( current_airport && ! xmlStrcmp( name, BAD_CAST "record" ) && xmlTextReaderNodeType(reader) == 1 ){
		xmlNodePtr node = xmlTextReaderExpand( reader );
		if ( node ){
			Chart *chart = current_airport->add_chart();
			chart->cycle = this;
			for(xmlNodePtr cur_node = node->children; cur_node != NULL; cur_node = cur_node->next){
				xmlChar *val = xmlNodeGetContent( cur_node );
				chart->set_property( reinterpret_cast<const char*>( cur_node->name ), 
						     reinterpret_cast<const char*>( val ) );
				xmlFree( val );
			}
		}
	}
}

void
Cycle::add_airport_to_booklet( boost::shared_ptr<Booklet> &bk, const std::string &id ){

	std::string upper_id(id);
	std::transform(upper_id.begin(), upper_id.end(), upper_id.begin(), toupper );

	LOG_DEBUG( "Attempting airport " << upper_id );

	map_t::iterator airport = map.find( upper_id );
	if ( airport == map.end() ){
		bk->not_found.push_back( upper_id );
	} else {
		bk->airports.push_back( airport->second );
	}
}

boost::shared_ptr<Booklet>
Cycle::make_booklet( const std::string &codes ){
      boost::regex re("\\W+");

      boost::shared_ptr<Booklet> bk( new Booklet( this, 792.0,612.0 ) ); 

      std::for_each( boost::sregex_token_iterator(codes.begin(), codes.end(), re, -1),
		     boost::sregex_token_iterator(),
		     boost::bind( &Cycle::add_airport_to_booklet, this, bk, _1 ) 
	      );
     
      return  bk;
}

Cycle::~Cycle(){
	for( map_t::iterator pair = map.begin(); pair != map.end(); ++pair ){
		delete pair->second;
	}
}



