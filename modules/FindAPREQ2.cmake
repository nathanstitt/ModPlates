FIND_PROGRAM(APREQ2 apreq2-config)
IF (APREQ2)
        EXEC_PROGRAM(${APREQ2} 
                ARGS "--includedir"
                OUTPUT_VARIABLE APREQ2_INCLUDES)
        EXEC_PROGRAM(${APREQ2} 
               ARGS "--ldflags"
               OUTPUT_VARIABLE APREQ2_LIBRARIES)
ELSE(APREQ2)
        MESSAGE(SEND_ERROR "Cannot find apreq2 anywhere in your path.  Please update your path to include the directory containing the script.")
ENDIF(APREQ2)
