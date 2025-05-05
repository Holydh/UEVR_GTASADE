local api = uevr.api
local vr = uevr.params.vr

-- All values are sent from the C++ plugin and retrieved in lua events below. Check GTASADE_LuaEvents.lua to see what events are raised.
local isPlayerDriving = false;
local leftHandedMode = 0;
local leftHandedOnlyWhileOnFoot = true;


-- swap both triggers
uevr.sdk.callbacks.on_xinput_get_state(function(retval, user_index, state)
    if (leftHandedMode == 1 and leftHandedOnlyWhileOnFoot and not isPlayerDriving) then
        local leftTrigger = state.Gamepad.bLeftTrigger
        local rightTrigger = state.Gamepad.bRightTrigger
        state.Gamepad.bRightTrigger = leftTrigger
        state.Gamepad.bLeftTrigger = rightTrigger
    end
end)

uevr.sdk.callbacks.on_lua_event(function(event_name, event_string)
    if (event_name == "playerIsLeftHanded") then
        local value = tonumber(event_string)
        if value and value == math.floor(value) then
            leftHandedMode = value
        end
        --print("Left handed mode : " .. tostring(leftHandedMode))
    end
    if (event_name == "leftHandedOnlyWhileOnFoot") then
        if (event_string == "true") then
            leftHandedOnlyWhileOnFoot = true;
        else
            leftHandedOnlyWhileOnFoot = false;
        end
        --print("Left handed only while on foot : " .. tostring(leftHandedOnlyWhileOnFoot))
    end
    if (event_name == "playerState") then
        if event_string ~= "OnFoot" then
            isPlayerDriving = true
        else
            isPlayerDriving = false
        end
        print("isPlayerDriving : " .. tostring(isPlayerDriving))
    end
end)
