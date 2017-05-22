-- You can create your own DataRefs to share them with other plugins
-- Here are some useless examples (meens this is only for learning the syntax).
-- Use the DataRefEditor to check it, they are automatically reported to the DRE.

-- A string must be created as a "Data" type
my_welcome_string = create_dataref_table("FlyWithLua/useless_samples/welcome", "Data")

-- no matter if it looks like a table, just index the first element to fill it
my_welcome_string[0] = "Hello World!"

-- another example, now we create an integer value
the_answer = create_dataref_table("FlyWithLua/useless_samples/the_answer", "Int")

-- again we will index zero to access a non-array DataRef
the_answer[0] = 42

-- in times of alternative facts, we can have multiple answers
all_answers = create_dataref_table("FlyWithLua/useless_samples/answers", "IntArray")

-- the array is build up dynamically, so just fill it with values and increase the index starting from zero
all_answers[0] = 42
all_answers[1] = 2017
all_answers[2] = 11

-- still not enough? At least an array of float values
roots = create_dataref_table("FlyWithLua/useless_samples/square_roots", "FloatArray")

-- now we fill it up with useless values
local i
for i = 0, 200 do
    roots[i] = math.sqrt(i)
end