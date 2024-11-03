-- mod-version:3
local core = require "core"
local command = require "core.command"
local common = require "core.common"
local style = require "core.style"
local View = require "core.view"
local keymap = require "core.keymap"

local HelloView = View:extend()

local filename
local filecontents

local function is_file(file_path)
  local file_info = system.get_file_info(file_path)
  if file_info and file_info.type == "file" then
    return true
  end
  return false
end

local function readfile(file_path)
  local file = io.open(file_path, "r")
  if not file then
    core.log("**** cannot read filename %s",filename) 
  else
    local content = file:read "*a"  -- *a or *all reads whole file
    file:close()
    return content  
  end
end

command.add(nil, {
  ["load:file"] = function()
    core.command_view:enter("filename to read", {
      submit = function(text)
        filename = text
        core.log("*** filename %s",filename)
        if not is_file(filename) then
          core.log("**** filename %s not found",filename)
          return
        end
        filecontents = readfile(filename)
        core.log("Length %i",#filecontents)
      end -- submit
    })
   end
})

keymap.add {
  ["alt+s"] = "load:file",
}
