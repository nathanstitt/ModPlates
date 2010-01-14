#ifndef __MOD_PLATES_H__
#define __MOD_PLATES_H__

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

#define MAX_RUNNING  10
#define KEEP_MINUTES 10


#ifdef __cplusplus
extern "C" {
#endif 

	typedef unsigned int booklet_key_t;
	typedef void (*plates_callback_t)(unsigned short int count, const char*, void*);
	typedef int ( *plates_write_t )( const void *buf, int nbyte, void *r);

	void			plates_config( const char *directory );
	
	booklet_key_t		plates_start_booklet( const char *airports, char** charts, int num_charts );

	const char *		plates_status( booklet_key_t key );
        unsigned short int	plates_report_status( booklet_key_t key,    plates_callback_t, void *cbarg );
        unsigned short int	plates_report_not_found( booklet_key_t key, plates_callback_t, void *cbarg );
	booklet_key_t           plates_to_key( const char *key );
	size_t                  plates_booklet_data( booklet_key_t key, const char **data, const char **suggested_filename );

	void			plates_destroy();

#ifdef __cplusplus
}
#endif

#endif // __MOD_PLATES_H__
