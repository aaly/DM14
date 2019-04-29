project (DM14)
cmake_minimum_required(VERSION 3.2)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 99)

SET(GCC_COVERAGE_COMPILE_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS}")

 #-I . -I includes/core

add_executable( DMC src/main.cpp src/scanner.cpp src/compiler.cpp src/includes/core/common.cpp src/DatatypeBase.cpp src/types.cpp src/parser.cpp src/ast.cpp)
target_link_libraries (DMC pthread)

include_directories(src/ src/includes/core)


######### doxygen

# first we can indicate the documentation build as an option and set it to ON by default
#option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
#find_package(Doxygen)
#if (DOXYGEN_FOUND)
if (false)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/generated_docs)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)   
