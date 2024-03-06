CheckVersion("0.5")

settings = NewSettings()

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function get_device()
    local device = ScriptArgs["device"]
    if device == nil then
        if family == "unix" then
          return "cloud"
        end
        return "laptop"
    end
    return device
end

function get_config()
    local config = ScriptArgs["config"]
    if config == nil then
        return "release"
    end
    return config
end

function intermediate_output(settings, input)
	return "objs/" .. string.sub(PathBase(input), string.len("src/")+1) .. settings.config_ext
end

function set_compiler(config)
	if config == "release" then
		settings.config_ext = ""
		settings.debug = 0
		settings.optimize = 1
		settings.cc.defines:Add("CONF_RELEASE")
	elseif config == "debug" then
		settings.config_ext = "_d"
		settings.debug = 1
		settings.optimize = 0
		settings.cc.defines:Add("CONF_DEBUG")
	end
	
	if ScriptArgs["dummy"] == "1" then
		settings.cc.defines:Add("CONF_DUMMY")
	end
end

function ResCompile(scriptfile)
	scriptfile = Path(scriptfile)
	output = PathBase(scriptfile) .. ".res"
	AddJob(output, "rc " .. scriptfile, "rc /fo " .. output .. " " .. scriptfile)
	AddDependency(output, scriptfile)
	return output
end

device = get_device()

config = get_config()
settings.cc.Output = intermediate_output
settings.cc.includes:Add("src")
set_compiler(config)


if family == "unix" then
	settings.link.libs:Add("pthread")
	settings.cc.flags:Add("-std=c++11")
end

settings_hive = settings:Copy()

if ScriptArgs["dummy"] ~= "1" then
	settings_hive.link.libs:Add("ssl")
	settings_hive.link.libs:Add("crypto")
	settings_hive.link.libs:Add("archive")
end

description = nul
if family == "windows" and device ~= "male" then
	icon_wasp = {ResCompile("icon/wasp.rc")}
	icon_logo = {ResCompile("icon/logo.rc")}
	description = {ResCompile("resources/descwasp.rc")}
end

name = nul
if device == "laptop" then
	settings.cc.defines:Add("CONF_DEVICE_LAPTOP")
	name = "lapwasp"
  
elseif device == "cloud" then
	settings.cc.defines:Add("CONF_DEVICE_CLOUD")
	name = "cloudwasp"
  
elseif device == "tower" then
	settings.cc.defines:Add("CONF_DEVICE_TOWER")
	name = "towerwasp"
elseif device == "pi" then
	settings.cc.defines:Add("CONF_DEVICE_PI")
	name = "raspiwasp"
elseif device == "male" then
	settings.cc.defines:Add("CONF_DEVICE_MALE")
	name = "male"
elseif ScriptArgs["dummy"] == "1" then
	settings.cc.defines:Add("CONF_DEVICE_LAPTOP")
end
-- Override name
if ScriptArgs["dummy"] == "1" then
	name = name.."_dummy"
end

source_core = CollectRecursive("src/core/*.cpp")
objects_core = Compile(settings, source_core);

if ScriptArgs["nohive"] ~= "1" then
	source_hive = CollectRecursive("src/hive/*.cpp")
	objects_hive = Compile(settings_hive, source_hive)
	Link(settings_hive, "hive", objects_core, objects_hive, icon_logo)
end

source_wasp = Collect("src/wasp/*.cpp")

if family == "windows" then
	table.insert(source_wasp, Collect("src/wasp/windows/*.cpp"))
end

if device == "pi" then
	table.insert(source_wasp, Collect("src/wasp/pi/*.cpp"))
	
	if ScriptArgs["dummy"] ~= "1" then
		settings.link.libs:Add("mosquitto")
	end
end

print("Building " .. name .. " for " .. device .. " in " .. config .. " mode")
objects_wasp = Compile(settings, source_wasp)
Link(settings, name, objects_core, objects_wasp, icon_wasp, description)
