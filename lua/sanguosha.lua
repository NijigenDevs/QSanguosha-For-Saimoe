--[[********************************************************************
	Copyright (c) 2013-2015 Mogara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  Mogara
*********************************************************************]]

-- This is the start script of QSanguosha

package.path = package.path .. ";./lua/lib/?.lua"

local old_type = type

type = function(object)
	local t = old_type(object)
	if t == "userdata" then
		local meta = getmetatable(object)
		if meta and meta[".type"] then
			return meta[".type"]
		else
			return t
		end
	else
		return t
	end
end

dofile "lua/utilities.lua"
dofile "lua/sgs_ex.lua"
dofile "lua/about_us.lua"

function load_translation(file)
	local t = dofile(file)
	if type(t) ~= "table" then
		error(("file %s is should return a table!"):format(file))
	end

	sgs.LoadTranslationTable(t)
end

function load_translations()
	local lang = sgs.GetConfig("Language", "zh_CN")
	local subdir = { "", "Audio", "Package" }
	for _, dir in ipairs(subdir) do
		local lang_dir = "lang/" .. lang .. "/" .. dir
		local lang_files = sgs.GetFileNames(lang_dir)
		for _, file in ipairs(lang_files) do
			load_translation(("%s/%s"):format(lang_dir, file))
		end
	end
end
--[[
function load_extensions(just_require)
	local scripts = sgs.GetFileNames("extensions")
	local package_names = {}
	for _, script in ipairs(scripts) do
		if script:match(".+%.lua$") then
			local name = script:sub(script:find("%w+"))
			local module_name = "extensions." .. name
			local loaded = require(module_name)
			if not loaded.hidden then
				table.insert(package_names, name)
				sgs.Sanguosha:addPackage(loaded.extension)
			end
		end
	end
	local lua_packages = ""
	if #package_names > 0 then lua_packages = table.concat(package_names, "+") end
	sgs.SetConfig("LuaPackages", lua_packages)
end
]]

global_packages = {}

function load_extensions()
	local scripts = sgs.GetFileNames("extensions")
	local package_names = {}
	for _, script in ipairs(scripts) do
		if script:match(".+%.lua$") then
			local extensions = dofile("./extensions/" .. script)
			if type(extensions) ~= "table" then
				extensions = {extensions}
			end
			for _, extension in ipairs(extensions) do
				local name = extension:objectName()
				table.insert(package_names, name)
				if extension:inherits("LuaScenario") then
					sgs.Sanguosha:addScenario(extension)
				elseif extension:inherits("Package") then
					sgs.Sanguosha:addPackage(extension)
				end
				table.insert(global_packages, extension)
			end
		end
	end
	local lua_packages = ""
	if #package_names > 0 then lua_packages = table.concat(package_names, "+") end
	sgs.SetConfig("LuaPackages", lua_packages)
end

if not sgs.GetConfig("DisableLua", false) then
	load_extensions()
end

local done_loading = sgs.Sanguosha:property("DoneLoading"):toBool()
if not done_loading then
	load_translations()
	done_loading = sgs.QVariant(true)
	sgs.Sanguosha:setProperty("DoneLoading", done_loading)

	createAboutUsPages()
end
