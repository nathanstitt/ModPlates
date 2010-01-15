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

#include "httpd.h" 
#include "http_config.h" 
#include "http_core.h" 
#include "http_log.h" 
#include "http_main.h" 
#include "http_protocol.h" 
#include "http_request.h" 
#include "util_script.h" 
#include "http_connection.h" 
#include "apreq_module.h"
#include "mod_plates.h"


/*
 * The default value for storage directory
 */
#ifndef DEFAULT_MODPLATES_DIRECTORY
#define DEFAULT_MODPLATES_DIRECTORY "/tmp"
#endif

module AP_MODULE_DECLARE_DATA plates_module;

typedef struct {
	char *directory;
} modplates_config;


/*
 *  outputs a value suitable for including in a json array
 */
void
mod_plates_json_output( unsigned short int count, const char *str, void *r ){
	if ( count ){
		ap_rprintf( r, ",'%s'", str );
	} else {
		ap_rprintf( r, "'%s'", str );
	}
}

/*
 *  handles the /start url, starts up a new booklet
 */
static int
mod_plates_start( request_rec *r ){
	size_t key;

	const char *airports;

	apr_table_t *args = apr_table_make( r->pool, APREQ_DEFAULT_NELTS); 
	apreq_parse_query_string( r->pool, args, r->parsed_uri.query);

	airports = apreq_params_as_string( r->pool, args, "airports", APREQ_JOIN_AS_IS);
	apr_array_header_t* arr =  apreq_params_as_array( r->pool, args, "charts" );
	
	if ( ! airports || ! arr->nelts ){
		apr_table_setn(r->notes, "error-notes", "Reason: You didn't set the airports parameter and / or give me at least one type of chart<br />\n" );
		return HTTP_BAD_REQUEST;
	}

	apreq_param_t **elt = (apreq_param_t **)arr->elts;
	apreq_param_t **const end = elt + arr->nelts;
	char *charts[10];
	int i =0;
	while ( elt < end && i < 10 ) {
		const apreq_value_t *a = &(**elt).v;

		char *d = apr_palloc(r->pool, a->dlen+1 );
		memcpy(d, a->data, a->dlen );
		d[ a->dlen ] = '\0';

		charts[ i++ ] = d;

		++elt;
	}

	key = plates_start_booklet( airports, charts, i );

	if ( key ){
		ap_rprintf(r, "{ ok:true, key: %lu, errors:[", (long unsigned)key );
		plates_report_not_found( key, &mod_plates_json_output, r );
		ap_rputs( "] }", r );
	} else {
		ap_rputs( "{ 'ok':false, 'errors': ['failed to bind booklet, server too busy'] }", r  );
	}
	return OK;
}
/* 
 * gets the key from the uri and converts it to int
 */
booklet_key_t 
key_from_req( request_rec *r ){
	apr_table_t *args = apr_table_make( r->pool, APREQ_DEFAULT_NELTS); 
	apreq_parse_query_string( r->pool, args, r->parsed_uri.query);
	const char *key = apreq_params_as_string( r->pool, args, "key", APREQ_JOIN_AS_IS);
	if ( strlen( key ) ){
		return plates_to_key( key );
	} else {
		return 0;
	}
}
/*
 *  handles the /status url, outputs the status of the given booklet
 */
static int
mod_plates_status( request_rec *r ){
	booklet_key_t key = key_from_req(r);
	if ( key ){
		ap_rprintf(r, "{ ok:true,status:'%s', messages: [", plates_status( key ) );
		plates_report_status( key, &mod_plates_json_output, r );
		ap_rputs( "] }", r );
	} else {
		ap_rputs( "{ 'ok':false, 'errors': ['key not found, perhaps it expired?'] }", r  );
	}
	

	return OK;
}

/*
 *  handles the /retrieve url, outputs the booklet as a pdf file
 */
static int
mod_plates_retrieve( request_rec *r ){
	booklet_key_t key = key_from_req(r);
	if ( key ){
		const char *data;
		size_t len=0;
		const char *fname;
		if ( ! ( len = plates_booklet_data( key, &data, &fname ) ) ) {
			ap_set_content_type(r, "text/plain" );
			ap_rputs( "Failed to render PDF", r  );
		} else {
			ap_set_content_type( r, "application/pdf" );

			apr_table_set(r->headers_out, "Content-Disposition", "attachment; filename=\"booklet.pdf\"");
			ap_rwrite(data, len,r);
		}

	} else {
		ap_set_content_type(r, "text/plain" );
		ap_rputs( "key not found, perhaps it expired?", r  );
	}
	

	return OK;
}


