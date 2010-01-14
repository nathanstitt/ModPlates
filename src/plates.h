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


#ifndef __PLATES_H__
#define __PLATES_H__

#include <string>
#include <boost/filesystem.hpp>
#include <curl/curl.h>
#include "cycle.h"
#include "booklet.h"
#include "mod_plates.h"
#include "logging.h"



class Plates {

public:
	typedef boost::shared_ptr<Booklet> booklet_t;
	typedef boost::unordered_map<unsigned int,booklet_t > running_t;

	Plates( const std::string &dir_name, boost::logging::mask_t log_mask );

	static Plates* config( const std::string &dir_name );
	static Plates* instance();
	booklet_key_t start_booklet( const std::string &airports, char** charts, int num_charts  );
	unsigned int evict();
	booklet_t get_booklet( booklet_key_t key );
	Cycle *current_cycle;

	~Plates();

private:
	const boost::filesystem::path directory;
	running_t running;
	boost::mutex mutex;
	unsigned int current_timestamp();
	boost::logging::sink sink;
};


#endif // _PLATES_H__
