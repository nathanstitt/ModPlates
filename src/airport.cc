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

#include "airport.h"
#include "chart.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>
#include "logging.h"
#include <cctype>
#include <math.h>

Airport::Airport( const std::string &i ) : identifier(i ){

	
}

Chart*
Airport::add_chart(){
	Chart *prc = new Chart( this );
	charts.push_back( prc );
	return prc;
}




bool compare_charts (Chart* first, Chart *second, Booklet *bk )
{
	int fs = bk->get_sort_order( first->type );
	int ss = bk->get_sort_order( second->type );
	LOG_INFO( "Compare : " << first->to_s() << " ~ " << second->to_s() << " = " << fs << " < " << ss );

	if (  fs == ss ){
		return first->compare_identical( second );
	} else if ( fs < ss ){
		return true;
	} else {
		return false;
	}
}

bool
Airport::layout_blank( Booklet *booklet, cairo_t *cr ) const {
	cairo_save (cr);

	cairo_scale( cr, (float)( booklet->width / 2 ) / 100, (float)booklet->height / 100 );

	for ( int y = 10; y < 100; y+=5 ){
		cairo_move_to(cr, 5, y );
		cairo_line_to(cr, 95, y);
		cairo_set_line_width (cr, 0.15);
		cairo_stroke (cr);
	}

	cairo_restore(cr);

	return true;
}


bool
Airport::bind( Booklet *booklet, cairo_surface_t *surface ) const {

	cairo_t *cr = cairo_create(surface);


	LOG_DEBUG( "Binding " << identifier << " - " << charts.size() << " charts" );
	size_t num_charts = 0;

	charts_t wanted;

	for ( charts_t::const_iterator chart = charts.begin(); chart != charts.end(); ++chart ){
		if ( booklet->is_wanted( (*chart)->type ) ){
			wanted.push_back( *chart );
		}
	} 
	std::sort( wanted.begin(), wanted.end(), boost::bind( &compare_charts, _1, _2, booklet ) );

	num_charts = wanted.size();

	size_t num_pages = num_charts / 2 ; //ceil( (float)num_charts / 2 );

	for ( charts_t::const_iterator chart = wanted.begin(); chart != wanted.end(); ++chart ){
		LOG_DEBUG("Chart " << (*chart)->str_type() << " : " << (*chart)->to_s() );
	}
	LOG_DEBUG( num_charts << " charts" );

	LOG_DEBUG("Need at least " << num_pages << " pages" );

	while ( num_pages % 2 ){
		num_pages++;
	}

	LOG_DEBUG("Rounding up to " << num_pages << " pages" );

	size_t chart_num=0;

	LOG_DEBUG( "------------" );

	for ( size_t curpage = 0; curpage < num_pages; ++curpage ){

		LOG_DEBUG( "Page: " << curpage );

		if ( curpage % 2 ) {
			chart_num = curpage;
		} else {
			chart_num = ( (num_pages*2) - curpage ) - 1;
		}

		cairo_save (cr);

		if ( wanted.size() > chart_num ){
			wanted[ chart_num ]->layout_contents( booklet, cr );
		} else {
			this->layout_blank( booklet, cr );
		}


		std::string pg("Pg ");
		pg += boost::lexical_cast<std::string>(chart_num);
		cairo_move_to(cr, 20,10 );
		cairo_show_text(cr, pg.c_str() ); 

		cairo_restore (cr);


		if ( curpage % 2 ) {
			chart_num = ( (num_pages*2) - curpage ) - 1;
		} else {
			chart_num = curpage;
		}

cairo_save (cr);
		cairo_move_to(cr, booklet->width/2, 0 );
		cairo_set_source_rgb (cr, 0.8, 0.8, 0.8 );
		cairo_set_line_width( cr, 0.5 );
		cairo_line_to(cr, booklet->width/2, booklet->height );

		cairo_stroke (cr);
cairo_restore (cr);
		cairo_save (cr);


		cairo_translate (cr, booklet->width/2, 0 );

		if ( wanted.size() > chart_num ){
			wanted[ chart_num ]->layout_contents( booklet, cr );
		} else {
			this->layout_blank( booklet, cr );
		}

		pg="Pg ";
		pg += boost::lexical_cast<std::string>(chart_num);
		cairo_move_to(cr, 300, 10 );
		cairo_show_text(cr, pg.c_str() ); 

		cairo_restore (cr);

		cairo_surface_show_page ( surface );
	}

	return true;
}


static void destroy( Chart *d ){
	delete d;
}


Airport::~Airport(){
	for_each (charts.begin(), charts.end(), destroy );
}

