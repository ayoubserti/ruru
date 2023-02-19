database
 - table
    - column
    - RecordTable
 - StorageEngine  -> file
   - Record
   - RecordFile
   - Field

    1- Create a new database from JSON schema  --> Application responsability
    2- Persiste Database table into the disc    --> done 
    3- Create Record and populate values       --> done 
    4- Save Record                             --> done
    5- Open Database from disq file            --> done
    6- Search record                           --> done
    7- Manage Indexes                  
    8- Cache                              
    9- Log files and transactions
   10- Allocator & memory management
   11- REPL (WIP)



In Progress:
  -[OK] fix Integer & double in field storage 
  -[OK] serialize database schema
  -[OK] Save new record into file 
  -[OK]A hidden storage index, also known as an internal index, is a type of index that is used by the database management system to efficiently locate the location in a file where a specific row is stored. In the case of rurudb, it is an index that is created on the Record.row_id field, which allows the system to quickly locate a specific record in storage.
  -[OK] compact schema as database file
  -[OK] use filesystem path 
  -[] remove pointers from public API --> to analyze 
  -[OK] hide StorageEngine & Database impl
  -[OK] one single public include
  -[OK] static library

  how the storage engine must works?
  - 2 kind of records: newly one, and modified one
  
  database folder structure:
   - folder name = database name
   - structure is stored into a table file (<db_name>.ru)
   - each table have it's data file (<table_name>.ru)

 user defined storage engine