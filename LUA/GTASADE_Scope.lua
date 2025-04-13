-- Script made by Mutar for Stalker 2
-- Tweaked by Holydh

local debugMode = false
local api = uevr.api

-- Static variables

-- You can lower these values for better performance. Keep the aspect ratio to avoid stretching.
local sniperRenderTargetResolution = {1024, 1024}
-- default sniper values : {1024, 1024}
local cameraRenderTargetResolution = {720, 1280}
-- default camera values : {720, 1280}
local emissive_material_name = "Material /Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"
local cylinder_mesh_name = "StaticMesh /Engine/BasicShapes/Cylinder.Cylinder"
local red_dot_texture_name = "Texture2D /Game/SanAndreas/Textures/gta3/Tilables/T_carpet_red_256_BC.T_carpet_red_256_BC"
local plane_mesh_name = "StaticMesh /Engine/BasicShapes/Plane.Plane"
local sniper_fname = "SM_sniper"
local camera_fname = "SM_camera"
local unarmed_fname ="unarmed"
local ftransform_c = nil
local flinearColor_c = nil
local hitresult_c = nil
local game_engine_c = nil
local Statics = nil
local KismetStringLibrary = nil
local KismetMathLibrary = nil
local KismetRenderingLibrary = nil
local KismetMaterialLibrary = nil
local KismetSystemLibrary = nil
local actor_c = nil
local static_mesh_c = nil
local texture2D_c = nil
local scene_capture_component_c = nil
local skeletal_mesh_component_c = nil
local static_mesh_component_c = nil
local cameraManager_c = nil
local weapon_c = nil
local cylinder_static_mesh = nil
local emissive_material_amplifier = 2.0 

-- Instance variables
local scope_actor = nil
local scope_plane_component = nil
local red_dot_plane_component = nil
local scene_capture_component_mesh = nil
local scene_capture_component = nil
local cameraRenderTarget = nil
local sniperRenderTarget = nil
local actual_render_target = nil
local reusable_hit_result = nil
local temp_vec3 = Vector3d.new(0, 0, 0)
local temp_vec3f = Vector3f.new(0, 0, 0)
local zero_color = nil
local zero_transform = nil
local isSniper = false
local cylinder_initialized = false
local cylinder_actor = nil
local previous_weapon_fname = nil

local function find_required_object(name)
    local obj = api:find_uobject(name)
    if not obj then
        error("Cannot find " .. name)
        return nil
    end
    return obj
end

local function find_required_object_no_cache(class, full_name)
    local matches = class:get_objects_matching(false)
    for i, obj in ipairs(matches) do
        if obj ~= nil and obj:get_full_name() == full_name then
            return obj
        end
    end
    return nil
end

local find_static_class = function(name)
    local c = find_required_object(name)
    return c:get_class_default_object()
end

