set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

include(FindPkgConfig)
find_package(APXS REQUIRED)
find_package(APREQ2 REQUIRED)
pkg_check_modules( POPPLER poppler-glib )
pkg_check_modules( CAIRO cairo )
pkg_check_modules( CURL libcurl )
pkg_check_modules( LIBXML libxml-2.0 )
pkg_check_modules( APR apr-1 )
find_package( Boost 1.40 COMPONENTS system filesystem regex thread REQUIRED )

add_library( plates airport.cc chart.cc booklet.cc cycle.cc plates.cc )


include_directories( mod_plates 
  ${APREQ2_INCLUDES}
  ${APXS2_INCLUDES}
  ${POPPLER_INCLUDE_DIRS}
  ${CAIRO_INCLUDE_DIRS} 
  ${LIBXML_INCLUDE_DIRS} 
)

include_directories( plates 
  ${APREQ2_INCLUDES}
  ${APXS2_INCLUDES}
  ${POPPLER_INCLUDE_DIRS}
  ${CAIRO_INCLUDE_DIRS} 
  ${LIBXML_INCLUDE_DIRS} 
)

add_library( mod_plates SHARED mod_plates.c airport.cc chart.cc booklet.cc cycle.cc plates.cc )

target_link_libraries( mod_plates
  ${APXS2_LIBRARIES}
  ${APREQ2_LIBRARIES}
  ${POPPLER_LIBRARIES} 
  ${CAIRO_LIBRARIES} 
  ${LIBXML_LIBRARIES} 
  ${CURL_LIBRARIES}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
)

target_link_libraries( plates
  ${APXS2_LIBRARIES}
  ${POPPLER_LIBRARIES} 
  ${CAIRO_LIBRARIES} 
  ${LIBXML_LIBRARIES} 
  ${CURL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_REGEX_LIBRARY}
)



add_executable( plates_test test.cc )
target_link_libraries( plates_test  plates )



# does the application run
add_test (PlatesRuns plates 25)

add_test (PlatesComp25 plates 25)
set_tests_properties (PlatesComp25 
  PROPERTIES PASS_REGULAR_EXPRESSION "25 is 5"
)



