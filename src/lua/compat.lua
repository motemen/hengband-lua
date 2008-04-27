-------------------------------------------------------------------
-- Real globals
-- _ALERT
-- _ERRORMESSAGE
-- _VERSION
-- _G
-- assert
-- error
-- metatable
-- next
-- print
-- require
-- tonumber
-- tostring
-- type
-- unpack

-------------------------------------------------------------------
-- collectgarbage
-- gcinfo

-- globals

-- call   -> protect(f, err)
-- loadfile
-- loadstring

-- rawget
-- rawset

-- getargs = Main.getargs ??

rawtype = type

function do_ (f, err)
  if not f then print(err); return end
  local a,b = pcall(f)
  if not a then print(b); return nil
  else return b or true
  end
end

function dostring(s) return do_(loadstring(s)) end
-- function dofile(s) return do_(loadfile(s)) end

-------------------------------------------------------------------
-- Table library
local tab = table
foreach = tab.foreach
foreachi = tab.foreachi
getn = tab.getn
tinsert = tab.insert
tremove = tab.remove
sort = tab.sort

-------------------------------------------------------------------
-- Debug library
local dbg = debug
getinfo = dbg.getinfo
getlocal = dbg.getlocal
setcallhook = function () error"`setcallhook' is deprecated" end
setlinehook = function () error"`setlinehook' is deprecated" end
setlocal = dbg.setlocal

-------------------------------------------------------------------
-- string library
local str = string
strbyte = str.byte
strchar = str.char
strfind = str.find
format = str.format
gsub = str.gsub
strlen = str.len
strlower = str.lower
strrep = str.rep
strsub = str.sub
strupper = str.upper

-------------------------------------------------------------------
-- os library
clock = os.clock
date = os.date
difftime = os.difftime
execute = os.execute --?
exit = os.exit
getenv = os.getenv
remove = os.remove
rename = os.rename
setlocale = os.setlocale
time = os.time
tmpname = os.tmpname

-------------------------------------------------------------------
-- compatibility only
getglobal = function (n) return _G[n] end
setglobal = function (n,v) _G[n] = v end

-------------------------------------------------------------------

local io, tab = io, table

-- IO library (files)
_STDIN = io.stdin
_STDERR = io.stderr
_STDOUT = io.stdout
_INPUT = io.stdin
_OUTPUT = io.stdout
seek = io.stdin.seek   -- sick ;-)
tmpfile = io.tmpfile
closefile = io.close
openfile = io.open

function flush (f)
  if f then f:flush()
  else _OUTPUT:flush()
  end
end

function readfrom (name)
  if name == nil then
    local f, err, cod = io.close(_INPUT)
    _INPUT = io.stdin
    return f, err, cod
  else
    local f, err, cod = io.open(name, "r")
    _INPUT = f or _INPUT
    return f, err, cod
  end
end

function writeto (name)
  if name == nil then
    local f, err, cod = io.close(_OUTPUT)
    _OUTPUT = io.stdout
    return f, err, cod
  else
    local f, err, cod = io.open(name, "w")
    _OUTPUT = f or _OUTPUT
    return f, err, cod
  end
end

function appendto (name)
  local f, err, cod = io.open(name, "a")
  _OUTPUT = f or _OUTPUT
  return f, err, cod
end

function read (...)
  local f = _INPUT
  if rawtype(arg[1]) == 'userdata' then
    f = tab.remove(arg, 1)
  end
  return f:read(unpack(arg))
end

function write (...)
  local f = _OUTPUT
  if rawtype(arg[1]) == 'userdata' then
    f = tab.remove(arg, 1)
  end
  return f:write(unpack(arg))
end

