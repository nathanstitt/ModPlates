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

#ifndef __CHART_H__
#define __CHART_H__

#include <string>
#include <cairo.h>
#include "booklet.h"

class Cycle;
class Airport;

class Chart {
	Airport *airport;
public:
	const std::string pdf;
	const std::string name;
	Cycle *cycle;
	
	enum State {
		UNFETCHED,
		FETCHING,
		READY
	};
	enum Type {
		APD,
		IAP,
		MIN,
		DP,
		DPO,
		STAR,
		CVFP,
		UNK
	};

	Chart( Airport *airport );

	bool layout_contents( Booklet *booklet, cairo_t *cr ) const;

	bool layout_identity(Booklet *booklet, cairo_t *cr ) const;

	bool fetch_if_unfresh( Booklet *booklet );
	bool fetch( Booklet *booklet );
	State state() const;
	std::string to_s() const;
	
 	bool set_property( const std::string& name, const std::string& value );
	
	const Type type;
	std::string str_type() const;
	static std::string type_to_str( Type t );
	static Type str_to_type( const std::string &type );

	std::string path() const;
	std::string working_path() const;

	unsigned short int runway() const;
	bool compare_identical( const Chart *other );
};


#endif // __CHART_H__
