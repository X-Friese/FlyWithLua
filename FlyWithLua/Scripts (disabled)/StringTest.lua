

function StringTest()
	local l = get("sim/aircraft/view/acf_notes")
	local m = get("sim/aircraft/view/acf_author")
	local n = get("sim/aircraft/view/acf_tailnum")
	local o = get("sim/aircraft/view/acf_descrip")
	local p = get("sim/aircraft/view/acf_livery_path")
	local q = get("sim/aircraft/parts/acf_Rafl0")
	print("%%% notes   = ["..l.."]")
	print("%%% author  = ["..m.."]")
	print("%%% tailnum = ["..n.."]")
	print("%%% descrip = ["..o.."]")
	print("%%% livery  = ["..p.."]")
	print("%%% Rafl0   = ["..q.."]")

	dataref("blub","sim/aircraft/view/acf_descrip","writable")
	blub = "descrip t t 1"

	bla = get("sim/aircraft/view/acf_descrip")
	print("%%% bla1 = ["..bla.."]")

	set("sim/aircraft/view/acf_descrip","descrip t 2")

	bla = get("sim/aircraft/view/acf_descrip")
	print("%%% bla2 = ["..bla.."]")
end


create_command("FlyWithLua/StringTest", "String Test", "StringTest()", "", "")
