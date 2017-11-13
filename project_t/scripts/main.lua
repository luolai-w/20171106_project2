module("Main",package.seeall)
local GetPWD = GameLib.GetPWD
local ListDir = GameLib.ListDir

local is_reload = ...
print('check reload',is_reload)
errlog('check reload',is_reload)
sg_print('check reload',is_reload)


local lua_root_path = GetPWD() .. '/../scripts'
package.path = package.path .. ";" ..lua_root_path .. "/?.lua"

sg_print("lua_root_path = " .. lua_root_path) --test

g_all_loaded_module = g_all_loaded_module or {}

local function Load_all_module()
    local ls_dir = ListDir(lua_root_path)
    
    for i, dir_info in pairs(ls_dir) do
        if dir_info.type == 'dir' then
            local m_name = dir_info.name
            local files = ListDir(lua_root_path .. '/' .. m_name)
            for i, file in pairs(files) do
                if file.type == 'file' and file.name:find("%a%.lua$") then
                    local prefix_name = file.name:sub(1,-5);       
                    require(m_name .. '/' .. prefix_name)
                    if package.loaded[m_name] then
                        g_all_loaded_module[m_name] = true
                        print(string.format("load %s finished.",m_name))
                    end
                end
            end
        end
    end
end

local function Unload_all_module()
    for k, v in pairs(g_all_loaded_module) do
        package.loaded[k] = nil
    end
end

local function Init_all_module()
    for k,v in pairs(g_all_loaded_module) do
        local m = package.loaded[k]
        if m and m.Init then
            m.Init(is_reload)
        end
    end
end

Unload_all_module();
Load_all_module();
Init_all_module();

errlog(tostring_r(package.loaded,1))
