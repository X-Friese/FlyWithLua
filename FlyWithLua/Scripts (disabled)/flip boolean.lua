-- There are often situations where you want to flip a boolean variable.
-- Here is an example:
flipvariable = true
print(tostring(flipvariable))

-- Now let's flip the value. If you like it most complicated, try this:
if flipvariable == true then
    flipvariable = false
else
    flipvariable = true
end
print(tostring(flipvariable))

-- But there is a much more elegant way. And it's very simple, if a variable
-- is true, it is not false. And if it is not false, then it must be true.
-- So just use this little pice of code:
flipvariable = not flipvariable
print(tostring(flipvariable))

-- Cool, isn't it? Here is the check if it can flip back.
flipvariable = not flipvariable
print(tostring(flipvariable))

-- You can do the same with an integer variable, that is 0 or 1.
flipvariable = 1
print(tostring(flipvariable))

-- Just do this. Remember that 1 - 1 = 0 and 1 - 0 = 1, so it will flip very well.
flipvariable = 1 - flipvariable
print(tostring(flipvariable))

-- and of corse it will flip back.
flipvariable = 1 - flipvariable
print(tostring(flipvariable))