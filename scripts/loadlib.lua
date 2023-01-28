local libfile = io.open('../lib/liblua.lua', 'r')
if libfile == nil then
    return 1
end
local libcode = libfile:read('a')
if libcode == nil then
    return 1
end
local text = 'extern const char liblua_code [] = {'
local idx = 1
while idx < #libcode do
    text = text .. string.byte(libcode, idx) .. ',\n'
    idx  = idx + 1
end
text = text .. '0,\n};'
local cppfile = io.open('./liblua.cc', 'w+')
if cppfile == nil then
    return 1
end
cppfile:write(text)
