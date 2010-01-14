FIND_PROGRAM(APXS2 apxs2)
IF (APXS2)
        EXEC_PROGRAM(${APXS2} 
                ARGS "-q CFLAGS"
                OUTPUT_VARIABLE APXS2_C_FLAGS)
        EXEC_PROGRAM(${APXS2} 
                ARGS "-q INCLUDEDIR"
                OUTPUT_VARIABLE APXS2_INCLUDEDIRS)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${APXS2_C_FLAGS} -I${APXS2_INCLUDEDIRS}")
        # apxs2 -q LDFLAGS outputs only a newline which breaks then CMAKE_SHARED_LINKER_FLAGS
        EXEC_PROGRAM(${APXS2} 
               ARGS "-q LDFLAGS"
               OUTPUT_VARIABLE APXS2_LIBRARIES)
        #SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${APXS2_LDFLAGS}")
        EXEC_PROGRAM(${APXS2}
                ARGS "-q libexecdir"
                OUTPUT_VARIABLE MOD_DIR)
        SET(APACHE_MODULE_DIR "${MOD_DIR}" CACHE PATH
                        "Installation directory for Apache modules")
ELSE(APXS2)
        MESSAGE(SEND_ERROR "Cannot find apxs2 anywhere in your path.  Please update your path to include the directory containing the script.")
ENDIF(APXS2)
