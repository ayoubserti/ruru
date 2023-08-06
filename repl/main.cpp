// Copyright (c) 2023 Ayoub Serti
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// REPL

#include <iostream>
#include "Console.hpp"
#include "../src/pch.h" /// to fix
#include "ruru.h"
#include <string>
#include <regex>

using ret = CppReadline::Console::ReturnCode;

std::shared_ptr<ruru::IDatabase> gDatabase = nullptr;
ruru::RecordTablePtr gRecord = nullptr;
ruru::TablePtr gTable = nullptr;

unsigned info(const std::vector<std::string> &args)
{
    if (args.size() == 1)
    {
        std::cout << "\033[1mruruDB\033[0m is WIP\n"
                  << "this REPL is a quick and dirty application of rurudb library\n"
                  << "command list:\n\n"
                  << "      help    show this help\n"
                  << "      quit    end the program\n"
                  << "      open    open a database\n"
                  << "      create  create a database\n"
                  << "      prompt  print text\n"
                  << "\n"
                  << "to get the help of command, type help + command\n"
                  << "use it in your own risk\n";
    }
    else if (args[1] == "open")
    {
        std::cout << "\nOpen an already existing database\n"
                  << "ex: open path/to/db.ru\n\n";
    }

    return ret::Ok;
}
unsigned print_rec(const std::shared_ptr<ruru::Table> tbl, const ruru::RecordTablePtr rec)
{
    auto &colls = tbl->getColumns();
    std::cout << "\n{ ";
    for (auto &&it : colls)
    {   
        bool isNull= false;
        rec->IsFieldNull(it.getName(),isNull);
        if ( isNull)
        {
            std::cout << "\033[30m\"" << it.getName() << "\"\033[0m : \"NULL\"\n";
            
            continue;
        }
        switch (it.getType())
        {
        
        case ruru::DataTypes::eVarChar:
        {
            std::string v;
            rec->GetFieldValue(it.getName(), v);
            std::cout << "\033[30m\"" << it.getName() << "\"\033[0m : \"" << v << "\"\n";
            break;
        }
        case ruru::DataTypes::eInteger:
        {
            int64_t v;
            rec->GetFieldValue(it.getName(), v);
            std::cout << "\033[30m\"" << it.getName() << "\"\033[0m : " << v << "\n";
            break;
        }
        case ruru::DataTypes::eDouble:
        {
            double v;
            rec->GetFieldValue(it.getName(), v);
            std::cout << "\033[30m\"" << it.getName() << "\"\033[0m : " << v << "\n";
            break;
        }
        default:
            break;
        }
    }
    std::cout << " }\n";
    return ret::Ok;
}

// search and print resutlt
unsigned search(std::shared_ptr<ruru::Table> tbl, const ruru::Filters_t &filters)
{
    if (tbl == nullptr)
        return ret::Error;

    std::cout << "Searching...\n";
    auto res = tbl->Search(filters);
    if (res == nullptr)
    {
        std::cout << "Empty\n";
        return ret::Ok;
    }
    while (!res->Eof())
    {
        auto rec = res->Next();
        if (rec != nullptr)
        {
            print_rec(tbl, rec);
        }
    }

    return ret::Ok;
}

unsigned count_records(std::shared_ptr<ruru::Table> tbl, const ruru::Filters_t &filters)
{
    if (tbl == nullptr)
        return ret::Error;

    auto res = tbl->Search(filters);
    if (res == nullptr)
    {
        std::cout << "0\n";
        return ret::Ok;
    }
    std::cout << res->GetSize() << "\n";

    return ret::Ok;
}
unsigned describe_table(std::shared_ptr<ruru::Table> tbl)
{
    if (tbl == nullptr)
        return ret::Error;

    auto cols = tbl->getColumns();
    for ( auto&& col : cols )
    {
        std::cout << col.getName() << "             " << ruru::getStringFromType(col.getType()) << "\n";
    }
   
    return ret::Ok;
}

