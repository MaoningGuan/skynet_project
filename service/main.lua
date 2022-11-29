local skynet = require 'skynet'
local socket = require 'skynet.socket'

local clients = {}

local function connect(fd, addr)
    print(fd..' connect addr:'..addr)
    socket.start(fd)
    clients[fd] = {}
    -- 消息处理
    while true do
        local read_data = socket.read(fd) -- 利用协程实现阻塞模式
        if read_data then
            print(fd..' recv '..read_data)
            for key, value in pairs(clients) do
                socket.write(key, read_data)
            end
        -- 断开连接
        elseif read_data == 'q' then
            print(fd..' close ')
            clients[fd] = nil
        end
    end
end

skynet.start(function ()
    skynet.newservice("debug_console", 8000)

	local ping1 = skynet.newservice("ping")
	local ping2 = skynet.newservice("ping")
	local ping3 = skynet.newservice("ping")

	skynet.send(ping1, "lua", "start", ping2)
	skynet.send(ping2, "lua", "start", ping3)
	skynet.exit()
end)