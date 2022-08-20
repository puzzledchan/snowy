set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(LIBS snowy)

set(TESTS_RPC_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests/rpc" TESTS_RPC_FILES)

foreach(TESTS_RPC_FILE_PATH ${TESTS_RPC_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_RPC_FILE_NAME ${TESTS_RPC_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_RPC_FILE_NAME} ")
    snowy_add_executable(${TESTS_RPC_FILE_NAME} rpc/${TESTS_RPC_FILE_NAME}.cpp snowy ${LIBS})
endforeach(TESTS_RPC_FILE_PATH)

aux_source_directory("${CMAKE_SOURCE_DIR}/tests/net" TESTS_NET_FILE_PATH)

foreach(TESTS_NET_FILE_PATH ${TESTS_NET_FILE_PATH})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_NET_FILE_PATH ${TESTS_NET_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_NET_FILE_PATH} ")
    snowy_add_executable(${TESTS_NET_FILE_PATH} net/${TESTS_NET_FILE_PATH}.cpp snowy ${LIBS})
endforeach(TESTS_NET_FILE_PATH)

aux_source_directory("${CMAKE_SOURCE_DIR}/tests/utils" TESTS_UTILS_FILE_PATH)

foreach(TESTS_UTILS_FILE_PATH ${TESTS_UTILS_FILE_PATH})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" TESTS_UTILS_FILE_PATH ${TESTS_UTILS_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_UTILS_FILE_PATH} ")
    snowy_add_executable(${TESTS_UTILS_FILE_PATH} utils/${TESTS_UTILS_FILE_PATH}.cpp snowy ${LIBS})
endforeach(TESTS_UTILS_FILE_PATH)