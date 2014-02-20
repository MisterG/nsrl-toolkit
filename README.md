nsrl-toolkit
============

Tools to manage the National Software Reference Library database

The binaries
------------

Three QMake projects are available:

* rds_import: reads the RDS files and inserts them into a database
* rds_benchmark: intends to parse the whole database looking for checksums. It provides some benchmark data
* nsrl_products: gives the product the checksum is part of

They behave like standard UNIX commands, ie. the open the standard input and process data. They use a configuration file or command-line arguments.


Requirements
------------

The project uses QMake to create the Makefiles. It is written in C++. The required libraries are:

* QMap
* QRegExp
* QSettings
* QSqlDatabase
* QSqlDriver
* QSqlField
* QString
* QStringBuilder
* QStringList
* QtGlobal
* QtSql
* cstdlib
* iostream
* stdio.h
* unistd.h

Building
--------

$ qmake rds_import.pro && make

$ qmake rds_benchmark.pro && make

$ qmake nds_products.pro && make
