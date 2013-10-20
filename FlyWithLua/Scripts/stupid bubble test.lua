function Steffi()
	local pos = 0
	pos = bubble(20, pos, "Steffi says:", "If you want to throw these stupid bubbles away,", 'just erase the file "stupid bubble test.lua" and they are gone forever!')
	pos = big_bubble(20, pos, "You might says:", "What the hell are these bubbles for?")
	pos = huge_bubble(20, pos, "Steffi says:", "Go and enjoy your flight!", "It will be easy to do this with me onboard.", "I will give you some tipps.")
end

do_every_draw("Steffi()")