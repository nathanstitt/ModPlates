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

#include <stdio.h>
#include <stdlib.h>


#include <iterator>
#include <iostream>
#include <fstream>
#include "plates.h"
#include <poppler.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include "chart.h"

int main(int argc, char* argv[]) {
	
	
	if ( argc < 4 ){
		std::cerr << "usage: " << argv[0] << " <storage directory> <output file> <airport ids>" << std::endl;
		return 1;
	}

	Plates plates( argv[1], LOG_MASK_LEVEL_DEBUG );


	Cycle *cycle = plates.current_cycle;


	boost::shared_ptr<Booklet> booklet=cycle->make_booklet( argv[3] );
	
//	booklet->enable_chart_type( Chart::str_to_type( "DP" ), 0 );
	booklet->enable_chart_type( Chart::str_to_type( "APD"), 1 );
	booklet->enable_chart_type( Chart::str_to_type( "IAP"), 2 );
//	booklet->enable_chart_type( Chart::str_to_type( "MIN"), 3 );

	booklet->start();

	if ( ! booklet->not_found.empty() ){
		std::cerr << "The following airport identifiers were not found: ";
		copy ( booklet->not_found.begin(), booklet->not_found.end(), std::ostream_iterator<std::string>(std::cerr," ") );
		std::cerr << std::endl;
	}

	while ( booklet->status() != Booklet::READY ){
		Booklet::progress_log_t log = booklet->progress();
//		std::cerr << "-----------------------------------------------------------" << std::endl;
//		std::copy ( log.begin(), log.end(), std::ostream_iterator<std::string>(std::cerr,"\n") );

		sleep( 2 );
	}

	std::ofstream out;
	std::cout << "Writing pdf to " << argv[2] << std::endl;
	out.open( argv[2] );
	out << *booklet;
	out.close();

	

	return 0;
	
}
