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

#ifndef __BOOKLET_H__
#define __BOOKLET_H__

#include <string>
#include <boost/thread.hpp>
#include <cairo.h>
#include <vector>
#include <iostream>
#include <curl/curl.h>

class Airport;
class Cycle;
class Chart;

class Booklet {
	friend std::ostream &operator<<(std::ostream &stream, const Booklet &bk );
	friend class Chart;

	Cycle *cycle;
	boost::shared_ptr<boost::thread> thread;
	boost::mutex mutex;
	bool stop_requested;
	void do_work();
	cairo_surface_t *surface;
	cairo_status_t cairo_status;
	double width, height;

	typedef std::vector<int> wanted_t;
	wanted_t wanted;

	typedef std::vector<int> sort_order_t;
	sort_order_t sort_order;

	typedef int ChartType;
public:
	enum State {
		PENDING,
		FETCHING,
		RENDERING,
		READY
	};
	typedef std::vector<Airport*> airports_t;
	typedef std::vector<std::string> not_found_t;
	typedef std::vector<std::string> progress_log_t;

	Booklet( Cycle*, double width, double height );


	progress_log_t progress();
	void log_msg( const std::string &msg );
	void log_replace_last_msg( const std::string &new_msg );


	bool enable_chart_type( ChartType t, int sort_order );
	int get_sort_order( ChartType t ) const;
	bool is_wanted( ChartType t ) const;

	airports_t airports;
	not_found_t not_found;

	void start(); 
	void stop();
	State status();
	std::string status_msg();
	
	~Booklet();

	typedef std::vector<char> buffer_t;
	std::string suggested_filename;
	size_t raw_data( const char **data );
private:
	State state;
	buffer_t buffer;
	progress_log_t progress_log;
	CURL *curl;
};


#endif // _BOOKLET_H__
