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
file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")
add_executable(rurudb ${SOURCE_FILES})

# Add the precompiled header to the project
target_precompile_headers(rurudb PRIVATE ${PCH_FILE})

#Add dependency to precompied header 
#add_dependencies( rurudb  ${PCH_FILE} )

# Link the CLI11 library
target_link_libraries(rurudb ${CONAN_LIBS})

# Add any include directories here
# target_include_directories(rurudb PRIVATE MyIncludeDirectory)
