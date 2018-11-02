--------------------------------
-- Script:  jjjLib1
-- Version: 1.7
-- Date:    2018-04-21
-- By:      3j@raeuber.com
--------------------------------
module(..., package.seeall)

jjjLib1Installed = true
jjjLib1Version = 1.7

require("graphics")

local ffi = require("ffi")
local XPLM = nil

local plIdSequence = 0

local alertsPlId = false

local anyPanelDraw        = false
local anyPanelDrawCheck   = false

local pluginName = {}
local warnings   = {}

local panelSize       = 14 -- size factor for panel (=line height)
local panelLineHeight = panelSize
local panelPadding    = panelSize*0.5  -- padding in panel

local panelState      = {}
local panelX          = {}
local panelY          = {}
local panelPosX       = {}
local panelPosY       = {}
local panelAlpha      = {}
local panelBgAlpha    = {}
local panelColor      = {}
local panelW          = {}
local panelH          = {}
local labelW          = {}
local panelShadow     = {}
local panelName       = {}
local panelItems      = {}
local panelRefreshAction = {}

local alertPanelCount = 0

local curX    = {}
local curY    = {}
local indentX = {}
local sldrLegend  = {}
local pnCtrlCount = {}

local aircraft = false

dataref("jjjLib1_clock", "sim/network/misc/network_time_sec", "readonly")
local currentClock = jjjLib1_clock
local frameSec     = 0.01

local panelFadeSpd = 5

local flashTime = 0.36
local flashRectTimer = {}
local flashRectX1 = {}
local flashRectY1 = {}
local flashRectX2 = {}
local flashRectY2 = {}

-- initialize generic parameters
local params = {}
local savedParams = {}
local saveScopes = {}
local saveCustomPath = {}
local saveCustomName = {}
local paramsActionOnSet = {}
local numMissingDRs = {}
local curParamProfile = {}


------------------------------------------------------------------------------------------------
-- Public functions (global, not plugin specific)
------------------------------------------------------------------------------------------------
function version()
	return jjjLib1Version
end

function getClock()
	return jjjLib1_clock
end

function fileExists(filePath)
	local file = io.open(filePath, "r")
	if not file then
		return false
	end
	file:close()
	return true
end

function getFileSize(filePath)
	local file = io.open(filePath, "r")
	if not file then
		return false
	end
	local size = file:seek("end") 
	file:close()
	return size
end

function fileCopy(filePathSrc, filePathDest)
	local destFile = io.open(filePathDest, "w")
	if not destFile then
		return false
	end
	local numLines = 0
	for line in io.lines(filePathSrc) do
		destFile:write(line, "\n")
		numLines = numLines + 1
	end
	destFile:close()
	if numLines < 1 then
		return false
	end
	if math.abs(getFileSize(filePathDest) - getFileSize(filePathSrc)) > numLines*4 then
		return false
	end
	return true
end

function arrayCopy(arrFrom)
	local arrTo = {}
	for i, val in pairs(arrFrom) do
		if type(val) == "table" then
			arrTo[i] = arrayCopy(val)
		else
	    arrTo[i] = val
	   end
  end
  return arrTo
end

function trim(string)
	return (string:gsub("^%s*(.-)%s*$", "%1"))
end

