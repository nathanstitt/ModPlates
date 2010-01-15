PlatesScreen = {
	URL: 'ajax/',
	KEY: 0,
	initialize:function(){
		PlatesScreen.KEY = $.cookie('key');
		$("#chart_types").sortable();
		$("#chart_types").disableSelection();

		$("#restart").click(function(){
					    PlatesScreen.showPrevDL();
					    $("#dl,#throbber").hide();
		    			    $("#form").show();
					    $('#status,#errors').empty();
				    });

		$("#form").submit(function() {
					  $("#prevdl,#form").hide();
					  $("#throbber").show();
					  var charts = jQuery.grep( $("#chart_types").sortable('toArray'), function(n,i){
									    return $("input[name='"+ n + "']").attr('checked');
								    });
					  jQuery.get( PlatesScreen.URL+'start',{
						       airports: $('#airports').val(),
						       charts: charts
						     }, PlatesScreen.bookletStarted, "json" );
					return false;
				      } );
		PlatesScreen.showPrevDL();
	},

	showDL: function(){
		$("#dlink").attr('href', PlatesScreen.URL + "retrieve?key=" + PlatesScreen.KEY );
		$("#dl").show();
	},

	showPrevDL: function(){
		if ( PlatesScreen.KEY ){
			$("#prevdlink").attr('href', PlatesScreen.URL + "retrieve?key=" + PlatesScreen.KEY );
			$("#prevdl").show();
		}
	},

	appendErrors: function (errors){
		if ( ! errors || ! errors.length )
			return;
		$.each( errors, function( indx,val ){
				$('#errors').append( '<li class="error">'+val+'</li>' );
			});
	},

	bookletStarted: function ( data,status ){
		PlatesScreen.KEY = data.key;
		$.cookie('key',data.key );
		if ( data.errors.length ){
			$('#errors').append( '<li class="error">'+ data.errors.join(', ') + ' were not found</li>' );
		}
		$.PeriodicalUpdater( PlatesScreen.URL + 'status', {
					     data: { key: PlatesScreen.KEY },
					     type: 'json'
				     }, PlatesScreen.updateRecved );

	},

	updateRecved: function( data ){
		PlatesScreen.appendErrors( data.errors );
		if ( 'READY' == data.status ){
			PlatesScreen.showDL();
			$("#throbber").hide();
			return false;
		} else if ( data.ok ){
    			$('#status').empty();
			$.each( data.messages, function( indx,val ){
					$('#status').prepend( '<li>'+val+'</li>' );
				} );
			return true;
		} else {
			return false;
		}

	}


};





$(document).ready( PlatesScreen.initialize );





/**
 * Create a cookie with the given name and value and other optional parameters.
 *
 * @example $.cookie('the_cookie', 'the_value');
 * @desc Set the value of a cookie.
 * @example $.cookie('the_cookie', 'the_value', { expires: 7, path: '/', domain: 'jquery.com', secure: true });
 * @desc Create a cookie with all available options.
 * @example $.cookie('the_cookie', 'the_value');
 * @desc Create a session cookie.

 * Get the value of a cookie with the given name.
 *
 * @example $.cookie('the_cookie');
 * @desc Get the value of a cookie.
 *

 * @author Klaus Hartl/klaus.hartl@stilbuero.de
 */

jQuery.cookie = function(name, value, options) {
    if (typeof value != 'undefined') { // name and value given, set cookie
        options = options || {};
        if (value === null) {
            value = '';
            options.expires = -1;
        }
        var expires = '';
        if (options.expires && (typeof options.expires == 'number' || options.expires.toUTCString)) {
            var date;
            if (typeof options.expires == 'number') {
                date = new Date();
                date.setTime(date.getTime() + (options.expires * 24 * 60 * 60 * 1000));
            } else {
                date = options.expires;
            }
            expires = '; expires=' + date.toUTCString(); // use expires attribute, max-age is not supported by IE
        }
        // CAUTION: Needed to parenthesize options.path and options.domain
        // in the following expressions, otherwise they evaluate to undefined
        // in the packed version for some reason...
        var path = options.path ? '; path=' + (options.path) : '';
        var domain = options.domain ? '; domain=' + (options.domain) : '';
        var secure = options.secure ? '; secure' : '';
        document.cookie = [name, '=', encodeURIComponent(value), expires, path, domain, secure].join('');
    } else { // only name given, get cookie
        var cookieValue = null;
        if (document.cookie && document.cookie != '') {
            var cookies = document.cookie.split(';');
            for (var i = 0; i < cookies.length; i++) {
                var cookie = jQuery.trim(cookies[i]);
                // Does this cookie string begin with the name we want?
                if (cookie.substring(0, name.length + 1) == (name + '=')) {
                    cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                    break;
                }
            }
        }
        return cookieValue;
    }
    return null;
};


