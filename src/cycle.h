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

#ifndef __CYCLE_H__
#define __CYCLE_H__

#include <string>
#include <boost/unordered_map.hpp>  
#include <boost/filesystem.hpp>
#include <libxml/xmlreader.h>
#include <boost/shared_ptr.hpp>

class Airport;
class Booklet;

class Cycle {
	typedef boost::unordered_map<std::string,Airport*> map_t;
	map_t map;
	Airport *current_airport;
	void process_node(xmlTextReaderPtr reader);

	void add_airport_to_booklet(  boost::shared_ptr<Booklet> &bk , const std::string &id );
	
public:
	Cycle( const boost::filesystem::path &dir );
	static Cycle *Latest( const boost::filesystem::path &root_dir );
	static Cycle *Import( const boost::filesystem::path &dir_name );

	const std::string id;

	const boost::filesystem::path directory;

	boost::shared_ptr<Booklet> make_booklet( const std::string &codes );

	~Cycle();
};


#endif // _CYCLE_H__
