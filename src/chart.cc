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

#include "chart.h"
#include "cycle.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <poppler.h>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "airport.h"
#include "logging.h"

// type is UNK by default
Chart::Chart(Airport *a ) :
	airport(a),
	type(UNK)
{

}

// could really just as easily just use straight fwrite
static size_t
my_curl_write_callback(void *ptr, size_t size, size_t nmemb, void *file )
{
	return fwrite( ptr, size, nmemb, (FILE*)file );
}

// utility method to output identifier
std::string
Chart::to_s() const {
	return airport->identifier + " - " + name;
}


// layout chart identity
bool
Chart::layout_identity( Booklet *booklet, cairo_t *cr ) const {

	cairo_set_line_width (cr, 1.0);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_rectangle (cr, 0.0, 0.0, 100, 100 );
	cairo_stroke (cr);

	return true;
}

// layout chart onto cairo surface owned by booket
bool
Chart::layout_contents( Booklet *booklet, cairo_t *cr ) const {
	PopplerDocument *document;
	PopplerPage *page;
	GError *error=NULL;
	std::string log( this->to_s() );

	// if we don't have the chart, fetch it
	const_cast<Chart*>(this)->fetch_if_unfresh( booklet );

	// convert it to a uri format
	gchar *uri = g_filename_to_uri( this->path().c_str(), NULL, &error);

	booklet->log_msg( log + " rendering");

	LOG_DEBUG("\t" << log );

	if ( uri == NULL) {
		log += "Failed to convert path to uri: ";
		log += error->message;
		LOG_ERROR( log );
		booklet->log_replace_last_msg( log );
		return false;
	}

	document = poppler_document_new_from_file ( uri , NULL, &error);
	if (document == NULL) {
		log += "Loading NACO pdf failed: ";
		log += error->message;
		LOG_ERROR( log );
		booklet->log_replace_last_msg( log );
		g_object_unref (document);
		return false;
	}

	page = poppler_document_get_page (document, 0);
	if (page == NULL) {
		log += "Failed to load first page from NACO pdf";
		LOG_ERROR( log );
		booklet->log_replace_last_msg( log );
		g_object_unref (page);
		g_object_unref (document);
		return false;
	}
	poppler_page_render_for_printing (page, cr);

	LOG_DEBUG( log );

	// since we are done rendering, replace the rendering message 
	// with our real message
	booklet->log_replace_last_msg( log );

	g_object_unref (page);
	g_object_unref (document);
	return true;
}


std::string
Chart::path() const{
	return ( this->cycle->directory / this->pdf ).string();
}

std::string
Chart::working_path() const{
	return ( this->cycle->directory / (this->pdf + ".wrk") ).string();
}

bool
Chart::fetch( Booklet *booklet ){

	std::string url( "http://www.naco.faa.gov/d-tpp/" ) ;
	url += this->cycle->id;
	url += "/";
	url += this->pdf;
	LOG_DEBUG( "Fetching: " << url );

	curl_easy_setopt( booklet->curl, CURLOPT_URL, url.c_str() );

	std::string log("Fetching " );
	log += this->name;

	FILE *dest = fopen( this->working_path().c_str(), "w");
	if (! dest ){
		LOG_ERROR( "Failed to open chart working file: " << this->working_path() );
		return false;
	}

	curl_easy_setopt( booklet->curl, CURLOPT_WRITEFUNCTION, my_curl_write_callback );

	curl_easy_setopt( booklet->curl, CURLOPT_WRITEDATA, (void *)dest);


        CURLcode res = curl_easy_perform( booklet->curl );

	fclose(dest);

	if ( res == CURLE_OK ){
		LOG_DEBUG( log );
		boost::filesystem::rename( this->working_path(), this->path() );
	} else {
		log += " - FAILED: ";
		log += curl_easy_strerror( res );
		LOG_ERROR( log );
		boost::filesystem::remove( this->working_path() );
	}


	return ( res == CURLE_OK );
}

bool
Chart::fetch_if_unfresh( Booklet *booklet ){
	if ( READY != this->state() ){
		this->fetch( booklet );
	}
	return true;
}

Chart::State
Chart::state() const{
	if ( boost::filesystem::is_regular_file( this->path() ) ){
		return READY;
	} else if ( boost::filesystem::is_regular_file( this->working_path() )  ){
		return FETCHING;
	} else {
		return UNFETCHED;
	}
}

std::string
Chart::str_type() const {
	return type_to_str(this->type);
}

std::string
Chart::type_to_str( Type t ){
	switch ( t ){
	case APD:
		return "APD";
	case IAP:
		return "IAP";
	case MIN:
		return "MIN";
	case DP:
		return "DP ";
	case STAR:
		return "STAR";
	case DPO:
		return "DPO";
	case CVFP:
		return "CVFP";
	default:
		return "UNK";
	}
}

Chart::Type
Chart::str_to_type( const  std::string& type ) {
	if ( ! type.compare("MIN") ) {
		return MIN;
	} else if ( ! type.compare("IAP") ){
		return IAP;
	} else if ( ! type.compare("DP") ) {
		return DP;
	} else if ( ! type.compare("APD") ){
		return APD;
	} else  if ( ! type.compare("STAR") ){
		return STAR;
	} else  if ( ! type.compare("DPO") ){
		return DPO;
	} else {
		return UNK;
	}
}

bool
Chart::set_property( const std::string& name, const std::string& value ){

	if ( ! name.compare( "chart_code") ){
		*const_cast<Type*>(&this->type) = Chart::str_to_type( value );
		if ( this->type == UNK ){
			LOG_ERROR("FAILED TO INTERPRET Unknown Chart Type " << value );
		}
		return true;
	}
	if ( ! name.compare( "chart_name") ){
		const_cast<std::string*>( &this->name )->assign( value );
		return true;
	}
	if ( ! name.compare( "pdf_name") ){
		const_cast<std::string*>( &pdf )->assign( value );
		return true;
	}
	return false;
}


unsigned short int 
Chart::runway() const {
	static const boost::regex e(".*RWY (\\d\\d).*$");
	static boost::smatch rw;
	if ( boost::regex_match( this->name, rw, e ) ){
		return boost::lexical_cast<unsigned short int>( rw[1] );
	} else {
		return 0;
	}
}

// we attempt to sort charts by the order the types are listed in
// the Type enum, and then by the runway
bool
Chart::compare_identical( const Chart *other )
{
	static unsigned short int this_rw, other_rw;

	if ( Chart::IAP == this->type && Chart::IAP == other->type ){
		this_rw = this->runway();
		other_rw = other->runway();
		LOG_DEBUG( this->name << " ~ " << other->name << " : " << this_rw << " ~ " << other_rw );
		if ( this_rw == other_rw ){
			return this->name < other->name;
		} else {
			return this_rw < other_rw;
		}
	} else {
		return false;
	}
}
