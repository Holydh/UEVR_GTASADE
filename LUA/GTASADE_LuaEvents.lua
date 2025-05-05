local api = uevr.api
local vr = uevr.params.vr

-- All values are sent from the C++ plugin and retrieved in lua events below.
local playerState; -- String Value. Can be : OnFoot, CarOrBoat, Bike, Helicopter, Plane
local vehicleCameraMode; -- String Value. Can be : Road, FPS, Normal, Far, Cinematic
local onFootCameraMode; -- String Value. Can be : FPS, Normal, Far
local leftHandedMode = 0; -- Int value. 0 = Disabled, 1 = triggers swap, 2 = Full input swap
local leftHandedOnlyWhileOnFoot = true;



uevr.sdk.callbacks.on_lua_event(function(event_name, event_string)
    if (event_name == "playerState") then
        playerState = event_string
        print("Player State : " .. event_string)
    end
        if (event_name == "onFootCameraMode") then
        vehicleCameraMode = event_string
        print("On Foot Camera Mode : " .. event_string)
    end
    if (event_name == "vehicleCameraMode") then
        vehicleCameraMode = event_string
        print("Vehicle Camera Mode : " .. event_string)
    end
    if (event_name == "playerIsLeftHanded") then
        local value = tonumber(event_string)
        if value and value == math.floor(value) then
            leftHandedMode = value
        end
        print("Left handed mode : " .. tostring(leftHandedMode))
    end
    if (event_name == "leftHandedOnlyWhileOnFoot") then
        if (event_string == "true") then
            leftHandedOnlyWhileOnFoot = true
        else
            leftHandedOnlyWhileOnFoot = false
        end
        print("Left handed only while on foot : " .. tostring(leftHandedOnlyWhileOnFoot))
    end
end)
