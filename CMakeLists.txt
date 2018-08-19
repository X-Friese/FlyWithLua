cmake_minimum_required(VERSION 3.9)
project(FlyWithLua VERSION 2.7.1 DESCRIPTION "FlyWithLua X-Plane plugin")

include_directories("${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/Widgets")
include_directories("${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/Wrappers")
include_directories("${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/XPLM")
include_directories("${CMAKE_SOURCE_DIR}/src/include64")
include_directories("${CMAKE_SOURCE_DIR}/src/hidapi")
include_directories("${CMAKE_SOURCE_DIR}/src/OpenAL")
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src/FloatingWindows")
include_directories("${CMAKE_SOURCE_DIR}/src/imgui")

if(WIN32)
	include_directories("${CMAKE_SOURCE_DIR}/src/GLUT_for_Windows")
endif(WIN32)

if(WIN32)
	link_directories("${CMAKE_SOURCE_DIR}/src/XPSDK301/Libraries/Win")
	link_directories("${CMAKE_SOURCE_DIR}/src/include64")
	link_directories("${CMAKE_SOURCE_DIR}/src/GLUT_for_Windows")

endif(WIN32)

if(APPLE)
	link_directories("${CMAKE_SOURCE_DIR}/src/mac_libs")    
elseif(UNIX)
	link_directories("${CMAKE_SOURCE_DIR}/src/linux_libs/luaXML/64")
	link_directories("${CMAKE_SOURCE_DIR}/src/linux_libs/glut/64")
endif()

add_subdirectory(src)