function split(string, separator, maxParts, keepSeparators, ignoreEmpty)
	local start = 1
	local stop  = 1
	local pos   = 0
	local parts = {}
	local sepLen = 0
	local keepSep
	if keepSeparators then
		keepSep = 1
	else
		keepSep = 0
	end
	local extra
	if separator[1] then
		stop = 999999
		for i, s in ipairs(separator) do
			pos = string.find(string, s, start, true)
			if pos and pos < stop then
				stop = pos
				sepLen = string.len(s)
			end
		end
		while stop < 999999 and ( not maxParts or #parts < maxParts) do
			if keepSeparators or not ignoreEmpty or stop - start > 0 then
				parts[#parts + 1] = string.sub(string, start, stop - 1 + keepSep*sepLen)
			end
			start = stop + sepLen
			stop = 999999
			for i, s in ipairs(separator) do
				pos = string.find(string, s, start, true)
				if pos then
					stop = math.min(stop, pos)
				end
			end
		end
	else
		sepLen = string.len(separator)
		stop = string.find(string, separator, start, true)
		while stop and ( not maxParts or #parts < maxParts) do
			if keepSeparators or not ignoreEmpty or stop - start > 0 then
				parts[#parts + 1] = string.sub(string, start, stop - 1 + keepSep*sepLen)
			end
			start = stop + sepLen
			stop = string.find(string, separator, start, true)
		end
	end
	parts[#parts + 1] = string.sub(string, start)
	return parts
end

function getAircraft()
	return aircraft
end

function binToNumber(string)
	return string.byte(string, 1) + string.byte(string, 2)*256 + string.byte(string, 3)*65536 + string.byte(string, 4)*16777216
end

function getWavFileLength(filePath)
	local file = io.open(filePath, "rb")
	local d = 0
	local size = 0
	local byteRate = 0
	if not file then 
		return false 
	end
	if file:read(4) ~= "RIFF" then 
		file:close()
		return false
	end
	size = binToNumber(file:read(4))
	if file:read(4) ~= "WAVE" then 
		file:close()
		return false
	end
	if file:read(4) ~= "fmt " then
		file:close()
		return false
	end
	file:read(12)
	byteRate = binToNumber(file:read(4))
	file:close()
	return (size - 42)/byteRate
end

-- special functions to patch other Scripts to avoid conflicts (e.g. RTH 4.0)
function checkPatchLuaScript(scriptFilePath, pluginName)
	file = io.open(scriptFilePath, "r")
	if not file then
		return false
	end
	local line = file:read()
	file:close()
	local patchMark = "#jjjLib1.patch[" .. pluginName .. "]#"
	if string.find(line, patchMark, 3, true) then
		return true
	end
	return false
end

function patchLuaScript(scriptFilePath, searchStrings, includeCondition, patchInfo, pluginName)
	local backupFile = scriptFilePath .. "_jjjLib_BACKUP"
	local backupFileSize = getFileSize(backupFile)
	if backupFileSize == false or backupFileSize < 1 then
		if not fileCopy(scriptFilePath, backupFile) then
			return false
		end
	end
	local tempFile = scriptFilePath .. "_jjjLib_TEMP"
	if not fileCopy(scriptFilePath, tempFile) then
		return false
	end
	
	destFile = io.open(scriptFilePath, "w")
	if not destFile then
		return false
	end
	local numLines      = 0
	local commentPos    = 0
	local lineNoComment = ""
	local comment       = ""
	local patchMark     = "#jjjLib1.patch[" .. pluginName .. "]#"
	local patchedLines  = 0
	local firstLine     = true
	for line in io.lines(tempFile) do
		if firstLine then
			firstLine = false
			if string.sub(trim(line), 1, 17) == "-- #jjjLib1.patch" then
				if not string.find(line, patchMark, 3, true) then
					line = line .. " " .. patchMark
				end
			else
				destFile:write("-- " .. patchMark, "\n")
			end
		end
		if string.sub(trim(line), 1, 2) ~= "--" and not string.find(line, patchMark, 1, true) then
			commentPos = string.find(line, "--", 2, true)
			if commentPos then
				lineNoComment = string.sub(line, 1, commentPos - 1)
				comment       = trim(string.sub(line, commentPos + 2)) .. " "
			else
				lineNoComment = line
				comment       = ""
			end
			for i, search in pairs(searchStrings) do
				if string.find(lineNoComment, search, 1, true) then
					line = "if " .. includeCondition .. " then " .. trim(lineNoComment) .. " end -- " .. comment .. patchInfo .. " " .. patchMark
					patchedLines = patchedLines + 1
					break
				end
			end
		end
		destFile:write(line, "\n")
		numLines = numLines + 1
	end
	destFile:close()
	if patchedLines < 1 then
		if os.remove(scriptFilePath) then
			if not os.rename(tempFile, scriptFilePath) then
				return false
			end
		else
			return false
		end
	end
	return patchedLines
end

------------------------------------------------------------------------------------------------
-- Plugin specific public functions
------------------------------------------------------------------------------------------------

function getPlId(plugin)
	plIdSequence = plIdSequence + 1
	local plId = plIdSequence
	pluginName[plId]  = plugin
	warnings[plId]    = nil
	panelState[plId]  = {}
	panelX[plId]      = {}
	panelY[plId]      = {}
	panelPosX[plId]   = {}
	panelPosY[plId]   = {}
	panelAlpha[plId]  = {}
	panelBgAlpha[plId]= 0.85
	panelColor[plId]  = {}
	panelW[plId]      = {}
	panelH[plId]      = {}
	labelW[plId]      = {}
	panelShadow[plId] = {}
	panelName[plId]   = {}
	panelItems[plId]  = {}
	panelRefreshAction[plId] = {}
	curX[plId]        = {}
	curY[plId]        = {}
	indentX[plId]     = {}
	sldrLegend[plId]  = {}
	pnCtrlCount[plId] = {}
	params[plId]      = {}
	savedParams[plId] = {}
	saveScopes[plId] = {}
	saveCustomPath[plId] = SCRIPT_DIRECTORY
	saveCustomName[plId] = plugin .. "-custom"
	paramsActionOnSet[plId] = false
	curParamProfile[plId] = "A"
	numMissingDRs[plId] = 0
	flashRectTimer[plId] = {}
	flashRectX1[plId] = {}
	flashRectY1[plId] = {}
	flashRectX2[plId] = {}
	flashRectY2[plId] = {}
	return plId
end

function warning(plId, text, distinctAlert)
	if distinctAlert then
		alert(plId, 'WARNING', text, 'white', 'OK')
	else
		if warnings[plId] ~= nil then
			warnings[plId] = warnings[plId] .. '||'
		else
			warnings[plId] = ""
		end
		warnings[plId] = warnings[plId] .. text
		anyPanelDraw = true
	end
end

-- DataRef functions
function dataRefAvailable(plId, dataRefName, warnMessageIfFailed)
	if XPLM.XPLMFindDataRef(dataRefName) ~= nil then
		return true
	else
		if warnMessageIfFailed then
			local text = 'DataRef "' .. dataRefName .. '" is missing!'
			warning(plId, text)
		end
		numMissingDRs[plId] = numMissingDRs[plId] + 1
		return false
	end
end

function getNumMissingDataRefs(plId)
	return numMissingDRs[plId]
end

function findDataRef(dataRefName)
	if XPLM then
		return XPLM.XPLMFindDataRef(dataRefName)
	end
	return nil
end

function getDataRef_i(dataRef)
	if dataRef ~= nil then
		return XPLM.XPLMGetDatai(dataRef)
	end
end
function getDataRef_f(dataRef)
	if dataRef ~= nil then
		return XPLM.XPLMGetDataf(dataRef)
	end
end
function getDataRef_d(dataRef)
	if dataRef ~= nil then
		return XPLM.XPLMGetDatad(dataRef)
	end
end

function setDataRef_i(dataRef, value)
	if dataRef ~= nil then
		XPLM.XPLMSetDatai(dataRef, value)
	end
end
function setDataRef_f(dataRef, value)
	if dataRef ~= nil then
		XPLM.XPLMSetDataf(dataRef, value)
	end
end
function setDataRef_d(dataRef, value)
	if dataRef ~= nil then
		XPLM.XPLMSetDatad(dataRef, value)
	end
end


-- Parameter management functions
function isAnyParamUnsaved(plId)
	for param, meta in pairs(params[plId]) do
    if meta["save"] and savedParams[plId][param] ~= nil and savedParams[plId][param]["val"] ~= meta["val"] then return true end
  end
  return false
end

function isAnyParamNotDefault(plId)
	for param, meta in pairs(params[plId]) do
    if meta["dflt"] ~= nil and meta["val"] ~= meta["dflt"] then return true end
  end
  return false
end

function setParamsDefault(plId)
	for param, meta in pairs(params[plId]) do
		if meta["dflt"] ~= nil then
			setParam(plId, param, params[plId][param]["dflt"])
		end
	end
end

function addParam(plId, param, metaArray, actionOnSet)
	params[plId][param] = metaArray
	params[plId][param]["val"] = metaArray["dflt"]
	if metaArray["save"] or metaArray["autosave"] then
		local saveScope = metaArray["save"]
		if not saveScope or saveScope == "" then
			saveScope = "global"
		end
		saveScope = string.lower(saveScope)
		params[plId][param]["save"] = saveScope
		if saveScope == "global" or saveScope == "global-profile" or saveScope == "aircraft" or saveScope == "aircraft-profile" or saveScope == "custom" or saveScope == "custom-profile" then
			saveScopes[plId][saveScope] = true
		end
	end
	if actionOnSet and actionOnSet ~= "" then
		params[plId][param]["action"] = loadstring(actionOnSet)
	end
end

function setParam(plId, param, value)
	local parMn
	local parMx
	local diff
	params[plId][param]["val"] = value
	if params[plId][param]["parMn"] then
		parMn = params[plId][param]["parMn"]
		diff  = params[plId][param]["parMnDiff"]
		if diff then diff = diff*getParamStep(plId, param) else diff = getParamStep(plId, param) end
		if parMn and params[plId][parMn] and value < params[plId][parMn]["val"] + diff then
			params[plId][parMn]["val"] = value - diff
		end
	end
	if params[plId][param]["parMx"] then
		parMx = params[plId][param]["parMx"]
		diff  = params[plId][param]["parMxDiff"]
		if diff then diff = diff*getParamStep(plId, param) else diff = getParamStep(plId, param) end
		if parMx and params[plId][parMx] and value > params[plId][parMx]["val"] - diff then
			params[plId][parMx]["val"] = value + diff
		end
	end
	if params[plId][param]["action"] and params[plId][param]["action"] ~= "" then
		params[plId][param]["action"]()
	end
	if paramsActionOnSet[plId] then
		paramsActionOnSet[plId]()
	end
end

function setParamsActionOnSet(plId, actionCode)
	if actionCode and actionCode ~= "" then
		paramsActionOnSet[plId] = loadstring(actionCode)
	end
end
function setParamDefault(plId, param, defaultVal)
	params[plId][param]["dflt"] = defaultVal
end

function getParam(plId, param)
	return params[plId][param]["val"]
end
function getParamDefault(plId, param)
	return params[plId][param]["dflt"]
end
function getParamMin(plId, param)
	return params[plId][param]["mn"]
end
function getParamMax(plId, param)
	return params[plId][param]["mx"]
end
function getParamStep(plId, param)
	return params[plId][param]["stp"]
end
function getParamRealStep(plId, param)
	if params[plId][param]["slMn"] < params[plId][param]["slMx"] then
		return params[plId][param]["stp"]
	else
		return -params[plId][param]["stp"]
	end
end
function getParamSliderMin(plId, param)
	return params[plId][param]["slMn"]
end
function getParamSliderMax(plId, param)
	return params[plId][param]["slMx"]
end
function getParamGoodMin(plId, param)
	return params[plId][param]["gdMn"]
end
function getParamGoodMax(plId, param)
	return params[plId][param]["gdMx"]
end
function getParamFormat(plId, param)
	return params[plId][param]["fmt"]
end

function setCustomPrefsPath(plId, path)
	saveCustomPath[plId] = path
end

function setCustomPrefsName(plId, name)
	saveCustomName[plId] = name
end

function setParamProfile(plId, profile)
	if profile ~= nil and profile ~= false and profile ~= "" then
		curParamProfile[plId] = profile
	end
end


------------------------------------------------------------------------------------------------
-- private functions (global, not plugin specific)
------------------------------------------------------------------------------------------------
function loadXPLMlib()
	local XPLMlib = ""
	if SYSTEM_ARCHITECTURE == 64 then
		if SYSTEM == "IBM" then
			XPLMlib = "XPLM_64"
		elseif SYSTEM == "LIN" then
			XPLMlib = "Resources/plugins/XPLM_64.so"
		else
			XPLMlib = "Resources/plugins/XPLM.framework/XPLM"
		end
	else
		if SYSTEM == "IBM" then
			XPLMlib = "XPLM"
		elseif SYSTEM == "LIN" then
			XPLMlib = "Resources/plugins/XPLM.so"
		else
			XPLMlib = "Resources/plugins/XPLM.framework/XPLM"
		end
	end
	XPLM = ffi.load(XPLMlib)
	-- define the functions
	ffi.cdef("typedef void * XPLMDataRef;")
	ffi.cdef("XPLMDataRef XPLMFindDataRef(const char * inDataRefName);")
	ffi.cdef("int XPLMGetDatai(XPLMDataRef inDataRef);")
	ffi.cdef("void XPLMSetDatai(XPLMDataRef inDataRef, int inValue);")
	ffi.cdef("float XPLMGetDataf(XPLMDataRef inDataRef);")
	ffi.cdef("void XPLMSetDataf(XPLMDataRef inDataRef, float inValue);")
	ffi.cdef("double XPLMGetDatad(XPLMDataRef inDataRef);")
	ffi.cdef("void XPLMSetDatad(XPLMDataRef inDataRef, double inValue);")
end


------------------------------------------------------------------------------------------------
-- Plugin specific private functions
------------------------------------------------------------------------------------------------

local function saveParamsScope(plId, scope)
	local file = false
	local profile = ""
	profile = "-" .. curParamProfile[plId]
	if scope == "global" then
		file = io.open(SCRIPT_DIRECTORY .. pluginName[plId] .. ".prf", "w")
	elseif scope == "global-profile" then
		file = io.open(SCRIPT_DIRECTORY .. pluginName[plId] .. profile .. ".prf", "w")
	elseif scope == "aircraft" then
		file = io.open(AIRCRAFT_PATH .. pluginName[plId] .. ".prf", "w")
	elseif scope == "aircraft-profile" then
		file = io.open(AIRCRAFT_PATH .. pluginName[plId] .. profile .. ".prf", "w")
	elseif scope == "custom" then
		file = io.open(saveCustomPath[plId] .. saveCustomName[plId] .. ".prf", "w")
	elseif scope == "custom-profile" then
		file = io.open(saveCustomPath[plId] .. saveCustomName[plId] .. profile .. ".prf", "w")
	end
	if not file then
		alert(plId, "ERROR!", "Could not write preferences file .prf! (scope = " .. scope .. ")", "white", "Ok")
		return false
	end
	file:write("# " .. scope .. " preferences of FlyWithLua plugin '" .. pluginName[plId] .. "' #\n")
	for param, meta in pairs(params[plId]) do
		if meta["save"] == scope then
			file:write(param .. "=" .. string.sub(type(meta["val"]), 1, 1) .. ":" .. tostring(meta["val"]), "\n")
		end
  end
	file:close()
	return true
end

function saveParams(plId, singleScope)
	local ok = true
	if singleScope then
		ok = saveParamsScope(plId, singleScope)
	else
		for scope, inUse in pairs(saveScopes[plId]) do
			if inUse then
				ok = ok and saveParamsScope(plId, scope)
			end
		end
	end
	if ok then
		savedParams[plId] = arrayCopy(params[plId])
	end
	return ok
end

local function loadParamsScope(plId, scope)
	local pos
	local param
	local value
	local type

	local file = false
	local profile = ""
	profile = "-" .. curParamProfile[plId]
	if scope == "global" then
		file = io.open(SCRIPT_DIRECTORY .. pluginName[plId] .. ".prf", "r")
	elseif scope == "global-profile" then
		file = io.open(SCRIPT_DIRECTORY .. pluginName[plId] .. profile .. ".prf", "r")
	elseif scope == "aircraft" then
		file = io.open(AIRCRAFT_PATH .. pluginName[plId] .. ".prf", "r")
	elseif scope == "aircraft-profile" then
		file = io.open(AIRCRAFT_PATH .. pluginName[plId] .. profile .. ".prf", "r")
	elseif scope == "custom" then
		file = io.open(saveCustomPath[plId] .. saveCustomName[plId] .. ".prf", "r")
	elseif scope == "custom-profile" then
		file = io.open(saveCustomPath[plId] .. saveCustomName[plId] .. profile .. ".prf", "r")
	end
	if not file then
		return false
	end
	for line in file:lines() do
		pos = string.find(line, "=", 2, true)
		if pos then
			param = string.sub(line, 1, pos - 1)
			type  = string.sub(line, pos + 1, pos + 1)
			value = string.sub(line, pos + 3)
			if params[plId][param] then
				if type == "t" then
					alert(plId, "ERROR!", "Preferences file .prf is corrupt! (scope = " .. scope .. ") Some preferences are set to default", "white", "Ok")
				elseif type == "s" then
					params[plId][param]["val"] = value
				elseif type == "b" then
					params[plId][param]["val"] = ( value == "true" )
				else
					params[plId][param]["val"] = tonumber(value)
				end
			end
		end
	end
	file:close()
	return true
end

function loadParams(plId, singleScope)
	local ok = true
	if singleScope then
		ok = loadParamsScope(plId, singleScope)
	else
		for scope, inUse in pairs(saveScopes[plId]) do
			if inUse then
				ok = ok and loadParamsScope(plId, scope)
			end
		end
	end
	savedParams[plId] = arrayCopy(params[plId])
	return ok
end

function getRGBA(color, hi, dark, line, disabled, alpha)
	local a = 0
	local b = 0
	local opacity = 0.9
	if disabled then
		opacity = 0.5
	end
	if hi then
		a = 0.7
		b = 0.3
	elseif dark then
		a = 0.16
		b = 0
	elseif line then
		a = 0.9
		b = 0.8
	else
		a = 0.32
		b = 0.12
		opacity = 1.0
	end
	if alpha ~= nil then
		opacity = opacity*alpha
	end
	if color == "white" then
		return {a, a, a, opacity}
	elseif color == "grey" then
		return {(a + b)*0.5, (a + b)*0.5, (a + b)*0.5, opacity}
	elseif color == "black" then
		return {b, b, b, opacity}
	elseif color == "violet" then
		return {(a + b)*0.5, b, a, opacity}
	elseif color == "magenta" then
		return {a, b, a, opacity}
	elseif color == "pink" then
		return {a, b, (a + b)*0.5, opacity}
	elseif color == "red" then
		return {a, b, b, opacity}
	elseif color == "orange" then
		return {a, (a + b)*0.5, b, opacity}
	elseif color == "yellow" then
		return {a, a, b, opacity}
	elseif color == "yellowgreen" then
		return {(a + b)*0.5, a, b, opacity}
	elseif color == "green" then
		return {b, a, b, opacity}
	elseif color == "cyan" then
		return {b, a, a, opacity}
	else
		return {b, b, a, opacity}
	end
end

function setColor(color, hi, dark, line, disabled, alpha)
	local rgba = getRGBA(color, hi, dark, line, disabled, alpha)
	graphics.set_color(rgba[1], rgba[2], rgba[3], rgba[4])
end


function createPanel(plId, name, width, refreshAction, color, shadow)
	if not plId then
		return false
	end
	local panelId = false
	
	for pnId = 1, #panelName[plId], 1 do
		if panelState[plId][pnId] == -1 and panelAlpha[plId][pnId] == 0 then
			panelId = pnId
			break
		end
	end
	if not panelId then
		panelId = #panelName[plId] + 1
	end

	if name then
		panelName[plId][panelId] = name
	else
		panelName[plId][panelId] = ""
	end
	if width then
		panelW[plId][panelId] = width + panelPadding*2
	else
		panelW[plId][panelId] = 394
	end
	if refreshAction and refreshAction ~= "" then
		panelRefreshAction[plId][panelId] = loadstring(refreshAction)
	else
		panelRefreshAction[plId][panelId] = false
	end
	if color then
		panelColor[plId][panelId] = color
	else
		panelColor[plId][panelId] = "blue"
	end
	panelShadow[plId][panelId] = (shadow == true)
	panelState[plId][panelId]  = 0
	panelAlpha[plId][panelId]  = 0.0
	panelX[plId][panelId]      = false
	panelY[plId][panelId]      = false
	curX[plId][panelId]        = 0
	curY[plId][panelId]        = 0
	indentX[plId][panelId]     = 0
	sldrLegend[plId][panelId]  = false
	pnCtrlCount[plId][panelId] = 0
	flashRectTimer[plId][panelId] = false
	flashRectX1[plId][panelId] = false
	flashRectY1[plId][panelId] = false
	flashRectX2[plId][panelId] = false
	flashRectY2[plId][panelId] = false

	clearPanel(plId, panelId)
	return panelId
end

function deletePanel(plId, panelId)
	panelState[plId][panelId] = -1
	panelRefreshAction[plId][panelId] = false
end

function clearPanel(plId, panelId)
	panelPosX[plId][panelId] = 0
	panelPosY[plId][panelId] = 0
	curX[plId][panelId]      = 0
	indentX[plId][panelId]   = 0
	panelH[plId][panelId] = panelPadding*2 + panelLineHeight*4.5
	if plId == alertsPlId then
		panelH[plId][panelId] = panelPadding*2 + panelLineHeight*7
		curY[plId][panelId]   = -panelPadding - panelLineHeight*5.5
	else
		panelH[plId][panelId] = panelPadding*2 + panelLineHeight*4.5
		curY[plId][panelId]   = -panelPadding - panelLineHeight*3
	end
	labelW[plId][panelId] = panelSize*8
	panelItems[plId][panelId]  = {}
	pnCtrlCount[plId][panelId] = 0
	sldrLegend[plId][panelId]  = false
	return true
end

function setPanelName(plId, panelId, name)
	panelName[plId][panelId] = name
end

function setPanelAlpha(plId, alpha)
	panelBgAlpha[plId] = alpha
end

function setPanelColor(plId, panelId, color)
	if color then
		panelColor[plId][panelId] = color
	else
		panelColor[plId][panelId] = "blue"
	end
end

function setPanelPos(plId, panelId, x, y)
	if x then panelX[plId][panelId] = x else panelX[plId][panelId] = false end
	if y then panelY[plId][panelId] = y else panelY[plId][panelId] = false end
end

function setPanelWidth(plId, panelId, width)
	panelW[plId][panelId] = width + panelPadding*2
end

function getPanelWidth(plId, panelId)
	return panelW[plId][panelId] - panelPadding*2
end

function getButtonWidth(size, text)
	if size == "xxs" then
		return panelSize*1.25
	elseif size == "xs" then
		return panelSize*3
	elseif size == "s" then
		return panelSize*4.5
	elseif size == "w" then
		return panelSize*8.5
	elseif size == "d" then
		return measure_string(text, "Helvetica_10") + panelPadding
	elseif size == "l" then
		return panelSize*12
	end
	return panelSize*6
end

function openPanel(plId, panelId)
	panelState[plId][panelId] = 1
	anyPanelDraw = true
end

function closePanel(plId, panelId)
	panelState[plId][panelId] = 0
end

function togglePanel(plId, panelId)
	if panelState[plId][panelId] == 1 then
		closePanel(plId, panelId)
	else
		openPanel(plId, panelId)
	end
end

function isPanelOpen(plId, panelId)
	return (panelState[plId][panelId] == 1)
end

local function checkBreak(plId, panelId, w, h)
	if curX[plId][panelId] + w > panelW[plId][panelId] - panelPadding*2 then
		curX[plId][panelId] = indentX[plId][panelId]
		curY[plId][panelId] = curY[plId][panelId] - h - panelPadding*2
	end
end

local function addPanelElement(plId, panelId, array, color)
	local id = #panelItems[plId][panelId] + 1
	panelItems[plId][panelId][id] = array
	if color then
		panelItems[plId][panelId][id]["col"] = color
	end
	return id
end

function setPanelIndent(plId, panelId, indent)
	if indent == nil then
		indentX[plId][panelId] = curX[plId][panelId]
	else
		indentX[plId][panelId] = indent
	end
end

function addPanelSpace(plId, panelId, width)
	if width then
		curX[plId][panelId] = curX[plId][panelId] + width
	else
		curX[plId][panelId] = curX[plId][panelId] + panelPadding
	end
	return
end

function addPanelSpaceRight(plId, panelId, width)
	if curX[plId][panelId] + width > panelW[plId][panelId] - panelPadding*2 then
		addPanelBR(plId, panelId, 1)
	end
	curX[plId][panelId] = panelW[plId][panelId] - panelPadding*2 - width
	return
end

function addPanelBR(plId, panelId, lines)
	curX[plId][panelId] = indentX[plId][panelId]
	curY[plId][panelId] = curY[plId][panelId] - panelLineHeight*lines
	return
end

function addPanelHR(plId, panelId)
	if curX[plId][panelId] > 0 then
		curY[plId][panelId] = curY[plId][panelId] - panelLineHeight
	end
	indentX[plId][panelId] = 0
	curX[plId][panelId] = 0
	curY[plId][panelId] = curY[plId][panelId] - panelLineHeight*1.5
	return addPanelElement(plId, panelId, {["type"]="hr", ["y"]=curY[plId][panelId] + panelLineHeight*1.5})
end

function addPanelTextLine(plId, panelId, text)
	local x = curX[plId][panelId]
	local y = curY[plId][panelId]
	curX[plId][panelId] = indentX[plId][panelId]
	curY[plId][panelId] = curY[plId][panelId] - panelLineHeight
	return addPanelElement(plId, panelId, {["type"]="tx", ["text"]=text, ["x"]=x, ["y"]=y})
end

function addPanelText(plId, panelId, text)
	local x = 0
	local spaceW = measure_string(" ", "Helvetica_12")
	local lines = split(text, "|")
	local words = ""
	local out   = ""
	for j, line in ipairs(lines) do
		words = split(line, {" ","/","\\",".","-"}, nil, true)
		out = ""
		x = curX[plId][panelId]
		for i, word in ipairs(words) do
			x = x + measure_string(word, "Helvetica_12")
			if x > panelW[plId][panelId] - panelPadding*2 then
				if out ~= "" then
					curY[plId][panelId] = curY[plId][panelId] - panelLineHeight
					addPanelElement(plId, panelId, {["type"]="tx", ["text"]=out, ["x"]=curX[plId][panelId], ["y"]=curY[plId][panelId] + panelLineHeight})
					x = measure_string(word, "Helvetica_12")
				end
				out = word
			else
				out = out .. word
			end
		end
		addPanelElement(plId, panelId, {["type"]="tx", ["text"]=out, ["x"]=curX[plId][panelId], ["y"]=curY[plId][panelId]})
		curX[plId][panelId] = indentX[plId][panelId]
		curY[plId][panelId] = curY[plId][panelId] - panelLineHeight
	end
	return true
end

function addPanelCaption(plId, panelId, text)
	local txw = measure_string(text, "Helvetica_12")
	curX[plId][panelId] = indentX[plId][panelId]
	curY[plId][panelId] = curY[plId][panelId] - panelLineHeight*1.75
	return addPanelElement(plId, panelId, {["type"]="cp", ["text"]=string.upper(text), ["x"]=panelW[plId][panelId]*0.5 - panelPadding - txw*0.5, ["y"]=curY[plId][panelId] + panelLineHeight*1.75})
end

function addPanelLabel(plId, panelId, text)
	checkBreak(plId, panelId, labelW[plId][panelId], panelLineHeight)
	curX[plId][panelId] = curX[plId][panelId] + labelW[plId][panelId] + panelPadding
	local txw = measure_string(text, "Helvetica_12")
	return addPanelElement(plId, panelId, {["type"]="lb", ["text"]=text, ["x"]=curX[plId][panelId] - panelPadding - txw, ["y"]=curY[plId][panelId] - 1})
end

function addPanelControl(plId, panelId, char, action, color)
	if not action then action = "" end
	local w = panelLineHeight + panelPadding
	local x = panelW[plId][panelId] - w*(pnCtrlCount[plId][panelId] + 1) - panelPadding
	local y = -panelPadding*1.5 - panelLineHeight - 2
	local textX = w*0.5 - measure_string(char, "Helvetica_10")*0.5 - 1
	pnCtrlCount[plId][panelId] = pnCtrlCount[plId][panelId] + 1
	return addPanelElement(plId, panelId, {["type"]="ctrl", ["click"]=true, ["text"]=char, ["action"]=loadstring(action), ["x"]=x, ["y"]=y, ["w"]=w, ["textX"]=textX}, color)
end

function addPanelClose(plId, panelId, action)
	if not action or action == "" then action = "jjjLib1.closePanel(" .. plId .. ", " .. panelId .. ")" end
	return addPanelControl(plId, panelId, "X", action)
end

local function cutTextToWidth(text, width, addPoints, font)
	if not font then
		font = "Helvetica_10"
	end
	local suffix  = ""
	local suffixW = 0
	if addPoints then
		suffix = "..."
		suffixW = measure_string(suffix, font)
		if suffixW > width then
			suffix  = ""
			suffixW = 0
		end
	end
	local textW = measure_string(text, font)
	if textW <= width then
		return text
	end
	while textW > width - suffixW do
		text = string.sub(text, 1, string.len(text) - 1)
		textW = measure_string(text, font)
	end
	return text .. suffix
end

local function addPanelButtonElement(plId, panelId, type, text, size, active, param, value, enabled, action, color, activeCallback, enabledCallback)
	if not action then action = "" end
	if not activeCallback then activeCallback = "" end
	if not enabledCallback then enabledCallback = "" end
	local w = getButtonWidth(size, text)
	local h = 0
	local textW = 0
	local padd = 0
	if size == "s" then
		h = panelLineHeight*1.25
		padd = 0
	elseif size == "xxs" then
		h = panelLineHeight*1.25
		padd = 0
	elseif size == "xs" then
		h = panelLineHeight*1.25
		padd = 0
	elseif size == "w" then
		h = panelLineHeight*1.5
		padd = panelPadding + 4
	elseif size == "l" then
		h = panelLineHeight*1.5
		padd = panelPadding + 6
	else
		h = panelLineHeight*1.5
		padd = panelPadding + 6
	end
	checkBreak(plId, panelId, w, h)
	curX[plId][panelId] = curX[plId][panelId] + w + padd
	if text ~= "" or type ~= "bt" then
		local textX = 0
		local font = "Helvetica_10"
		if size == "w" or size == "l" then
			font = "Helvetica_12"
		end
		text = cutTextToWidth(text, w - 4, true, font)
		textX = w*0.5 - measure_string(text, font)*0.5
		local arr = {["type"]=type, ["size"]=size, ["click"]=enabled, ["text"]=text, ["enbl"]=enabled, ["x"]=curX[plId][panelId] - w - padd, ["y"]=curY[plId][panelId], ["w"]=w, ["h"]=h, ["textX"]=textX}
		if type == "bt" then
			arr["act"] = active
			arr["action"] = loadstring(action)
			if activeCallback ~= "" then
				arr["actCb"] = assert(loadstring(activeCallback))
			end
			if enabledCallback ~= "" then
				arr["enblCb"] = assert(loadstring(enabledCallback))
			end
			return addPanelElement(plId, panelId, arr, color)
		elseif type == "parbt" then
			arr["type"] = "bt"
			arr["param"] = param
			arr["val"] = value
			arr["act"] = (getParam(plId, param)==value)
			if enabledCallback ~= "" then
				arr["enblCb"] = loadstring(enabledCallback)
			end
			return addPanelElement(plId, panelId, arr, color)
		elseif type == "tgbt" then
			arr["param"] = param
			arr["act"] = getParam(plId, param)
			if enabledCallback ~= "" then
				arr["enblCb"] = loadstring(enabledCallback)
			end
			return addPanelElement(plId, panelId, arr, color)
		end
	end
end

function addPanelButton(plId, panelId, text, size, active, enabled, action, color, activeCallback, enabledCallback)
	return addPanelButtonElement(plId, panelId, "bt", text, size, active, nil, nil, enabled, action, color, activeCallback, enabledCallback)
end

function addPanelParamButton(plId, panelId, text, size, param, value, enabled, color)
	return addPanelButtonElement(plId, panelId, "parbt", text, size, nil, param, value, enabled, nil, color)
end

function addPanelToggleButton(plId, panelId, text, size, param, enabled, color)
	if not text or text == "" then
		if getParam(plId, param) then
			text = "ON"
		else
			text = "OFF"
		end
	end
	return addPanelButtonElement(plId, panelId, "tgbt", text, size, nil, param, nil, enabled, nil, color)
end

local function addPanelSliderElement(plId, panelId, type, text, param, enabled, action, color)
	if not action then action = "" end
	if type == "num" then
		local w = panelSize*4
		local h = panelLineHeight*1.25
		local padd = 4
		curX[plId][panelId] = curX[plId][panelId] + w + padd
		return addPanelElement(plId, panelId, {["type"]=type, ["click"]=enabled, ["mWheel"]=enabled, ["enbl"]=enabled, ["action"]=loadstring(action), ["param"]=param, ["x"]=curX[plId][panelId] - w - padd, ["y"]=curY[plId][panelId], ["w"]=w, ["h"]=h}, color)
	elseif type == "sldr" then
		local w = panelW[plId][panelId] - panelPadding*2 - curX[plId][panelId]
		local h = panelLineHeight*1.25
		local lgndl  = nil
		local lgndr  = nil
		local lgndrw = nil
		if sldrLegend[plId][panelId] then
			lgndl  = sldrLegend[plId][panelId]["textl"]
			lgndr  = sldrLegend[plId][panelId]["textr"]
			lgndrw = measure_string(lgndr, "Helvetica_10")
			sldrLegend[plId][panelId] = false
		end
		local x = curX[plId][panelId]
		local y = curY[plId][panelId]
		curX[plId][panelId] = indentX[plId][panelId]
		curY[plId][panelId] = curY[plId][panelId] - panelLineHeight*2
		return addPanelElement(plId, panelId, {["type"]="numSldr", ["click"]=enabled, ["mWheel"]=enabled, ["enbl"]=enabled, ["action"]=loadstring(action), ["param"]=param, ["x"]=x, ["y"]=y, ["w"]=w, ["h"]=h, ["lgndl"]=lgndl, ["lgndr"]=lgndr, ["lgndrw"]=lgndrw}, color)
	else
		local w = panelLineHeight - 2
		local h = panelLineHeight - 2
		local padd = 0
		if type == "num--" then
			padd = 1
		elseif type == "num-" then
			padd = 4
		elseif type == "num+" then
			padd = 1
		elseif type == "num++" then
			padd = panelPadding*2
		end
		local textX = w*0.5 - measure_string(text, "Helvetica_10")*0.5
		curX[plId][panelId] = curX[plId][panelId] + w + padd
		return addPanelElement(plId, panelId, {["type"]=type, ["click"]=enabled, ["param"]=param, ["text"]=text, ["enbl"]=enabled, ["action"]=loadstring(action), ["x"]=curX[plId][panelId] - w - padd, ["y"]=curY[plId][panelId], ["w"]=w, ["h"]=h, ["textX"]=textX}, color)
	end
end

function addPanelNumericInput(plId, panelId, param, enabled, action, color)
	addPanelSliderElement(plId, panelId, "num--", "<<", param, enabled, action, color)
	addPanelSliderElement(plId, panelId, "num-",  "<",  param, enabled, action, color)
	addPanelSliderElement(plId, panelId, "num",   "",   param, enabled, action, color)
	addPanelSliderElement(plId, panelId, "num+",  ">",  param, enabled, action, color)
	addPanelSliderElement(plId, panelId, "num++", ">>", param, enabled, action, color)
	return addPanelSliderElement(plId, panelId, "sldr", nil, param, enabled, action, color)
end

function addPanelSliderLegend(plId, panelId, textL, textR)
	sldrLegend[plId][panelId] = {["textl"]=textL, ["textr"]=textR}
	return
end

function addPanelDataRefDisplay(plId, panelId, dataRefPointer, dataRefType, size, format)
	local w = getButtonWidth(size, nil) + panelPadding
	checkBreak(plId, panelId, w, panelLineHeight)
	curX[plId][panelId] = curX[plId][panelId] + w
	return addPanelElement(plId, panelId, {["type"]="DrD", ["dr"]=dataRefPointer, ["drtype"]=dataRefType, ["fmt"]=format, ["x"]=curX[plId][panelId] - w, ["y"]=curY[plId][panelId]})
end

function addPanelFpsDisplay(plId, panelId)
	return addPanelElement(plId, panelId, {["type"]="fpsD", ["id"]="fpsD", ["click"]=true, ["x"]=curX[plId][panelId], ["y"]=curY[plId][panelId]})
end

function alert(plId, headline, text, color, button1Text, button1Action, button2Text, button2Action, button3Text, button3Action)
	if #panelName[alertsPlId] > 15 then
		table.remove(panelName[alertsPlId])
	end
	local panelId = createPanel(alertsPlId, pluginName[plId] .. ".lua", 394, "", color)
	addPanelCaption(alertsPlId, panelId, headline)
	addPanelText(alertsPlId, panelId, text)
	addPanelBR(alertsPlId, panelId, 1.5)
	
	local close = "jjjLib1.deletePanel(" .. alertsPlId .. "," .. panelId .. "); "
	if not button1Action then button1Action = "" end
	if not button2Action then button2Action = "" end
	if not button3Action then button3Action = "" end
	
	if button1Text and button2Text and button3Text then
		addPanelButton(alertsPlId, panelId, button1Text, "w", false, true, close .. button1Action, "red")
		addPanelButton(alertsPlId, panelId, button2Text, "w", false, true, close .. button2Action, "red")
		addPanelButton(alertsPlId, panelId, button3Text, "w", false, true, close .. button3Action, "red")
	elseif button1Text and button2Text then
		addPanelButton(alertsPlId, panelId, button1Text, "w", false, true, close .. button1Action, "red")
		addPanelButton(alertsPlId, panelId, "", "w", false, false)
		addPanelButton(alertsPlId, panelId, button2Text, "w", false, true, close .. button2Action, "red")
	elseif button1Text then
		addPanelButton(alertsPlId, panelId, "", "w", false, false)
		addPanelButton(alertsPlId, panelId, button1Text, "w", false, true, close .. button1Action, "red")
		addPanelButton(alertsPlId, panelId, "", "w", false, false)
	else
		addPanelButton(alertsPlId, panelId, "", "w", false, false)
		addPanelButton(alertsPlId, panelId, "OK", "w", false, true, close .. button1Action, "red")
		addPanelButton(alertsPlId, panelId, "", "w", false, false)
	end
	addPanelBR(alertsPlId, panelId, 1)
	openPanel(alertsPlId, panelId)
	return true
end

function checkPanelClick(plId, panelId)
	if panelState[plId][panelId] ~= 1 or not panelPosX[plId][panelId] or not panelW[plId][panelId] or not panelPosY[plId][panelId] or not panelH[plId][panelId] or #panelItems[plId][panelId] < 1 then
		return false
	end

	if MOUSE_X >= panelPosX[plId][panelId] and MOUSE_X <= panelPosX[plId][panelId] + panelW[plId][panelId] and MOUSE_Y <= panelPosY[plId][panelId] and MOUSE_Y >= panelPosY[plId][panelId] - panelH[plId][panelId] then
		local i
		local element
		local id
		local param
		local val
		for i, element in ipairs(panelItems[plId][panelId]) do
			if element["click"] then
				if element["x1"] and MOUSE_X >= element["x1"] and element["x2"] and MOUSE_X <= element["x2"] and element["y1"] and MOUSE_Y >= element["y1"] and element["y2"] and MOUSE_Y <= element["y2"] then
					flashRectTimer[plId][panelId] = jjjLib1_clock
					flashRectX1[plId][panelId] = math.floor(element["x1"] - panelPosX[plId][panelId])
					flashRectY1[plId][panelId] = math.floor(element["y1"] - panelPosY[plId][panelId])
					flashRectX2[plId][panelId] = math.floor(element["x2"] - panelPosX[plId][panelId])
					flashRectY2[plId][panelId] = math.floor(element["y2"] - panelPosY[plId][panelId])
					if element["param"] then
						param = element["param"]
						if element["type"] == "num" then
							setParam(plId, param, getParamDefault(plId, param))
						end
						if element["type"] == "num-" then
							setParam(plId, param, math.min(getParamMax(plId, param), math.max(getParamMin(plId, param), getParam(plId, param) - getParamRealStep(plId, param))))
						end
						if element["type"] == "num--" then
							setParam(plId, param, math.min(getParamMax(plId, param), math.max(getParamMin(plId, param), getParam(plId, param) - getParamRealStep(plId, param)*10)))
						end
						if element["type"] == "num+" then
							setParam(plId, param, math.max(getParamMin(plId, param), math.min(getParamMax(plId, param), getParam(plId, param) + getParamRealStep(plId, param))))
						end
						if element["type"] == "num++" then
							setParam(plId, param, math.max(getParamMin(plId, param), math.min(getParamMax(plId, param), getParam(plId, param) + getParamRealStep(plId, param)*10)))
						end
						if element["type"] == "numSldr" then
							val = getParamSliderMin(plId, param) + (MOUSE_X - element["x1"])/(element["x2"]-element["x1"])*(getParamSliderMax(plId, param) - getParamSliderMin(plId, param))
							setParam(plId, param, math.max(getParamMin(plId, param), math.min(getParamMax(plId, param), math.floor(val/getParamStep(plId, param) + 0.5)*getParamStep(plId, param))))
						end
						if element["type"] == "bt" then
							setParam(plId, param, element["val"])
						end
						if element["type"] == "tgbt" then
							setParam(plId, param, not getParam(plId, param))
						end
						if panelRefreshAction[plId][panelId] then
							panelRefreshAction[plId][panelId]()
						end
						if params[plId][param]["autosave"] then
							saveParamsScope(plId, params[plId][param]["save"])
						end
					end
					action = element["action"]
					if action then
						action()
						if panelRefreshAction[plId][panelId] and element["type"] ~= "ctrl" then
							panelRefreshAction[plId][panelId]()
						end
					end
				end
			end
		end
		return true
	end
	return false
end

function onMouseClick()
	if MOUSE_STATUS == "down" and anyPanelDraw then
		for plId = 1, #pluginName, 1 do
			for pnId = #panelName[plId], 1, -1 do
				if checkPanelClick(plId, pnId) then
					return true
				end
			end
		end
	end
end

function checkPanelMouseWheel(plId, panelId)
	if panelState[plId][panelId] ~= 1 or not panelPosX[plId][panelId] or not panelW[plId][panelId] or not panelPosY[plId][panelId] or not panelH[plId][panelId] or #panelItems[plId][panelId] < 1 then
		return false
	end

	if MOUSE_X >= panelPosX[plId][panelId] and MOUSE_X <= panelPosX[plId][panelId] + panelW[plId][panelId] and MOUSE_Y <= panelPosY[plId][panelId] and MOUSE_Y >= panelPosY[plId][panelId] - panelH[plId][panelId] then
		local i
		local element
		local param
		local val
		for i, element in ipairs(panelItems[plId][panelId]) do
			if element["mWheel"] and element["param"] then
				if element["x1"] and MOUSE_X >= element["x1"] and element["x2"] and MOUSE_X <= element["x2"] and element["y1"] and MOUSE_Y >= element["y1"] and element["y2"] and MOUSE_Y <= element["y2"] then
					if element["type"] == "num" or element["type"] == "numSldr" then
						param = element["param"]
						val = getParam(plId, param)
						val = val + MOUSE_WHEEL_CLICKS*getParamRealStep(plId, param)
						setParam(plId, param, math.max(getParamMin(plId, param), math.min(getParamMax(plId, param), val)))
						if panelRefreshAction[plId][panelId] then
							panelRefreshAction[plId][panelId]()
						end
					end
				end
			end
		end
		return true
	end
	return false
end

function onMouseWheel()
	if anyPanelDraw then
		for plId = 1, #pluginName, 1 do
			for pnId = #panelName[plId], 1, -1 do
				if checkPanelMouseWheel(plId, pnId) then
					return true
				end
			end
		end
	end
end

local function panelRect(x1, y1, x2, y2, active, enabled, color, alpha)
	local opacity = 0.5
	if enabled then opacity = 0.9 end
	setColor(color, false, false, true, not enabled, opacity*alpha*alpha)
	graphics.draw_rectangle(math.floor(x1 - 1), math.floor(y1 - 1), math.floor(x2 + 1), math.floor(y2 + 1))
	setColor(color, active, true, false, not enabled, opacity*alpha)
	graphics.draw_rectangle(math.floor(x1), math.floor(y1), math.floor(x2), math.floor(y2))
end

local function drawPanelElement(plId, panelId, e)
	local tp = e["type"]
	local alpha = panelAlpha[plId][panelId]
	local pX = panelPosX[plId][panelId] + panelPadding
	local pY = panelPosY[plId][panelId] + panelPadding
	local tx = ""
	local min = 0
	local max = 0
	local gdMin = 0
	local gdMax = 0
	local default = 0
	local value = 0
	local colR = 1
	local colG = 1
	local colB = 1
	local col = panelColor[plId][panelId]
	if e["col"] then
		col = e["col"]
	end
	
	if tp == "hr" then
		graphics.set_color(colR, colG, colB, 0.2*alpha)
		graphics.draw_line(math.floor(pX - panelPadding), math.floor(pY + e["y"])+0.5, math.floor(pX + panelW[plId][panelId] - panelPadding), math.floor(pY + e["y"])+0.5, 1)
	elseif tp == "tx" then
		graphics.set_color(colR, colG, colB, 1*alpha)
		draw_string_Helvetica_12(pX + e["x"], pY + e["y"], e["text"])
	elseif tp == "cp" then
		graphics.set_color(colR, colG, colB, 1*alpha)
		draw_string_Helvetica_12(pX + e["x"], pY + e["y"], e["text"])
	elseif tp == "lb" then
		graphics.set_color(colR, colG, colB, 0.7*alpha)
		draw_string_Helvetica_12(pX + e["x"], pY + e["y"], e["text"])
	elseif tp == "bt" or tp == "tgbt" then
		e["x1"] = pX + e["x"]
		e["y1"] = pY + e["y"] - e["h"]*0.33 + 1
		e["x2"] = pX + e["x"] + e["w"]
		e["y2"] = pY + e["y"] + e["h"]*0.67 + 1
		if e["actCb"] then
			e["act"] = e["actCb"]()
		end
		if e["enblCb"] then
			e["enbl"] = e["enblCb"]()
		end
		if e["enbl"] then
			panelRect(e["x1"], e["y1"], e["x2"], e["y2"], e["act"], e["enbl"], col, alpha)
			graphics.set_color(colR, colG, colB, 1*alpha)
		else
			panelRect(e["x1"], e["y1"], e["x2"], e["y2"], e["act"], e["enbl"], panelColor[plId][panelId], alpha)
			graphics.set_color(colR, colG, colB, 0.5*alpha)
		end
		if e["size"] == "w" or e["size"] == "l"  then
			draw_string_Helvetica_12(pX + e["x"] + e["textX"], pY + e["y"], e["text"])
		else
			draw_string_Helvetica_10(pX + e["x"] + e["textX"], pY + e["y"], e["text"])
		end
	elseif tp == "num" then
		e["x1"] = pX + e["x"]
		e["y1"] = pY + e["y"] - e["h"]*0.33 + 1
		e["x2"] = pX + e["x"] + e["w"]
		e["y2"] = pY + e["y"] + e["h"]*0.67 + 1
		panelRect(e["x1"], e["y1"], e["x2"], e["y2"], false, e["enbl"], col, alpha)
		if e["enbl"] then
			graphics.set_color(colR, colG, colB, 1*alpha)
		else
			graphics.set_color(colR, colG, colB, 0.5*alpha)
		end
		tx = string.format(getParamFormat(plId, e["param"]), getParam(plId, e["param"]))
		draw_string_Helvetica_12(pX + e["x"] + e["w"]*0.5 - measure_string(tx, "Helvetica_12")*0.5, pY + e["y"] - 1, tx)
	elseif tp == "num--" or tp == "num-" or tp == "num+" or tp == "num++" then
		e["x1"] = pX + e["x"]
		e["y1"] = pY + e["y"] - e["h"]*0.33 + 1
		e["x2"] = pX + e["x"] + e["w"]
		e["y2"] = pY + e["y"] + e["h"]*0.67 + 1
		panelRect(e["x1"], e["y1"], e["x2"], e["y2"], false, e["enbl"], col, alpha)
		if e["enbl"] then
			graphics.set_color(colR, colG, colB, 1*alpha)
		else
			graphics.set_color(colR, colG, colB, 0.5*alpha)
		end
		draw_string_Helvetica_10(pX + e["x"] + e["textX"], pY + e["y"] - 1, e["text"])
	elseif tp == "numSldr" then
		e["x1"] = math.floor(pX + e["x"])
		e["y1"] = math.floor(pY + e["y"] - e["h"]*0.33 + 1)
		e["x2"] = math.floor(pX + e["x"] + e["w"])
		e["y2"] = math.floor(pY + e["y"] + e["h"]*0.67 + 1)
		min = getParamSliderMin(plId, e["param"])
		max = getParamSliderMax(plId, e["param"])
		gdMin = getParamGoodMin(plId, e["param"])
		gdMax = getParamGoodMax(plId, e["param"])
		if e["enbl"] then
			graphics.set_color(0, 0.8, 0.0, 0.5*alpha)
		else
			graphics.set_color(0, 0.8, 0.0, 0.25*alpha)
		end
		if gdMin and gdMax then
			graphics.draw_rectangle(e["x1"] + (gdMin - min)/(max - min)*e["w"], e["y1"] + 2, e["x1"] + (gdMax - min)/(max - min)*e["w"], e["y2"] - 3)
		end
		graphics.set_color(colR, colG, colB, 0.4*alpha)
		graphics.draw_rectangle(e["x1"], e["y1"] + e["h"]*0.4, e["x2"], e["y2"] - e["h"]*0.5)
		graphics.draw_rectangle(e["x1"]-1, e["y1"] + 2, e["x1"], e["y2"] - 3)
		graphics.draw_rectangle(e["x2"], e["y1"] + 2, e["x2"]+1, e["y2"] - 3)
		if e["enbl"] then
			graphics.set_color(colR, colG, colB, 1*alpha)
			graphics.draw_rectangle(e["x1"] + (getParamMin(plId, e["param"]) - min)/(max - min)*e["w"], e["y1"] + e["h"]*0.4, e["x1"] + (getParamMax(plId, e["param"]) - min)/(max - min)*e["w"], e["y2"] - e["h"]*0.5)
		end
		if getParamDefault(plId, e["param"]) then
			default = e["x1"] + (getParamDefault(plId, e["param"]) - min)/(max - min)*e["w"]
			graphics.draw_line(default, e["y1"] + 1, default, e["y2"] - e["h"]*0.5)
		end
		value  = e["x1"] + (getParam(plId, e["param"]) - min)/(max - min)*e["w"]
		graphics.draw_triangle(value, e["y2"] - e["h"]*0.55, value - 6, e["y2"], value + 6, e["y2"])
		if e["lgndl"] and e["lgndr"] then
			graphics.set_color(0.73, 0.73, 0.86, 1*alpha)
			draw_string_Helvetica_10(e["x1"] - 3, e["y2"] + 4, e["lgndl"])
			draw_string_Helvetica_10(e["x2"] + 3 - e["lgndrw"], e["y2"] + 4, e["lgndr"])
		end
		e["x1"] = e["x1"] - 4
		e["x2"] = e["x2"] + 4
		e["y1"] = e["y1"] - 2
		e["y2"] = e["y2"] + 2
	elseif tp == "ctrl" then
		e["x1"] = pX + e["x"]
		e["y1"] = pY + e["y"]
		e["x2"] = pX + e["x"] + e["w"] - 1
		e["y2"] = pY + e["y"] + panelPadding*0.5 + panelLineHeight + 1
		panelRect(e["x1"], e["y1"], e["x2"], e["y2"], false, true, col, alpha)
		graphics.set_color(colR, colG, colB, 0.5*alpha)
		draw_string_Helvetica_10(pX + e["x"] + e["textX"], pY + e["y"] + panelLineHeight*0.4 - 1, e["text"])
		draw_string_Helvetica_10(pX + e["x"] + e["textX"] + 1, pY + e["y"] + panelLineHeight*0.4 - 1, e["text"])
	elseif tp == "DrD" then
		graphics.set_color(colR, colG, colB, 1*alpha)
		if e["drtype"] == "f" then
			tx = string.format(e["fmt"], getDataRef_f(e["dr"]))
		elseif e["drtype"] == "i" then
			tx = string.format(e["fmt"], getDataRef_i(e["dr"]))
		else
			tx = ""
		end
		draw_string_Helvetica_12(pX + e["x"], pY + e["y"], tx)
	elseif tp == "fpsD" then
		e["x1"] = pX + e["x"]
		e["y1"] = pY + e["y"] - panelLineHeight*0.33
		e["x2"] = pX + e["x"] + panelSize*8.5
		e["y2"] = pY + e["y"] + panelLineHeight*0.67 + 2
	end
end

local function drawPanel(plId, panelId)
	local isAlert = (plId == alertsPlId)
	local headH   = panelLineHeight*1.5
	local headCol = panelColor[plId][panelId]

	panelH[plId][panelId] = -curY[plId][panelId] + panelPadding + 1
	if isAlert then
		headH = panelLineHeight*4
		headCol = "red"
	end

	if not panelX[plId][panelId] then
		panelPosX[plId][panelId] = math.floor(SCREEN_WIDTH*0.5 - panelW[plId][panelId]*0.5)
	elseif panelX[plId][panelId] >= 0 then
		panelPosX[plId][panelId] = math.floor(panelX[plId][panelId])
	else
		panelPosX[plId][panelId] = math.floor(SCREEN_WIDTH + panelX[plId][panelId] - panelW[plId][panelId])
	end
	if not panelY[plId][panelId] then
		panelPosY[plId][panelId] = math.floor(SCREEN_HIGHT*0.5 + panelH[plId][panelId]*0.5)
	elseif panelY[plId][panelId] >= 0 then
		panelPosY[plId][panelId] = math.floor(panelY[plId][panelId] + panelH[plId][panelId])
	else
		panelPosY[plId][panelId] = math.floor(SCREEN_HIGHT + panelY[plId][panelId])
	end
	
	if isAlert then
		panelPosX[plId][panelId] = panelPosX[plId][panelId] - alertPanelCount*12
		panelPosY[plId][panelId] = panelPosY[plId][panelId] + alertPanelCount*12
		graphics.set_color(0, 0, 0, 0.7*panelAlpha[plId][panelId])
		graphics.draw_rectangle(panelPosX[plId][panelId] + 2, panelPosY[plId][panelId] - 2, panelPosX[plId][panelId] + panelW[plId][panelId] + 4, panelPosY[plId][panelId] - panelH[plId][panelId] - 5)
		alertPanelCount = alertPanelCount + 1
	elseif panelShadow[plId][panelId] then
		graphics.set_color(0, 0, 0, 0.7*panelAlpha[plId][panelId])
		graphics.draw_rectangle(panelPosX[plId][panelId] + 2, panelPosY[plId][panelId] - 2, panelPosX[plId][panelId] + panelW[plId][panelId] + 4, panelPosY[plId][panelId] - panelH[plId][panelId] - 5)
	end
	
	local curY = panelPosY[plId][panelId] - headH*0.5 - panelLineHeight*0.3333 + 1

	setColor(headCol, false, true, false, false, panelBgAlpha[plId]*panelAlpha[plId][panelId])
	graphics.draw_rectangle(panelPosX[plId][panelId], panelPosY[plId][panelId], panelPosX[plId][panelId] + panelW[plId][panelId], panelPosY[plId][panelId] - headH)
	setColor(panelColor[plId][panelId], false, false, false, false, panelBgAlpha[plId]*panelAlpha[plId][panelId])
	graphics.draw_rectangle(panelPosX[plId][panelId], panelPosY[plId][panelId] - headH, panelPosX[plId][panelId] + panelW[plId][panelId], panelPosY[plId][panelId] - panelH[plId][panelId])

	graphics.set_color(1, 1, 1, 1*panelAlpha[plId][panelId])
	draw_string_Helvetica_12(panelPosX[plId][panelId] + panelW[plId][panelId]*0.5 - measure_string(panelName[plId][panelId], "Helvetica_12")*0.5, curY, panelName[plId][panelId])
	setColor(headCol, true, false, false, false, panelAlpha[plId][panelId])
	draw_string_Helvetica_12(panelPosX[plId][panelId] + panelPadding, panelPosY[plId][panelId] - panelLineHeight - 1, pluginName[plId])
	draw_string_Helvetica_12(panelPosX[plId][panelId] + panelPadding + 1, panelPosY[plId][panelId] - panelLineHeight - 1, pluginName[plId])

	if isAlert then
		local x = panelPosX[plId][panelId] + panelPadding + headH*0.5
		setColor(headCol, true, false, false, false, panelAlpha[plId][panelId])
		graphics.draw_filled_circle( x, curY + 4, headH*0.5 - 1.5 )
		setColor(headCol, false, true, false, false, panelAlpha[plId][panelId])
		graphics.draw_filled_circle( x, curY - headH*0.25, headH*0.125 )
		graphics.draw_rectangle( x - headH*0.125, curY + headH*0.45, x + headH*0.125, curY - headH*0.125 + 1)

		x = panelPosX[plId][panelId] + panelW[plId][panelId] - panelPadding - measure_string("jjjLib1", "Helvetica_12")
		setColor(headCol, true, true, false, false, panelAlpha[plId][panelId])
		draw_string_Helvetica_12(x, panelPosY[plId][panelId] - panelLineHeight - 1, "jjjLib1")
		draw_string_Helvetica_12(x + 1, panelPosY[plId][panelId] - panelLineHeight - 1, "jjjLib1")
	end

	for i, element in ipairs(panelItems[plId][panelId]) do
		drawPanelElement(plId, panelId, element)
	end
	
	if flashRectTimer[plId][panelId] then
		if currentClock - flashRectTimer[plId][panelId] < flashTime then
			if flashRectX1[plId][panelId] and flashRectY1[plId][panelId] and flashRectX2[plId][panelId] and flashRectY2[plId][panelId] then
				graphics.set_color(1, 1, 1, 0.2 - (currentClock - flashRectTimer[plId][panelId])/flashTime*0.2)
				graphics.draw_rectangle(panelPosX[plId][panelId] + flashRectX1[plId][panelId], panelPosY[plId][panelId] + flashRectY1[plId][panelId], panelPosX[plId][panelId] + flashRectX2[plId][panelId], panelPosY[plId][panelId] + flashRectY2[plId][panelId])
				graphics.set_color(1, 1, 1, 1 - (currentClock - flashRectTimer[plId][panelId])/flashTime)
				graphics.draw_line(panelPosX[plId][panelId] + flashRectX1[plId][panelId] - 2, panelPosY[plId][panelId] + flashRectY1[plId][panelId] - 1, panelPosX[plId][panelId] + flashRectX2[plId][panelId] + 2, panelPosY[plId][panelId] + flashRectY1[plId][panelId] - 1)
				graphics.draw_line(panelPosX[plId][panelId] + flashRectX1[plId][panelId] - 2, panelPosY[plId][panelId] + flashRectY2[plId][panelId] + 1, panelPosX[plId][panelId] + flashRectX2[plId][panelId] + 2, panelPosY[plId][panelId] + flashRectY2[plId][panelId] + 1)
				graphics.draw_line(panelPosX[plId][panelId] + flashRectX1[plId][panelId] - 1, panelPosY[plId][panelId] + flashRectY1[plId][panelId], panelPosX[plId][panelId] + flashRectX1[plId][panelId] - 1, panelPosY[plId][panelId] + flashRectY2[plId][panelId])
				graphics.draw_line(panelPosX[plId][panelId] + flashRectX2[plId][panelId] + 1, panelPosY[plId][panelId] + flashRectY1[plId][panelId], panelPosX[plId][panelId] + flashRectX2[plId][panelId] + 1, panelPosY[plId][panelId] + flashRectY2[plId][panelId])
			end
		else
			flashRectTimer[plId][panelId] = false
		end
	end

end

function getPanelElementPos(plId, panelId, elementId)
	return panelItems[plId][panelId][elementId]["x1"], panelItems[plId][panelId][elementId]["y1"], panelItems[plId][panelId][elementId]["x2"], panelItems[plId][panelId][elementId]["y2"]
end

function draw()
	frameSec = jjjLib1_clock - currentClock
	currentClock = jjjLib1_clock
	if anyPanelDraw then
		XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
		anyPanelDrawCheck = false
			alertPanelCount = 0
--		draw_string(20, 120, #pluginName)
		for plId = #pluginName, 1, -1 do
--			draw_string(20, 120 - plId*16, plId .. " = " .. #panelName[plId])
			if warnings[plId] ~= nil then
				alert(plId, 'WARNING', warnings[plId], 'white', 'OK')
				anyPanelDrawCheck = true
				warnings[plId] = nil
			end

			for panelId = 1, #panelName[plId], 1 do
				if panelW[plId][panelId] and #panelItems[plId][panelId] > 0 then
					if panelState[plId][panelId] == 1 and panelAlpha[plId][panelId] < 1 then
						panelAlpha[plId][panelId] = math.min(1, panelAlpha[plId][panelId] + frameSec*panelFadeSpd)
					elseif panelState[plId][panelId] < 1 and panelAlpha[plId][panelId] > 0 then
						panelAlpha[plId][panelId] = math.max(0, panelAlpha[plId][panelId] - frameSec*panelFadeSpd)
					end
					if panelAlpha[plId][panelId] > 0 then
						anyPanelDrawCheck = true
						drawPanel(plId, panelId)
					end
				end
			end
		end
		anyPanelDraw = anyPanelDrawCheck
	end
end

loadXPLMlib()

local parts = split(AIRCRAFT_PATH, "/", false)
aircraft = parts[#parts-1]

alertsPlId = getPlId("")

do_every_draw("jjjLib1.draw()")
do_on_mouse_click("if jjjLib1.onMouseClick() then RESUME_MOUSE_CLICK = true end")
do_on_mouse_wheel("if jjjLib1.onMouseWheel() then RESUME_MOUSE_WHEEL = true end")
