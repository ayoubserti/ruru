database
 - table
    - column
    - RecordTable
 - StorageEngine  -> file
   - Record
   - RecordFile
   - Field

    1- Create a new database from JSON schema
    2- Persiste Database table into the disc
    3- Create Record and populate values
    4- Save Record 
    5- Open Database from disq file
    6- Search record 
    7- Manage Indexes
    8- Cache
    9- Log files and transactions
   10- Allocator & memory management



In Progress:
  -[OK] fix Integer & double in field storage 
  -[OK] serialize database schema
  -[OK] Save new record into file 
  -[OK]A hidden storage index, also known as an internal index, is a type of index that is used by the database management system to efficiently locate the location in a file where a specific row is stored. In the case of rurudb, it is an index that is created on the Record.row_id field, which allows the system to quickly locate a specific record in storage.


  how the storage engine must works?
  - 2 kind of records: newly one, and modified one
  - 