-- Testing an Arcaze USB Display Driver32

require "arcaze"

-- connect to the first Arcaze device we can open
my_first_arcaze = arcaze.open_first_device()

arcaze.init_display( my_first_arcaze, 0, 5, 8)
arcaze.init_display( my_first_arcaze, 2, 5, 6)

A1, A2, A3, A4, A5, A6, A7, A8 = arcaze.read_pins( my_first_arcaze )

logMsg("A1 = "..tostring(A1)..", A2 = "..tostring(A2)..", A3 = "..tostring(A3)..", A4 = "..tostring(A4)..", A5 = "..tostring(A5)..", A6 = "..tostring(A6)..", A7 = "..tostring(A7)..", A8 = "..tostring(A8))


E1, E2, E3, E4, E5, E6, E7, E8 = arcaze.read_encoders( my_first_arcaze )

logMsg("E1 = "..tostring(E1)..", E2 = "..tostring(E2)..", E3 = "..tostring(E3)..", E4 = "..tostring(E4)..", E5 = "..tostring(E5)..", E6 = "..tostring(E6)..", E7 = "..tostring(E7)..", E8 = "..tostring(E8))

arcaze.show(my_first_arcaze, "1B", 0xff, "  876.543")

do_every_frame([[-- show first encoder on display 3A
E1 = arcaze.read_encoders( my_first_arcaze )
arcaze.show(my_first_arcaze, "3A", 0xff, E1)]])