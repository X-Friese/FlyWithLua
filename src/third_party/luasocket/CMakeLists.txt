cmake_minimum_required(VERSION 3.9)
project(LuaSocket VERSION 10.10.12.2 DESCRIPTION "A module that maps between Lua and XML without much ado.")

set(LUASOCKET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/upstream/src)

# "mime/core" module.
add_library(LuaSocket_Mime STATIC
  ${LUASOCKET_SRC_DIR}/mime.c
  ${LUASOCKET_SRC_DIR}/compat.c)
# target_link_libraries(LuaSocket_Mime luajit::LuaJIT)
target_link_libraries(LuaSocket_Mime ${LUAJIT_LIBRARY})

# "socket/core" module.
add_library(LuaSocket_Socket STATIC
  ${LUASOCKET_SRC_DIR}/luasocket.c
  ${LUASOCKET_SRC_DIR}/timeout.c
  ${LUASOCKET_SRC_DIR}/buffer.c
  ${LUASOCKET_SRC_DIR}/io.c
  ${LUASOCKET_SRC_DIR}/auxiliar.c
  ${LUASOCKET_SRC_DIR}/compat.c
  ${LUASOCKET_SRC_DIR}/options.c
  ${LUASOCKET_SRC_DIR}/inet.c
  ${LUASOCKET_SRC_DIR}/except.c
  ${LUASOCKET_SRC_DIR}/select.c
  ${LUASOCKET_SRC_DIR}/tcp.c
  ${LUASOCKET_SRC_DIR}/udp.c)
# target_link_libraries(LuaSocket_Socket luajit::LuaJIT)
target_link_libraries(LuaSocket_Socket ${LUAJIT_LIBRARY})

if(WIN32)
  target_sources(LuaSocket_Socket PRIVATE ${LUASOCKET_SRC_DIR}/wsocket.c)
  target_link_libraries(LuaSocket_Socket ws2_32)
else()
  target_sources(LuaSocket_Socket PRIVATE ${LUASOCKET_SRC_DIR}/usocket.c)
endif()

# "socket/unix" module.
if(UNIX)
  add_library(LuaSocket_Unix STATIC
    ${LUASOCKET_SRC_DIR}/buffer.c
    ${LUASOCKET_SRC_DIR}/auxiliar.c
    ${LUASOCKET_SRC_DIR}/options.c
    ${LUASOCKET_SRC_DIR}/timeout.c
    ${LUASOCKET_SRC_DIR}/io.c
    ${LUASOCKET_SRC_DIR}/usocket.c
    ${LUASOCKET_SRC_DIR}/unixstream.c
    ${LUASOCKET_SRC_DIR}/unixdgram.c
    ${LUASOCKET_SRC_DIR}/compat.c
    ${LUASOCKET_SRC_DIR}/unix.c)
#  target_link_libraries(LuaSocket_Unix luajit::LuaJIT)
  target_link_libraries(LuaSocket_Unix ${LUAJIT_LIBRARY})
else()
  # Add a stub, since it's not supported on non-UNIX platforms.
  add_library(LuaSocket_Unix INTERFACE)
endif()

if(UNIX)
  add_library(LuaSocket_Serial STATIC
    ${LUASOCKET_SRC_DIR}/buffer.c
    ${LUASOCKET_SRC_DIR}/auxiliar.c
    ${LUASOCKET_SRC_DIR}/options.c
    ${LUASOCKET_SRC_DIR}/timeout.c
    ${LUASOCKET_SRC_DIR}/io.c
    ${LUASOCKET_SRC_DIR}/usocket.c
    ${LUASOCKET_SRC_DIR}/serial.c)
#  target_link_libraries(LuaSocket_Serial luajit::LuaJIT)
  target_link_libraries(LuaSocket_Serial ${LUAJIT_LIBRARY})
  target_compile_definitions(LuaSocket_Serial PRIVATE LUASOCKET_API=extern)
else()
  # Add a stub, since it's not supported on non-UNIX platforms.
  add_library(LuaSocket_Serial INTERFACE)
endif()


add_library(LuaSocket::Mime ALIAS LuaSocket_Mime)
add_library(LuaSocket::Socket ALIAS LuaSocket_Socket)
add_library(LuaSocket::Unix ALIAS LuaSocket_Unix)
add_library(LuaSocket::Serial ALIAS LuaSocket_Serial)