local function init_static_objects()
    -- Try to initialize all required objects
    ftransform_c = find_required_object("ScriptStruct /Script/CoreUObject.Transform")
    if not ftransform_c then return false end
    print(ftransform_c:get_full_name())
    flinearColor_c = find_required_object("ScriptStruct /Script/CoreUObject.LinearColor")
    if not flinearColor_c then return false end
    print(flinearColor_c:get_full_name())
    hitresult_c = find_required_object("ScriptStruct /Script/Engine.HitResult")
    if not hitresult_c then return false end
    print(hitresult_c:get_full_name())
    game_engine_c = find_required_object("Class /Script/Engine.GameEngine")
    if not game_engine_c then return false end
    print(game_engine_c:get_full_name())
    Statics = find_static_class("Class /Script/Engine.GameplayStatics")
    if not Statics then return false end
    print(Statics:get_full_name())

    KismetStringLibrary = find_static_class("Class /Script/Engine.KismetStringLibrary")
    if not KismetStringLibrary then return false end
    print(KismetStringLibrary:get_full_name())
    KismetMathLibrary = find_static_class("Class /Script/Engine.KismetMathLibrary")
    if not KismetMathLibrary then return false end
    print(KismetMathLibrary:get_full_name())

    KismetRenderingLibrary = find_static_class("Class /Script/Engine.KismetRenderingLibrary")
    if not KismetRenderingLibrary then return false end
    print(KismetRenderingLibrary:get_full_name())
    KismetMaterialLibrary = find_static_class("Class /Script/Engine.KismetMaterialLibrary")
    if not KismetMaterialLibrary then return false end
    print(KismetMaterialLibrary:get_full_name())
    KismetSystemLibrary = find_static_class("Class /Script/Engine.KismetSystemLibrary")
    if not KismetSystemLibrary then return false end
    print(KismetSystemLibrary:get_full_name())
    actor_c = find_required_object("Class /Script/Engine.Actor")
    if not actor_c then return false end
    print(actor_c:get_full_name())
    static_mesh_component_c = find_required_object("Class /Script/Engine.StaticMeshComponent")
    if not static_mesh_component_c then return false end
    print(static_mesh_component_c:get_full_name())
    static_mesh_c = find_required_object("Class /Script/Engine.StaticMesh")
    if not static_mesh_c then return false end
    print(static_mesh_c:get_full_name())
    scene_capture_component_c = find_required_object("Class /Script/Engine.SceneCaptureComponent2D")
    if not scene_capture_component_c then return false end
    print(scene_capture_component_c:get_full_name())
    skeletal_mesh_component_c = api:find_uobject("Class /Script/Engine.SkeletalMeshComponent")
    if not skeletal_mesh_component_c then return false end
    print(skeletal_mesh_component_c:get_full_name())
    static_mesh_component_c = api:find_uobject("Class /Script/Engine.StaticMeshComponent")
    if not static_mesh_component_c then return false end
    print(static_mesh_component_c:get_full_name())
    cameraManager_c = find_required_object("Class /Script/Engine.PlayerCameraManager")
    if not cameraManager_c then return false end
    print(cameraManager_c:get_full_name())

    weapon_c = find_required_object("Class /Script/GTABase.GTAWeapon")
    if not weapon_c then return false end
    print(weapon_c:get_full_name())

    -- Initialize reusable objects
    reusable_hit_result = StructObject.new(hitresult_c)
    if not reusable_hit_result then return false end

    zero_color = StructObject.new(flinearColor_c)
    if not zero_color then return false end
    
    zero_transform = StructObject.new(ftransform_c)
    if not zero_transform then return false end
    zero_transform.Rotation.W = 1.0
    zero_transform.Scale3D = temp_vec3:set(1.0, 1.0, 1.0)

    texture2D_c = find_required_object("Class /Script/Engine.Texture2D")
    if not texture2D_c then return false end
    print(texture2D_c:get_full_name())

    print("init done")
    return true
end

local function reset_static_objects()
    ftransform_c = nil
    flinearColor_c = nil
    hitresult_c = nil
    game_engine_c = nil
    Statics = nil
    KismetRenderingLibrary = nil
    KismetMaterialLibrary = nil
    AssetRegistry = nil
    actor_c = nil
    static_mesh_component_c = nil
    static_mesh_c = nil
    scene_capture_component_c = nil
    skeletal_mesh_component_c = nil
    static_mesh_component_c = nil
    cameraManager_c = nil

    
    reusable_hit_result = nil
    zero_color = nil
    zero_transform = nil
end

local function validate_object(object)
    if object == nil or not UEVR_UObjectHook.exists(object) then
        return nil
    else
        return object
    end
end

local function destroy_actor(actor)
    if actor ~= nil and not UEVR_UObjectHook.exists(actor) then
        pcall(function() 
            if actor.K2_DestroyActor ~= nil then
                actor:K2_DestroyActor()
            end
        end)
    end
    return nil
end


local function spawn_actor(world_context, actor_class, location, collision_method, owner)

    local actor = Statics:BeginDeferredActorSpawnFromClass(world_context, actor_class, zero_transform, collision_method, owner)

    if actor == nil then
        print("Failed to spawn actor")
        return nil
    end

    Statics:FinishSpawningActor(actor, zero_transform)
    print("Spawned actor")

    return actor
end


