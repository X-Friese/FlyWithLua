set(CMAKE_CXX_STANDARD 14)

if(Windows)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wshadow -Wfloat-equal -pedantic -w -std=c++11 -fexceptions -fpermissive")
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wshadow -Wfloat-equal -pedantic -w -std=c++11 -fexceptions -fpermissive")
elseif(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC -Wno-reorder")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=gnu++11 -fpermissive -O2 -Wall -W -D_REENTRANT -fPIC")
endif()


include_directories("${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/XPLM" "${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/Wrappers" "${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders/Widgets" "${CMAKE_SOURCE_DIR}/src/XPSDK301/CHeaders")
include_directories("../src" "../src/FloatingWindows" "../src/imgui" "hidapi" "OpenAL" "../" "include64")
if(WIN32)
include_directories("GLUT_for_Windows")
endif()


if(WIN32)
	add_library(FWL_XPLM_64 UNKNOWN IMPORTED)
	set_target_properties(FWL_XPLM_64 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/XPSDK301/Libraries/Win/XPLM_64.lib")
	add_library(FWL_XPWidgets_64 UNKNOWN IMPORTED)
	set_target_properties(FWL_XPWidgets_64 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/XPSDK301/Libraries/Win/XPWidgets_64.lib")

	add_library(FWL_lua51 UNKNOWN IMPORTED)
	set_target_properties(FWL_lua51 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/include64/lua51.lib")
	add_library(FWL_OpenGL32 UNKNOWN IMPORTED)
	set_target_properties(FWL_OpenGL32 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/include64/OpenGL32.lib")
	add_library(FWL_OpenAL32 UNKNOWN IMPORTED)
	set_target_properties(FWL_OpenAL32 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/include64/OpenAL32.lib")

	add_library(FWL_glut64 UNKNOWN IMPORTED)
	set_target_properties(FWL_glut64 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/GLUT_for_Windows/GL/glut64.lib")

elseif(APPLE)

elseif(UNIX)

	
endif()


if(WIN32)
	add_library(FlyWithLua SHARED FlyWithLua.cpp FloatingWindows/FLWIntegration.cpp FloatingWindows/ImGUIIntegration.cpp FloatingWindows/FloatingWindow.cpp imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_lua_bindings.cpp hidapi/windows/hid.c)
elseif(APPLE)
	add_library(FlyWithLua SHARED FlyWithLua.cpp FloatingWindows/FLWIntegration.cpp FloatingWindows/ImGUIIntegration.cpp FloatingWindows/FloatingWindow.cpp imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_lua_bindings.cpp hidapi/mac/hid.c)
elseif(UNIX)
	add_library(FlyWithLua SHARED FlyWithLua.cpp FloatingWindows/FLWIntegration.cpp FloatingWindows/ImGUIIntegration.cpp FloatingWindows/FloatingWindow.cpp imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_lua_bindings.cpp hidapi/linux/hid.c)
endif()


if(WIN32)
	target_link_libraries(FlyWithLua --shared -static -static-libgcc -static-libstdc++ setupapi FWL_XPLM_64 FWL_XPWidgets_64 FWL_lua51 FWL_OpenGL32 FWL_glut64 FWL_OpenAL32)
elseif(APPLE)
	link_directories("/opt/XSDK/Libraries/Mac")
	target_link_libraries(FlyWithLua "-framework XPLM")
elseif(UNIX)
	target_link_libraries(FlyWithLua -Wl,-rpath=./Resources/plugins/FlyWithLua/64 -Wl,-rpath=./Resources/plugins/FlyWithLua/Modules -static-libgcc -static-libstdc++ -ldl -Wl,--version-script -Wl,${CMAKE_SOURCE_DIR}/src/flywithlua.sym -L${CMAKE_SOURCE_DIR}/src/linux_libs/luaXML/64 -lLuaXML_lib_64 -L${CMAKE_SOURCE_DIR}/src/linux_libs/glut/64 -lglut ${CMAKE_SOURCE_DIR}/src/linux_libs/luajit/64/libluajit-5.1.a -lGLU -lGL -ludev)
endif()


if(win32)
	set_target_properties(FlyWithLua PROPERTIES PREFIX "")
	set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "win")
	set_target_properties(FlyWithLua PROPERTIES SUFFIX ".xpl")
elseif(APPLE)
	set_target_properties(FlyWithLua PROPERTIES PREFIX "")
	set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "mac")
	set_target_properties(FlyWithLua PROPERTIES SUFFIX ".xpl")
elseif(UNIX)
	set_target_properties(FlyWithLua PROPERTIES PREFIX "")
	set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "lin")
	set_target_properties(FlyWithLua PROPERTIES SUFFIX ".xpl")
endif()


# X-Plane stuff
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1")
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN64 -DNDEBUG -D_WINDOWS -D_MBCS -D_USRDLL -DCustomCommands_EXPORTS")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSIMDATA_EXPORTS -DUSE_EXTERNAL_ALLOCATOR -DCREATECOMPLETEEDITION")
elseif(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCREATECOMPLETEEDITION")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCREATECOMPLETEEDITION")
endif()


if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=0 -DIBM=1 -DLIN=0")
elseif(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=1 -DIBM=0 -DLIN=0")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=0 -DIBM=0 -DLIN=1")
endif()
