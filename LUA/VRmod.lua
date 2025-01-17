print("Initializing VRmod.lua")

UEVR_UObjectHook.activate()

local api = uevr.api
--local uobjects = uevr.types.FUObjectArray.get()
local playerController = api:get_player_controller(0)
local weapon
local rotationAbsoluteSet = true

local function checkWeaponChange()
    --print(playerController:get_full_name())
    local newWeapon = playerController.Children[5]
    if newWeapon ~= nil
    then
        --print(newWeapon:get_full_name())
        if weapon ~= newWeapon then
            rotationAbsoluteSet = false
            weapon = newWeapon
        end
    end
end

local function setAbsoluteRotationOnWeaponMesh()
    local weaponMesh = weapon.WeaponMesh
    --print(weaponMesh:get_full_name())
    weaponMesh:SetAbsolute(true, true, true)
end

uevr.sdk.callbacks.on_pre_engine_tick(function(engine, delta)
    checkWeaponChange()
    if not rotationAbsoluteSet then
        setAbsoluteRotationOnWeaponMesh()
        rotationAbsoluteSet = true;
    end
end)