unsigned openDB(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cout << "\nOpen an already existing database\n"
                  << "ex: open path/to/db.ru\n\n";
        return ret::Error;
    }

    gDatabase = ruru::IDatabase::openDatabase(args[1]);

    return ret::Ok;
}
unsigned dbCmd(const std::vector<std::string> &args)
{
    if (gDatabase == nullptr)
        return ret::Error;
    if (args[1] == "tables")
    {
        auto alltables = gDatabase->getAllTables();
        for (auto &&it : alltables)
        {
            std::cout << it->getName() << std::endl;
        }
    }
    else if (args[1] == "table" && args.size() > 2)
    {
        auto table = gDatabase->getTable(args[2]);
        if (table == nullptr)
        {
            std::cout << "Table " << args[2] << " doesn't exist\n";
            return ret::Error;
        }
        if (args.size() > 3 && args[3] == "all")
        {
            return search(table, {});
        }
        else if (args.size() && args[3] == "new")
        {
            // create a new records
            gRecord = table->CreateRecord();
            if (gRecord != nullptr)
                gTable = table;
        }
        else if ( args.size() && args[3] == "count")
        {
            return count_records(table, {});
        }
        else if ( args.size() && args[3] == "describe")
        {
            return describe_table(table);
        }
    }

    return ret::Ok;
}


unsigned record(const std::vector<std::string> &args)
{
    if (gRecord == nullptr)
    {
        std::cout << "No record selected" << std::endl;
        return ret::Error;
    }
    if ( args[1] == "show")
    {
        //show the record value
        return print_rec(gTable, gRecord);
        
    }
    else if (args[1] == "save")
    {
        return gRecord->Save() ? ret::Ok : ret::Error;
    }
    if (args.size() > 3 && args[1] == "set")
    {
        std::cout << "set " << args[2] << "   " << args[3] << "\n";
        // set value
        std::regex pattern_str(R"(^\s*"(?:[^"\\]|\\.)*"\s*$)");
        std::regex pattern_integer("[0-9]+");
        std::regex pattern_double(R"([0-9]+(?:\.[0-9]+))");
        std::smatch match;
        if (std::regex_match(args[3], pattern_str))
        {
            std::regex_search(args[3], match, pattern_str);
            std::string vv = match.suffix().str();
            // vv = vv.replace(vv.find("\\\""), 1, "\"");
            std::cout << vv << std::endl;
            gRecord->SetFieldValue(args[2], vv);
        }
        else if (std::regex_match(args[3], pattern_integer))
        {
            gRecord->SetFieldValue( args[2], (int64_t)std::atoi(args[3].c_str()));
        }
        else if (std::regex_match(args[3], pattern_double))
        {
            gRecord->SetFieldValue( args[2], (double)std::atof(args[3].c_str()));
        }
    }
    return ret::Ok;
}

unsigned clear(const std::vector<std::string> &)
{
    std::cout << "\033[2J" << std::endl;
    return ret::Ok;
}

unsigned prompt(const std::vector<std::string> &args)
{
    // show text
    int i = 1;
    for (  i =1 ; i< args.size() ; i++)
    {
        std::cout << "\033[31m"<< args[i] << " \033[0m";
    } 
    if ( i != 1)
        std::cout << "\n";
    return ret::Ok;
}

int main(int argc, char **argv)
{
    ruru::Init();

    CppReadline::Console console("> ");
    console.registerCommand("clear", clear);
    console.registerCommand("help", info);
    console.registerCommand("open", openDB);
    console.registerCommand("database", dbCmd);
    console.registerCommand("record", record);
    console.registerCommand("prompt" , prompt);

    console.executeCommand("help");

    if (argc > 1)
        console.executeFile(argv[1]);

    int retCode;
    do
    {
        retCode = console.readLine();
        // We can also change the prompt based on last return value:
        if (retCode == ret::Ok)
            console.setGreeting("> ");
        else
            console.setGreeting("!> ");

        if (retCode == 1)
        {
            std::cout << "Received error code 1\n";
        }
        else if (retCode == 2)
        {
            std::cout << "Received error code 2\n";
        }
    } while (retCode != ret::Quit);

    return 0;
}