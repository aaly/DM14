project ( DM14 )
cmake_minimum_required( VERSION 3.2 )
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 99)

SET(GCC_COVERAGE_COMPILE_FLAGS "-g")
SET(CMAK_CXX_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS}")

 #-I . -I includes/core

add_executable( DMC src/main.cpp src/scanner.cpp src/compiler.cpp src/includes/io/string.cpp src/includes/core/common.cpp src/DatatypeBase.cpp src/types.cpp src/parser.cpp src/ast.cpp src/includes/core/message.cpp src/includes/core/Node.cpp src/includes/core/Socket.cpp)
target_link_libraries (DMC pthread)

include_directories(src/ src/includes/core)