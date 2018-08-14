cmake_minimum_required(VERSION 3.9)
project(FlyWithLua VERSION 2.7.0 DESCRIPTION "FlyWithLua X-Plane plugin")


if(WIN32)
    link_directories("/opt/XSDK/Libraries/Win")
    link_directories("/usr/x86_64-w64-mingw32/lib")
endif(WIN32)

if(APPLE)
    
elseif(UNIX)
    
endif()

message("The full path to the CMAKE_SOURCE_DIR  folder is.") 
message(${CMAKE_SOURCE_DIR})


add_subdirectory(src)
