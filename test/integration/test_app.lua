local skynet = require 'skynet'
require "skynet.manager"
local ldbus = require "ldbus"

local function prepare_data()
    skynet.error("---------prepare_data-------")
end

local function clear_data()
    skynet.error("---------clear_data-------")
    skynet.timeout(10, function ()
        skynet.abort()
    end)
end

local function test_main()
    skynet.error("---------test main start-------")
    os.execute("loginctl enable-linger $USER")
    skynet.setenv("XDG_RUNTIME_DIR", "/run/user/$(id -u)/dbus-1")
    local conn = assert ( ldbus.bus.get ( "session" ) )
    assert ( ldbus.bus.request_name ( conn , "test.signal.source" , { replace_existing = true } ) )

    skynet.error("---------test main complete-------")
end

skynet.start(function ()
    prepare_data()
    skynet.uniqueservice("main")
    skynet.fork(function ()
        local ok, err = pcall(test_main)
        -- clear_data()
        if not ok then
            error(err)
        end
    end)
end)