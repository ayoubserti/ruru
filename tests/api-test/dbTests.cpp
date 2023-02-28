#include <gtest/gtest.h>
#include "pch.h"
#include "ruru.h"
using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

TEST(newDatabase, no_existant_file)
{
    // clear test directory
    std::filesystem::remove("test/newdb.ru");
    ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
}

TEST(newDatabase, existant_file)
{

    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        db->saveSchema("test/newdb.ru");
        db = ruru::IDatabase::newDatabase("test/newdb.ru");
        EXPECT_TRUE(db != nullptr);
    }
}

TEST(saveSchema, empty_schema)
{
    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        db->saveSchema("test/newdb.ru");
    }
    EXPECT_TRUE(std::filesystem::exists("test/newdb.ru"));
}

TEST(saveSchema, no_empty_schema)
{
    // clear the env
    std::filesystem::remove("test/newdb.ru");
    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        ruru::TablePtr tbl = db->newTable("myTable");
        db->saveSchema("test/newdb.ru");
    }
    EXPECT_TRUE(std::filesystem::exists("test/newdb.ru"));
}

TEST(getAllTables, no_empty_schema)
{
    // clear the env
    std::filesystem::remove("test/newdb.ru");
    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        ruru::TablePtr tbl = db->newTable("myTable");
        auto tbls = db->getAllTables();
        EXPECT_EQ(tbls.size(), 1);
        EXPECT_EQ(tbls[0], tbl);
    }
}

TEST(getTable, no_empty_schema)
{
    // clear the env
    std::filesystem::remove("test/newdb.ru");
    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        ruru::TablePtr tbl = db->newTable("myTable");
        auto tbl2 = db->getTable("myTable");
        EXPECT_EQ(tbl2, tbl);

        auto tbl_no_existing = db->getTable("inexistant_table");
        EXPECT_TRUE(tbl_no_existing == nullptr);
    }
}

TEST(removeTable, table_exist)
{
    // clear the env
    std::filesystem::remove("test/newdb.ru");
    {
        ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
        db->newTable("myTable");
        db->removeTable("myTable");
        EXPECT_TRUE(db->getTable("myTable") == nullptr);
    }
}

TEST(openDatabase, empty_db)
{
    ruru::Init();
    using ruru::IDatabase;
    {
        ruru::DatabasePtr db = IDatabase::newDatabase("test/empty_db.ru");
        db->saveSchema("test/empty_db.ru");
    }
    //openDatabase
    {
        auto db = IDatabase::openDatabase("test/empty_db.ru");
        EXPECT_TRUE(db != nullptr);
        EXPECT_EQ( db->getAllTables().size() , 0);
    }
}

TEST(openDatabase, non_empty_db)
{
    ruru::Init();
    using ruru::IDatabase;
    {
        ruru::DatabasePtr db = IDatabase::newDatabase("test/non_empty_db.ru");
        db->newTable("myTable1");
        db->newTable("myTable2");
        db->newTable("myTable3");
        db->newTable("myTable4");
        
        db->saveSchema("test/non_empty_db.ru");
    }
    //openDatabase
    {
        auto db = IDatabase::openDatabase("test/non_empty_db.ru");
        EXPECT_TRUE(db != nullptr);
        EXPECT_EQ( db->getAllTables().size() , 4);
    }
}

int main(int argc, char **argv)
{

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

    return 0;
}