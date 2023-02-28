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

TEST(Table, add_get_column)
{
    // clear test directory
    std::filesystem::remove("test/newdb.ru");
    std::filesystem::remove("test/MyTable.ru");
    ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
    {
        ruru::TablePtr tbl = db->newTable("MyTable");
        ruru::Column col1("col1", ruru::DataTypes::eInteger);
        ruru::Column col2("col2", ruru::DataTypes::eVarChar);
        tbl->addColumn(col1);
        tbl->addColumn(col2);

        EXPECT_EQ(tbl->getColumnIndex("col2"), 1);
        EXPECT_EQ(tbl->getColumnIndex("col1"), 0);
        EXPECT_EQ(tbl->getColumn("col1").getName(), "col1");
        EXPECT_EQ(tbl->getColumn("col2").getName(), "col2");
    }
}

TEST( Table, getName)
{
    // clear test directory
    std::filesystem::remove("test/newdb.ru");
    std::filesystem::remove("test/MyTable.ru");
    ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
    {
        ruru::TablePtr tbl = db->newTable("MyTable");
        
        EXPECT_EQ(tbl->getName(), "MyTable");
    }
}

TEST(Table, createRecord)
{
    // clear test directory
    std::filesystem::remove("test/newdb.ru");
    std::filesystem::remove("test/MyTable.ru");
    ruru::DatabasePtr db = ruru::IDatabase::newDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
    {
        ruru::TablePtr tbl = db->newTable("MyTable");
        ruru::Column col1("col1", ruru::DataTypes::eInteger);
        ruru::Column col2("col2", ruru::DataTypes::eVarChar);
        tbl->addColumn(col1);
        tbl->addColumn(col2);

        auto rec = tbl->CreateRecord();
        EXPECT_TRUE( rec != nullptr);
        rec->SetFieldValue("col1", (int64_t)20);
        rec->SetFieldValue("col2", "Hello");
        EXPECT_TRUE(rec->Save());
    }
}

TEST ( Table, Search)
{
    ruru::DatabasePtr db = ruru::IDatabase::openDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
    {
        auto tbl = db->getTable("MyTable");
        auto recs = tbl->Search({});
        EXPECT_TRUE(recs != nullptr);
    }
}

TEST( Table, GetRecord)
{
    ruru::DatabasePtr db = ruru::IDatabase::openDatabase("test/newdb.ru");
    EXPECT_TRUE(db != nullptr);
    {
        auto tbl = db->getTable("MyTable");
        auto rec  = tbl->GetRecord(1);
        EXPECT_TRUE(rec != nullptr);
    }
}


int main(int argc, char **argv)
{

    testing::InitGoogleTest(&argc, argv);
    ruru::Init();

    return RUN_ALL_TESTS();

    return 0;
}