local function get_equipped_weapon(playerController)
    if not playerController then return nil end
   
    local playerControllerChildren = playerController.Children
    local weapon
    for i, child in ipairs(playerControllerChildren) do
        if child:is_a(weapon_c) then
            weapon = child
        end
    end
    local weapon_mesh = nil
    if weapon ~= nil then
        weapon_mesh = weapon.WeaponMesh
    end
    return weapon_mesh
end

local function get_render_target(world, isSniper)
    sniperRenderTarget = validate_object(sniperRenderTarget)
    cameraRenderTarget = validate_object(cameraRenderTarget)
    if cameraRenderTarget == nil then
        cameraRenderTarget = KismetRenderingLibrary:CreateRenderTarget2D(world, cameraRenderTargetResolution[1], cameraRenderTargetResolution[2], 6, zero_color, false)
    end
    if sniperRenderTarget == nil then
        sniperRenderTarget = KismetRenderingLibrary:CreateRenderTarget2D(world, sniperRenderTargetResolution[1], sniperRenderTargetResolution[2], 6, zero_color, false)
    end
    if isSniper then
        actual_render_target = sniperRenderTarget
    else
        actual_render_target = cameraRenderTarget
    end
    print("Render Target Created " .. actual_render_target:get_full_name())
    return actual_render_target
end

local function spawn_scope_plane(world, owner, pos, rt, isSniper)
    local local_scope_mesh = scope_actor:AddComponentByClass(static_mesh_component_c, false, zero_transform, false)
    local local_red_dot_mesh = scope_actor:AddComponentByClass(static_mesh_component_c, false, zero_transform, false)
    if local_scope_mesh == nil then
        print("Failed to spawn scope mesh")
        return
    end

    local wanted_mat = api:find_uobject(emissive_material_name)
    if wanted_mat == nil then
        print("Failed to find material")
        return
    end

    wanted_mat:set_property("TwoSided", false)
    wanted_mat:set_property("BlendMode", 0)
    wanted_mat:set_property("bDisableDepthTest", true)
    wanted_mat:set_property("MaterialDomain", 0)
    wanted_mat:set_property("ShadingModel", 0)

    print(wanted_mat:get_full_name())

    local scopePlane
    local redDotPlane
    local redDotTexture
    if isSniper then
        scopePlane = find_required_object_no_cache(static_mesh_c, cylinder_mesh_name)
        redDotPlane = find_required_object_no_cache(static_mesh_c, cylinder_mesh_name)
        redDotTexture = find_required_object_no_cache(texture2D_c, red_dot_texture_name)
    else
        scopePlane = find_required_object_no_cache(static_mesh_c, plane_mesh_name)
    end

    if scopePlane == nil then
        print("Failed to find plane mesh")
        return
    end
    
    
    local_scope_mesh:SetStaticMesh(scopePlane)
    local_scope_mesh:SetVisibility(false)
    local_scope_mesh:SetCollisionEnabled(0)

    if isSniper then
        local_red_dot_mesh:SetStaticMesh(redDotPlane)
        local_red_dot_mesh:SetVisibility(false)
        local_red_dot_mesh:SetCollisionEnabled(0)
    end


    local scope_dynamic_material = local_scope_mesh:CreateAndSetMaterialInstanceDynamicFromMaterial(0, wanted_mat)
    local red_dot_dynamic_material = local_red_dot_mesh:CreateAndSetMaterialInstanceDynamicFromMaterial(0, wanted_mat)

    scope_dynamic_material:SetTextureParameterValue(KismetStringLibrary:Conv_StringToName("LinearColor"), rt)
    
    local color = StructObject.new(flinearColor_c)
    color.R = emissive_material_amplifier
    color.G = emissive_material_amplifier
    color.B = emissive_material_amplifier
    color.A = emissive_material_amplifier
    scope_dynamic_material:SetVectorParameterValue(KismetStringLibrary:Conv_StringToName("Color"), color)

    if isSniper then
        red_dot_dynamic_material:SetTextureParameterValue(KismetStringLibrary:Conv_StringToName("LinearColor"), redDotTexture)
        local redDotColor = StructObject.new(flinearColor_c)
        redDotColor.R = 0.6
        redDotColor.G = 0.6
        redDotColor.B = 0.6
        redDotColor.A = 0.6
        red_dot_dynamic_material:SetVectorParameterValue(KismetStringLibrary:Conv_StringToName("Color"), redDotColor)
    end

    scope_plane_component = local_scope_mesh
    red_dot_plane_component = local_red_dot_mesh
    print("Scope plane spawned")
