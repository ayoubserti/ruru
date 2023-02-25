cmake_minimum_required(VERSION 3.18)
project(rurudb)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Add the Conan package management system
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Set the name of the precompiled header file
set(PCH_FILE "src/pch.h")

# Add all of the .cpp files in the src directory to the target executable
file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h" "src/internal/*.cpp")
add_library(rurudb STATIC ${SOURCE_FILES})

target_include_directories(rurudb PUBLIC "include" PRIVATE "src")

# Add the precompiled header to the project
target_precompile_headers(rurudb PRIVATE ${PCH_FILE})

########################## EXAMPLE ################################
# Example
file(GLOB SOURCES_EXAMPLE "example/*.cpp" "example/*.h")
add_executable(example_ruru ${SOURCES_EXAMPLE})
#precompiled header
target_precompile_headers(example_ruru PRIVATE "example/pch.h")

# Add include 
target_include_directories(example_ruru PUBLIC "include")
# Make the example dependent of rurudb
add_dependencies(example_ruru rurudb)
# Link with rurudb library
target_link_libraries(example_ruru "rurudb")

# Link the external libraries
target_link_libraries(example_ruru ${CONAN_LIBS})


########################## REPL ################################
# Example
file(GLOB SOURCES_REPL "repl/*.cpp" "example/*.h")
add_executable(repl ${SOURCES_REPL})
#precompiled header
#target_precompile_headers(example_ruru PRIVATE "example/pch.h")

# Add include 
target_include_directories(repl PUBLIC "include")
# Make the example dependent of rurudb
add_dependencies(repl rurudb)
# Link with rurudb library
target_link_libraries(repl "rurudb")
target_link_libraries( repl "readline")

# Link the external libraries
target_link_libraries(repl ${CONAN_LIBS})


# include tests
add_subdirectory(tests)


