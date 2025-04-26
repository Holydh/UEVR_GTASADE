local api = uevr.api
local vr = uevr.params.vr

local leftHandedMode = false;
local isPlayerDriving = false;

-- swap both triggers
uevr.sdk.callbacks.on_xinput_get_state(function(retval, user_index, state)
    -- print("leftHandedMode :")
    -- print(leftHandedMode)
    -- print("isPlayerDriving :")
    -- print(isPlayerDriving)
    if (leftHandedMode and not isPlayerDriving) then
        local leftTrigger = state.Gamepad.bLeftTrigger
        local rightTrigger = state.Gamepad.bRightTrigger
        state.Gamepad.bRightTrigger = leftTrigger
        state.Gamepad.bLeftTrigger = rightTrigger
    end
end)

uevr.sdk.callbacks.on_lua_event(function(event_name, event_string)
    if (event_name == "playerIsLeftHanded") then
        if (event_string == "true") then
            leftHandedMode = true
        end
        if (event_string == "false") then
            leftHandedMode = false
        end
    end
    if (event_name == "playerIsInVehicle") then
        if (event_string == "true") then
            isPlayerDriving = true
        end
        if (event_string == "false") then
            isPlayerDriving = false
        end
    end
end)