end

local function spawn_scene_capture_component(world, owner, pos, fov, rt)
    scene_capture_component_mesh = scope_actor:AddComponentByClass(static_mesh_component_c, false, zero_transform, false)
    scene_capture_component = scope_actor:AddComponentByClass(scene_capture_component_c, false, zero_transform, false)
    if scene_capture_component == nil then
        print("Failed to spawn scene capture")
        return
    end
    if scene_capture_component_mesh == nil then
        print("Failed to spawn scene capture mesh")
        return
    end
    scene_capture_component.TextureTarget = rt
    scene_capture_component:SetVisibility(false)
    scene_capture_component_mesh:SetVisibility(false)
    print("scene_capture_component spawned")
end

local function spawn_scope(game_engine, weaponMesh, isSniper)
    local viewport = game_engine.GameViewport
    if viewport == nil then
        print("Viewport is nil")
        return
    end

    local world = viewport.World
    if world == nil then
        print("World is nil")
        return
    end

    if not weaponMesh then
        print("weaponMesh is nil")
        return
    end

    local childrenArray = weaponMesh.AttachChildren
    if (childrenArray ~= nil and #childrenArray > 0) then
        for i, child in ipairs(childrenArray) do
            if child:is_a(static_mesh_component_c) then
                child:K2_DestroyComponent(child)
                scope_plane_component = nil
                red_dot_plane_component = nil
            end
        end
    end

    local rt = get_render_target(world, isSniper)

    if rt == nil then
        print("Failed to get render target destroying actors")
        rt = nil
        scope_actor = destroy_actor(scope_actor)
        scope_plane_component = nil
        red_dot_plane_component = nil
        scene_capture_component = nil
        scene_capture_component_mesh = nil
        return
    end

    local weaponPos = weaponMesh:K2_GetComponentLocation()
    if not validate_object(scope_actor) then
        scope_actor = destroy_actor(scope_actor)
        scope_plane_component = nil
        red_dot_plane_component = nil
        scene_capture_component = nil
        scene_capture_component_mesh = nil
        scope_actor = spawn_actor(world, actor_c, temp_vec3:set(0, 0, 0), 1, nil)
        if scope_actor == nil then
            print("Failed to spawn scope actor")
            return
        end
    end

    if not validate_object(scope_plane_component) then
        print("scope_plane_component is invalid -- recreating")
        spawn_scope_plane(world, nil, weaponPos, rt, isSniper)
    end

    if not validate_object(scene_capture_component) then
        print("spawn_scene_capture_component is invalid -- recreating")
        spawn_scene_capture_component(world, nil, weaponPos, rt)
    end

    scene_capture_component.TextureTarget = rt
end

local function attach_components_to_weapon(weapon_mesh, isSniper)
    if not weapon_mesh then return end
    
    local rotation
    if isSniper then
        rotation = KismetMathLibrary:FindLookAtRotation(temp_vec3:set(5.94806 , -2.75068, 13.2024),temp_vec3f:set(30.6871 , -0.22823, 15.6848))
    else
        rotation = KismetMathLibrary:FindLookAtRotation(temp_vec3:set(13.8476, -11.6162, 1.72577),temp_vec3f:set(27.6432, -11.6162, 2.84382))
    end
    --print("rotation x = " .. rotation.x .. " rotation y = " .. rotation.y ..  " rotation z = " .. rotation.z)

    -- Attach scene capture to weapon
    if scene_capture_component ~= nil and scene_capture_component_mesh ~=  nil then
        print("Attaching scene_capture_component to weapon:" .. weapon_mesh:get_fname():to_string())
        scene_capture_component_mesh:K2_AttachToComponent(
            weapon_mesh,
            "gunflash",
            2, -- Location rule
            2, -- Rotation rule
            0, -- Scale rule
            true -- Weld simulated bodies
        )

        if isSniper then
            
            scene_capture_component_mesh:K2_SetRelativeRotation(rotation, false, reusable_hit_result, false)
            scene_capture_component_mesh:K2_SetRelativeLocation(temp_vec3:set(30.6871 , -0.22823, 15.6848), false, reusable_hit_result, false)
            scene_capture_component_mesh:SetVisibility(false)
        else
            local test = temp_vec3:set(rotation.x, rotation.y, rotation.z - 90)
            scene_capture_component_mesh:K2_SetRelativeRotation( test, false, reusable_hit_result, false)
            scene_capture_component_mesh:K2_SetRelativeLocation(temp_vec3:set(27.6432, -11.6162, 2.84382), false, reusable_hit_result, false)
            scene_capture_component_mesh:SetVisibility(false)
        end

        scene_capture_component:K2_AttachToComponent(
            scene_capture_component_mesh,
            "gunflash",
            2, -- Location rule
            2, -- Rotation rule
            0, -- Scale rule
            true -- Weld simulated bodies
        )

        if isSniper then
            scene_capture_component:SetVisibility(false)
        else
            scene_capture_component:SetVisibility(false)
        end
    end
    
    -- Attach plane to weapon
    if scope_plane_component then
        if weapon_mesh == nil then
            print("Failed to find weapon mesh")
            return
        end
        -- OpticCutoutSocket
        scope_plane_component:K2_AttachToComponent(
            weapon_mesh,
            "gunflash",
            2, -- Location rule
            2, -- Rotation rule
            2, -- Scale rule
            true -- Weld simulated bodies
        )

        if isSniper then
            local test = temp_vec3:set(rotation.x + 90, rotation.y, rotation.z)
            scope_plane_component:K2_SetRelativeRotation(test, false, reusable_hit_result, false)
            scope_plane_component:K2_SetRelativeLocation(temp_vec3:set(5.91537, -2.75402, 13.1992), false, reusable_hit_result, false)
            scope_plane_component:SetWorldScale3D(temp_vec3:set(0.033, 0.033, 0.000001))
            red_dot_plane_component:K2_AttachToComponent(
                scope_plane_component,
                "gunflash",
                2, -- Location rule
                2, -- Rotation rule
                2, -- Scale rule
                true -- Weld simulated bodies
            )
            red_dot_plane_component:K2_SetRelativeLocation(temp_vec3:set(0, 0, 11800), false, reusable_hit_result, false)
            red_dot_plane_component:SetWorldScale3D(temp_vec3:set(0.0018, 0.0018, 0.000001))
        else
            local test = temp_vec3:set(rotation.x + 90, rotation.y, rotation.z)
            scope_plane_component:K2_SetRelativeRotation(test, false, reusable_hit_result, false)
            scope_plane_component:K2_SetRelativeLocation(temp_vec3:set(4.1, -12.494, 1.537), false, reusable_hit_result, false)
            scope_plane_component:SetWorldScale3D(temp_vec3:set(0.06, 0.106, 0))
        end
       
        scope_plane_component:SetVisibility(false)
        red_dot_plane_component:SetVisibility(false)
        print("Scope attached")
    end
end

local function switch_scope_state(state)
    if scene_capture_component ~= nil then
        scene_capture_component:SetVisibility(state)
    end
    if scope_plane_component ~= nil then
        scope_plane_component:SetVisibility(state)
    end
    if red_dot_plane_component ~= nil then
        red_dot_plane_component:SetVisibility(state)
    end
end

local function mapRange(value, minSource, maxSource, minTarget, maxTarget)
    return minTarget + ((value - minSource) / (maxSource - minSource)) * (maxTarget - minTarget)
end

-- Initialize static objects when the script loads
if not init_static_objects() then
    print("Failed to initialize static objects")
end

local current_weapon = nil
local last_level = nil

uevr.sdk.callbacks.on_pre_engine_tick(
	function(engine, delta)
        local viewport = engine.GameViewport
        if viewport then
            local world = viewport.World
            if world then

                if validate_object(cylinder_static_mesh) == nil then
                    cylinder_static_mesh = api:find_uobject("StaticMesh /Engine/BasicShapes/Cylinder.Cylinder")
                end
                if not cylinder_initialized and cylinder_static_mesh ~= nil then
                    cylinder_actor = spawn_actor(world, actor_c, temp_vec3:set(0, 0, 0), 1, nil)
                    if cylinder_actor ~= nil then
                        local cylinderMesh = cylinder_actor:AddComponentByClass(static_mesh_component_c, false, zero_transform, false)
                        cylinderMesh:SetStaticMesh(cylinder_static_mesh)
                        cylinderMesh:SetVisibility(false)
                        cylinderMesh:SetCollisionEnabled(0)
                    end
                    cylinder_initialized = true
                end
                local level = world.PersistentLevel

                if last_level ~= level then
                    print("Level changed .. Reseting")
                    destroy_actor(scope_actor)
                    scope_plane_component = nil
                    red_dot_plane_component = nil
                    scene_capture_component = nil
                    actual_render_target = nil
                    reset_static_objects()
                    init_static_objects()
                end
                last_level = level
            end
        end
        local playerController = api:get_player_controller()
        local weapon_mesh = get_equipped_weapon(playerController)
        local current_weapon_fname = nil;
        local weapon_changed = false;

        if weapon_mesh ~= nil then
            if debugMode then print(weapon_mesh:get_full_name()) end
            current_weapon_fname = weapon_mesh.StaticMesh:get_fname():to_string()
            weapon_changed = previous_weapon_fname ~= current_weapon_fname
        else
            current_weapon_fname = unarmed_fname
            weapon_changed = previous_weapon_fname ~= current_weapon_fname;
        end

        if weapon_changed then
            print("Weapon changed")
            if (red_dot_plane_component ~= nil) then
                red_dot_plane_component:K2_DestroyComponent(red_dot_plane_component)
                red_dot_plane_component = nil
            end
            if (scope_plane_component ~= nil) then
                scope_plane_component:K2_DestroyComponent(scope_plane_component)
                scope_plane_component = nil
            end
            print("Previous weapon: " .. (previous_weapon_fname or "none"))
            print("New weapon: " .. current_weapon_fname)

            -- Update current weapon reference
            current_weapon = weapon_mesh

            isSniper = current_weapon_fname == sniper_fname
            if current_weapon_fname == camera_fname or isSniper then
                spawn_scope(engine, weapon_mesh, isSniper)
                attach_components_to_weapon(weapon_mesh, isSniper)
                switch_scope_state(true)
            else
                switch_scope_state(false)
            end
            previous_weapon_fname = current_weapon_fname
        end
        if debugMode then print("check scene_capture_component") end
        if scene_capture_component ~= nil then
            if debugMode then print(scene_capture_component:get_full_name()) end
            local game_camera_manager = UEVR_UObjectHook.get_first_object_by_class(cameraManager_c)
            if debugMode then print("check game_camera_manager") end
            if game_camera_manager ~= nil then
                local actualFov = game_camera_manager.CameraCachePrivate.POV.FOV
                if actualFov >= 70 then
                    actualFov = 70
                end
                if debugMode then print("check if isSniper") end
                if isSniper then
                    scene_capture_component.FOVAngle = mapRange(actualFov, 12, 70, 0.75, 2.6)     --remap fov range for better VR use
                else
                    scene_capture_component.FOVAngle = mapRange(actualFov, 3, 70, 3, 50)          --remap fov range for better VR use
                end
            end
        end
        if debugMode then print("after if scene_capture_component") end
    end
)


uevr.sdk.callbacks.on_script_reset(function()
    if debugMode then print("Resetting") end
    destroy_actor(scope_actor)
    if scope_plane_component ~= nil then
        scope_plane_component:K2_DestroyComponent(scope_plane_component)
    end
    scope_plane_component = nil
    if red_dot_plane_component ~= nil then
        red_dot_plane_component:K2_DestroyComponent(scope_plane_component)
    end
    red_dot_plane_component = nil
    if scene_capture_component ~= nil then
        scene_capture_component:K2_DestroyComponent(scene_capture_component)
    end
    scene_capture_component = nil
    if scene_capture_component_mesh ~= nil then
        scene_capture_component_mesh:K2_DestroyComponent(scene_capture_component_mesh)
    end
    scene_capture_component_mesh = nil
    actual_render_target = nil
    reset_static_objects()
end
)