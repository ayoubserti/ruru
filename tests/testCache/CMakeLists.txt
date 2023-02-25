# testCache
file(GLOB SOURCES_REPL "*.cpp")
add_executable(testCache ${SOURCES_REPL})
#precompiled header
#target_precompile_headers(example_ruru PRIVATE "example/pch.h")

# Add include 
target_include_directories(testCache PUBLIC "../../src" "../../include")
# Make the example dependent of rurudb
add_dependencies(testCache rurudb)
# Link with rurudb library
target_link_libraries(testCache "rurudb")