/**
 * PeriodicalUpdater - jQuery plugin for timed, decaying ajax calls
 *
 * http://www.360innovate.co.uk/blog/2009/03/periodicalupdater-for-jquery/
 * http://enfranchisedmind.com/blog/posts/jquery-periodicalupdater-ajax-polling/
 *
 * Copyright (c) 2009 by the following:
 *  Frank White (http://customcode.info)
 *  Robert Fischer (http://smokejumperit.com)
 *  360innovate (http://www.360innovate.co.uk)
 *
 * Dual licensed under the MIT and GPL licenses:
 * http://www.opensource.org/licenses/mit-license.php
 * http://www.gnu.org/licenses/gpl.html
 *
 * Version: 3.0
 *
 */

(function($) {
   function pu_log(msg) {
     try {
       console.log(msg);
     } catch(err) {}
   }

   // Now back to our regularly scheduled work
   $.PeriodicalUpdater = function(url, options, callback){

     var settings = jQuery.extend(true, {
				    url: url,					// URL of ajax request
				    cache: false,		  // By default, don't allow caching
				    method: 'GET',		// method; get or post
				    data: '',				  // array of values to be passed to the page - e.g. {name: "John", greeting: "hello"}
				    minTimeout: 1000,	// starting value for the timeout in milliseconds
				    maxTimeout: 8000,	// maximum length of time between requests
				    multiplier: 2,		// if set to 2, timerInterval will double each time the response hasn't changed (up to maxTimeout)
				    maxCalls: 0,      // maximum number of calls. 0 = no limit.
				    autoStop: 0       // automatically stop requests after this many returns of the same data. 0 = disabled
				  }, options);

     // set some initial values, then begin
     var timerInterval = settings.minTimeout;
     var maxCalls      = settings.maxCalls;
     var autoStop      = settings.autoStop;
     var calls         = 0;
     var noChange      = 0;

     // Function to boost the timer (nop unless multiplier > 1)
     var boostPeriod = function() { return; };
     if(settings.multiplier > 1) {
       boostPeriod = function() {
	 timerInterval = timerInterval * settings.multiplier;

	 if(timerInterval > settings.maxTimeout) {
	   timerInterval = settings.maxTimeout;
	 }
       };
     }

     // Construct the settings for $.ajax based on settings
     var ajaxSettings = jQuery.extend(true, {}, settings);
     if(settings.type && !ajaxSettings.dataType) ajaxSettings.dataType = settings.type;
     if(settings.sendData) ajaxSettings.data = settings.sendData;
     ajaxSettings.type = settings.method; // 'type' is used internally for jQuery.  Who knew?
     ajaxSettings.ifModified = true;

     // Create the function to get data
     // TODO It'd be nice to do the options.data check once (a la boostPeriod)
     function getdata() {

       var toSend  = jQuery.extend(true, {}, ajaxSettings); // jQuery screws with what you pass in
       if(typeof(options.data) == 'function') {
	 toSend.data = options.data();
	 if(toSend.data) {
	   // Handle transformations (only strings and objects are understood)
	   if(typeof(toSend.data) == "number") {
	     toSend.data = toSend.data.toString();
	   }
	 }
       }

       if(maxCalls == 0) {
         $.ajax(toSend);
       } else if(maxCalls > 0 && calls < maxCalls) {
         $.ajax(toSend);
         calls++;
       }
     }

     // Implement the tricky behind logic
     var remoteData  = null;
     var prevData    = null;

     ajaxSettings.success = function(data) {
       pu_log("Successful run! (In 'success')");
       remoteData      = data;
       timerInterval   = settings.minTimeout;
     };

     ajaxSettings.complete = function(xhr, success) {
       pu_log("Status of call: " + success + " (In 'complete')");
       if(success == "success" || success == "notmodified") {
	 var rawData = $.trim(xhr.responseText);
         if(rawData == 'STOP_AJAX_CALLS')
         {
           maxCalls = -1;
           return;
         }
	 if(prevData == rawData) {
           if(autoStop > 0)
           {
             noChange++;
             if(noChange == autoStop)
             {
               maxCalls = -1;
               return;
             }
           }
	   boostPeriod();
	 } else {
           noChange        = 0;
           timerInterval   = settings.minTimeout;
	   prevData        = rawData;
	   if(remoteData == null) remoteData = rawData;
           if(ajaxSettings.dataType == 'json') remoteData =  window["eval"]("(" + rawData + ")");
	   if(settings.success) { settings.success(remoteData); }
	   if ( callback && ! callback(remoteData) ){
	       maxCalls = -1;
               return;
	   }

	 }
       }
       remoteData = null;
       setTimeout(getdata, timerInterval);
     };


     ajaxSettings.error = function (xhr, textStatus) {
       pu_log("Error message: " + textStatus + " (In 'error')");
       if(textStatus == "notmodified") {
	 boostPeriod();
       } else {
	 prevData = null;
	 timerInterval = settings.minTimeout;
       }
       if(settings.error) { settings.error(xhr, textStatus); }
     };

     // Make the first call
     $(function() { getdata(); });
   };
 })(jQuery);
