-- This is a test case for basic LuaSocket functionality.
--
-- This script is intentionally designed to crash Lua in case of any error, if
-- it didn't crash, LuaSocket works (at least in its core parts).
--
-- See http://w3.impa.br/~diego/software/luasocket/reference.html for detailed
-- LuaSocket documentation.
local socket = require "socket"
local http = require "socket.http"

-- Test low-level TCP API: create a listening socket, connect to it and close
-- both.
local server, err = socket.bind("*", 0)
assert(server, err)
local _, port = server:getsockname()
local client, err = socket.connect("127.0.0.1", port)
assert(client, err)
server:close()
client:close()

-- Test high-level HTTP client API: fetch VATSIM METAR for the nearest airport.
local ICAO = select(7, XPLMGetNavAidInfo(XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport)))
local metar, err = http.request("http://metar.vatsim.net/metar.php?id="..ICAO)
assert(metar, err)
assert(
  metar:sub(1, 4) == ICAO,
  string.format("Got METAR response: %s, want METAR starts with ICAO %s", metar, ICAO))

print("PASSED: " .. metar:gsub("%s+$", ""))
