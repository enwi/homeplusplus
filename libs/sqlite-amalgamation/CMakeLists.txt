cmake_minimum_required(VERSION 2.8.3)
project(sqlite3)

set(sqlite3_SOURCES sqlite3.c)
add_library(sqlite3 STATIC ${sqlite3_SOURCES})
install(TARGETS sqlite3 DESTINATION lib)