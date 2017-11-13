module("Timer",package.seeall)

function Init(is_reload)
    print("init module Timer ", is_reload)
end

function Update(curr_time)
    sg_print("update " .. curr_time)
    print("update " .. curr_time)
end 
