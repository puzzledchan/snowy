set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/examples)

set(LIBS snowy)

set(EXAMPLE_RPC_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/examples/rpc" EXAMPLE_RPC_FILES)

foreach(EXAMPLE_RPC_FILES ${EXAMPLE_RPC_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cpp" "\\1" EXAMPLE_RPC_FILES ${EXAMPLE_RPC_FILES})
    message(STATUS "Generating example target: ${EXAMPLE_RPC_FILES} ")
    snowy_add_executable(${EXAMPLE_RPC_FILES} rpc/${EXAMPLE_RPC_FILES}.cpp snowy ${LIBS})
endforeach(EXAMPLE_RPC_FILES)