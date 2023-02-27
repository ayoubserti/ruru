#macro create_test_func
macro ( create_test_func test_name)
    add_executable(${test_name} "${test_name}.cpp")

    # Add include 
    target_include_directories(${test_name}  PUBLIC  "../../include" "../../src")
    # Make the example dependent of rurudb
    add_dependencies(${test_name}  rurudb)
    # Link with rurudb library
    target_link_libraries(${test_name}  "rurudb" GTest::GTest GTest::Main)
endmacro()

# test-api
#testing the API using gtest suite
find_package(GTest REQUIRED)

# create target for Database interface Tests
create_test_func("dbTests")

# create target for Table interface Tests
create_test_func("tableTests")
