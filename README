Name: Šimon Benčík
Login: xbenci01
Date: 20.11.2023

Description: Implementation of simple LDAP server, which allows searching records in csv files.

Usage: ./isa-ldapserver {-p <port>} -f <file>
(it is possible to use make run, which will run server on port 389 and use file ./resources/lidi.csv)

Known limitations:
- Supports only ascii encoded csv files
- Search does not support attributes
- Server ignores invalid requests instead of sending error messages to client
- Server is case sensitive - attributes and values

Structure:
- src/ - contains source files
- include/ - contains header files
- resources/ - contains csv files
- doc/ - When make doc is called, doxygen generates documentation in this folder

Submitted files:
- src/
  - main.cpp
  - ber.cpp
  - message.cpp
  - search.cpp
- include/
  - ber.h
  - message.h
  - search.h
- resources/
  - lidi.csv
- Makefile
- README
- manual.pdf
- manual.md
- doxyfile
