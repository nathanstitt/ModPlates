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

#ifndef __AIRPORT_H__
#define __AIRPORT_H__

#include <string>
#include <vector>
#include <cairo.h>
#include "booklet.h"

class Chart;

class Airport {
	typedef std::vector<Chart*> charts_t;
	charts_t charts;
public:

	Airport( const std::string &identifier );

	const std::string identifier;

	Chart* add_chart();

	bool bind( Booklet *booklet, cairo_surface_t *surface ) const;

	bool layout_blank( Booklet *booklet, cairo_t *cr ) const;

	~Airport();
};


#endif // _AIRPORT_H__