/*
 *  the main handler for the module
 */
static int
mod_plates_method_handler (request_rec *r) {

	static const char *sep = "/";
	char *tok = 0;
	char *arg;

	/* mod_plates is polite and only handles what it aught */
	if (strcmp(r->handler, "mod_plates")) { 
                return DECLINED; 
        } 
	

        /* we don't do any header only stuff, probably 
	   should consider returning not-changed on
	   expensive retrieve call
	 */ 
        if (r->header_only) {
                return OK; 
        }

	if (strcmp(r->method, "GET")) { 
		apr_table_setn(r->notes, "error-notes", "I only listen to GET requests" );
		return HTTP_BAD_REQUEST;
           }

	/*  everything we do is via JSON  */
        ap_set_content_type(r, "application/json" );

	if ( strstr(r->uri, "/start") ){
		return mod_plates_start( r );
	} else if ( strstr(r->uri, "/status") ){
		return mod_plates_status( r );
	} else if ( strstr(r->uri, "/retrieve") ){
		return mod_plates_retrieve( r );
	} else {
		return HTTP_NOT_FOUND;
	}
}

/* We're shutting down, cleanup
 */
static apr_status_t
destroy_modplates(void *arg) {
	plates_destroy();
}

/* becouse we are built as a DSO, apache loads us twice
   therefore we only initialize on the secont time around
   idea taken from somewhere on google, forgot who to credit, sorry
 */
static int mod_plates_initialize(apr_pool_t *pconf, apr_pool_t *plog,
				 apr_pool_t *ptemp, server_rec *s)
{

	void *data;
	const char *userdata_key = "modplates_init";

	/* initialize_module() will be called twice, and if it's a DSO
	 * then all static data from the first call will be lost. Only
	 * set up our static data on the second call. */
	apr_pool_userdata_get(&data, userdata_key, s->process->pool);
	if (!data) {
		apr_pool_userdata_set((const void *)1, userdata_key,
				      apr_pool_cleanup_null, s->process->pool);
		return OK;
	} else {
		modplates_config *cfg = ap_get_module_config(
			s->module_config, &plates_module);

		apr_initialize();

		plates_config( cfg->directory );

		apr_pool_cleanup_register(pconf, NULL,
					  destroy_modplates,
					  apr_pool_cleanup_null);

	}
	return OK;
}



static void mod_plates_register_hooks (apr_pool_t *p)
{
	ap_hook_handler(mod_plates_method_handler, NULL, NULL, APR_HOOK_LAST);
	ap_hook_post_config(mod_plates_initialize, NULL, NULL, APR_HOOK_MIDDLE);
}


static const char *set_modplates_directory(cmd_parms *parms, void *mconfig, const char *arg)
{
	modplates_config *cfg = ap_get_module_config(
		parms->server->module_config, &plates_module);
	cfg->directory = (char *) arg;
	return NULL;
}

/**
 * A declaration of the configuration directives that are supported by this module.
 */
static const command_rec mod_plates_cmds[] =
{
	AP_INIT_TAKE1(
		"PlatesStorageDirectory",
		set_modplates_directory,
		NULL,
		RSRC_CONF,
		"PlatesStorageDirectory <string> -- the directory mod_plates should store it's files in"
	),
	{NULL}
};


static void *create_modplates_config(apr_pool_t *p, server_rec *s) {
	modplates_config *newcfg;

	// allocate space for the configuration structure from the provided pool p.
	newcfg = (modplates_config *) apr_pcalloc(p, sizeof(modplates_config));

	// set the default value for the error string.
	newcfg->directory = DEFAULT_MODPLATES_DIRECTORY;

	// return the new server configuration structure.
	return (void *) newcfg;
}


module AP_MODULE_DECLARE_DATA mod_plates =
{
	STANDARD20_MODULE_STUFF, // standard stuff; no need to mess with this.
	NULL, // create per-directory configuration structures - we do not.
	NULL, // merge per-directory - no need to merge if we are not creating anything.
	create_modplates_config, // create per-server configuration structures.
	NULL, // merge per-server - hrm - examples I have been reading don't bother with this for trivial cases.
	mod_plates_cmds, // configuration directive handlers
	mod_plates_register_hooks, // request handlers
};

