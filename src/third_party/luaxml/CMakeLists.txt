cmake_minimum_required(VERSION 3.9)
project(LuaXML VERSION 10.10.12.2 DESCRIPTION "A module that maps between Lua and XML without much ado.")

add_library(LuaXML_lib STATIC upstream/LuaXML_lib.c)
# ***** This was used for building LuaJIT 2.0.5
# target_link_libraries(LuaXML_lib luajit::LuaJIT)
# *** We are using this to build LuaJIT 2.1.0 beta3
target_link_libraries(LuaXML_lib ${LUAJIT_LIBRARY})
