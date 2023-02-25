# About

``rurudb`` is an excellent choice for those who want to learn more about database internals. As a joy database, it offers a simple and intuitive interface that makes it easy to experiment with different concepts and approaches. The codebase is well-structured and easy to read, which is essential when trying to understand how databases work under the hood. By diving into the inner workings of ``rurudb``, users can gain a deeper understanding of how databases handle data storage, indexing, and query execution. Overall, ``rurudb`` is a great tool for those who want to expand their knowledge of databases and experiment with new ideas in a safe and controlled environment.

# How to Build

```shell
mkdir build
cd build 
conan install ..
cmake ..
make 
```

# Requirements

| `conan` | cross-platform dependencies management system |
| --------- | --------------------------------------------- |
| `cmake` | cross-platform build system                   |
| `c++17` | c++17 compiler                                |



# Features

- [X] Basic Storage Engine
- [X] Schema
- [X] Data Persistance
- [X] Filters
- [X] Index
- [X] Data Cache (in progress)
- [ ] Multi-threading
- [ ] Journal File
- [ ] Transactions
- [ ] Tests

# Directory Structure
rurudb
├── src
├── repl
├── tests
├── example
├── documentation
└── include
    └── ruru.h

The code for the project is well-organized into several directories, each with its own distinct purpose. The `src` folder contains all of the source code for ``rurudb``, including its implementation and any supporting code. Meanwhile, the `repl` directory is a work-in-progress REPL (Read-Evaluate-Print-Loop) program, designed to give developers a way to experiment with rurudb's capabilities and features.

For testing purposes, the `tests` directory is where all of the C++ tests for noregressions can be found. This ensures that any changes or updates made to the codebase don't negatively impact existing functionality or introduce new bugs.

The `example` directory contains an example program that makes use of ``rurudb``, providing developers with a practical demonstration of how the database can be used in real-world scenarios.

In addition to the source code, the project also includes a `documentation` directory that is a work-in-progress. This will eventually contain comprehensive documentation on how to use and interact with ``rurudb``.

Finally, the `include` directory contains all of the public APIs (i.e `ruru.h`), which can be included and utilized by external applications and codebases. The organization of these directories makes it easy for developers to navigate the project and locate the code they need for their specific use cases.

