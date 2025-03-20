-- Script made by Mutar for Stalker 2
-- Tweaked by Holydh

local api = uevr.api
local vr = uevr.params.vr

local emissive_material_amplifier = 2.0 
local sniperFov = 10.0
local cameraFov = 70.0
local actualFov = 10.0

-- Static variables
local emissive_mesh_material_name = "Material /Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"
local ftransform_c = nil
local flinearColor_c = nil
local hitresult_c = nil
local game_engine_class = nil
local Statics = nil
local KismetStringLibrary = nil
local KismetMathLibrary = nil
local KismetRenderingLibrary = nil
local KismetMaterialLibrary = nil
local AssetRegistryHelpers = nil
local actor_c = nil
local static_mesh_component_c = nil
local static_mesh_c = nil
local scene_capture_component_c = nil
local MeshC = nil
local StaticMeshC = nil
local CameraManager_c = nil
local Weapon_c = nil


-- Instance variables
local scope_actor = nil
local scope_plane_component = nil
local scene_capture_component = nil
local render_target = nil
local reusable_hit_result = nil
local temp_vec3 = Vector3d.new(0, 0, 0)
local temp_vec3f = Vector3f.new(0, 0, 0)
local zero_color = nil
local zero_transform = nil

local function find_required_object(name)
    local obj = uevr.api:find_uobject(name)
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
    game_engine_class = find_required_object("Class /Script/Engine.GameEngine")
    if not game_engine_class then return false end
    print(game_engine_class:get_full_name())
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
    AssetRegistryHelpers = find_static_class("Class /Script/AssetRegistry.AssetRegistryHelpers")
    if not AssetRegistryHelpers then return false end
    print(AssetRegistryHelpers:get_full_name())
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
    MeshC = api:find_uobject("Class /Script/Engine.SkeletalMeshComponent")
    if not MeshC then return false end
    print(MeshC:get_full_name())
    StaticMeshC = api:find_uobject("Class /Script/Engine.StaticMeshComponent")
    if not StaticMeshC then return false end
    print(StaticMeshC:get_full_name())
    CameraManager_c = find_required_object("Class /Script/Engine.PlayerCameraManager") -- Class Engine.PlayerController/ObjectProperty PlayerCameraManager
    if not CameraManager_c then return false end
    print(CameraManager_c:get_full_name())

    Weapon_c = find_required_object("Class /Script/GTABase.GTAWeapon")
    if not Weapon_c then return false end
    print(Weapon_c:get_full_name())

    -- Initialize reusable objects
    reusable_hit_result = StructObject.new(hitresult_c)
    if not reusable_hit_result then return false end
    --print(reusable_hit_result:get_full_name())

    zero_color = StructObject.new(flinearColor_c)
    if not zero_color then return false end
    --print(zero_color:get_full_name())
    
    zero_transform = StructObject.new(ftransform_c)
    if not zero_transform then return false end
    zero_transform.Rotation.W = 1.0
    zero_transform.Scale3D = temp_vec3:set(1.0, 1.0, 1.0)
    print("init done")
    return true
end

local function reset_static_objects()
    ftransform_c = nil
    flinearColor_c = nil
    hitresult_c = nil
    game_engine_class = nil
    Statics = nil
    KismetRenderingLibrary = nil
    KismetMaterialLibrary = nil
    AssetRegistryHelpers = nil
    actor_c = nil
    static_mesh_component_c = nil
    static_mesh_c = nil
    scene_capture_component_c = nil
    MeshC = nil
    StaticMeshC = nil
    CameraManager_c = nil

    
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

local function get_scope_mesh(parent_mesh)
    if not parent_mesh then return nil end

    local child_components = parent_mesh.AttachChildren
    if not child_components then return nil end

    for _, component in ipairs(child_components) do
        if component:is_a(StaticMeshC) and string.find(component:get_fname():to_string(), "scope") then
            return component
        end
    end

    return nil
end


local function get_equipped_weapon(playerController)
    if not playerController then return nil end
   
    local playerControllerChildren = playerController.Children
    local weapon
    for i, child in ipairs(playerControllerChildren) do
        if child:is_a(Weapon_c) then
            weapon = child
        end
    end
    
    local weapon_mesh = weapon.WeaponMesh
    return weapon_mesh
end

local function get_render_target(world)
    render_target = validate_object(render_target)
    if render_target == nil then
        render_target = KismetRenderingLibrary:CreateRenderTarget2D(world, 1024, 1024, 6, zero_color, false)
    end
    print("Render Target Created " .. render_target:get_full_name())
    return render_target
end

local function spawn_scope_plane(world, owner, pos, rt, isSniper)
    local local_scope_mesh = scope_actor:AddComponentByClass(static_mesh_component_c, false, zero_transform, false)
    if local_scope_mesh == nil then
        print("Failed to spawn scope mesh")
        return
    end

    local wanted_mat = api:find_uobject(emissive_mesh_material_name)
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

    local plane
    if isSniper then
        plane = find_required_object_no_cache(static_mesh_c, "StaticMesh /Engine/BasicShapes/Cylinder.Cylinder")
        print("Cylinder spawning")
    else
        plane = find_required_object_no_cache(static_mesh_c, "StaticMesh /Engine/BasicShapes/Plane.Plane")
        print("Plane spawning")
    end

    if plane == nil then
        print("Failed to find plane mesh")
        return
    end
    --print("plane" .. plane:get_full_name())
    
    
    local_scope_mesh:SetStaticMesh(plane)
    local_scope_mesh:SetVisibility(false)
    -- local_scope_mesh:SetHiddenInGame(false)
    local_scope_mesh:SetCollisionEnabled(0)


    local dynamic_material = local_scope_mesh:CreateAndSetMaterialInstanceDynamicFromMaterial(0, wanted_mat)

    dynamic_material:SetTextureParameterValue(KismetStringLibrary:Conv_StringToName("LinearColor"), rt)
    
    local color = StructObject.new(flinearColor_c)
    color.R = emissive_material_amplifier
    color.G = emissive_material_amplifier
    color.B = emissive_material_amplifier
    color.A = emissive_material_amplifier
    dynamic_material:SetVectorParameterValue(KismetStringLibrary:Conv_StringToName("Color"), color)

    scope_plane_component = local_scope_mesh
    print("Scope plane spawned")
end

-- local function create_emissive_mat(component, materialSocketName)
--     -- local wanted_mat = api:find_uobject(emissive_mesh_material_name)
--     -- if wanted_mat == nil then
--     --     print("Failed to find material")
--     --     return
--     -- end
--     -- wanted_mat.BlendMode = 0
--     -- wanted_mat.TwoSided = 1
--     local index = component:GetMaterialIndex(materialSocketName)
--     -- local dynamic_material = component:CreateDynamicMaterialInstance(index, wanted_mat, "ScopeMaterial")
--     local materials = component:GetMaterials()
--     local materal = materials[index]
--     materal:SetTextureParameterValue("SightMask ", render_target)
--     material.ShadingModel = 0
--     material.BlendMode = 0
--     -- dynamic_material:SetTextureParameterValue("LinearColor", render_target)
-- end

local function spawn_scene_capture_component(world, owner, pos, fov, rt)
    scene_capture_component = scope_actor:AddComponentByClass(scene_capture_component_c, false, zero_transform, false)
    if scene_capture_component == nil then
        print("Failed to spawn scene capture")
        return
    end
    scene_capture_component.TextureTarget = rt
    scene_capture_component:SetVisibility(false)
    scene_capture_component:CaptureScene()
    --print(scene_capture_component:get_full_name())
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
            end
        end
    end

    local rt = get_render_target(world)

    if rt == nil then
        print("Failed to get render target destroying actors")
        rt = nil
        scope_actor = destroy_actor(scope_actor)
        scope_plane_component = nil
        scene_capture_component = nil
        return
    end

    local weaponPos = weaponMesh:K2_GetComponentLocation()
    if not validate_object(scope_actor) then
        scope_actor = destroy_actor(scope_actor)
        scope_plane_component = nil
        scene_capture_component = nil
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
        spawn_scene_capture_component(world, nil, weaponPos, actualFov, rt)
    end

end


local weapon_mesh = nil
local last_scope_state = false

local function attach_components_to_weapon(weapon_mesh, isSniper)
    if not weapon_mesh then return end
    
    local rotation
    if isSniper then
        rotation = KismetMathLibrary:FindLookAtRotation(temp_vec3:set(5.94806 , -2.75068, 13.2024),temp_vec3f:set(30.6871 , -0.22823, 15.6848))
    else
        rotation = KismetMathLibrary:FindLookAtRotation(temp_vec3:set(13.8476, -11.6162, 1.72577),temp_vec3f:set(27.6432, -11.6162, 2.84382))
    end

    print("rotation x = " .. rotation.x .. " rotation y = " .. rotation.y ..  " rotation z = " .. rotation.z)
    -- Attach scene capture to weapon
    if scene_capture_component ~= nil then
            
        -- scene_capture:DetachFromParent(true, false)
        -- "AimSocket"
        print("Attaching scene_capture_component to weapon:" .. weapon_mesh:get_fname():to_string())
        scene_capture_component:K2_AttachToComponent(
            weapon_mesh,
            "gunflash",
            2, -- Location rule
            2, -- Rotation rule
            0, -- Scale rule
            true -- Weld simulated bodies
        )

        if isSniper then
            
            scene_capture_component:K2_SetRelativeRotation(rotation, false, reusable_hit_result, false)
            scene_capture_component:K2_SetRelativeLocation(temp_vec3:set(30.6871 , -0.22823, 15.6848), false, reusable_hit_result, false)
            scene_capture_component:SetVisibility(false)
            actualFov = sniperFov
        else
            local test = temp_vec3:set(rotation.x, rotation.y, rotation.z - 90)
            scene_capture_component:K2_SetRelativeRotation( test, false, reusable_hit_result, false)
            scene_capture_component:K2_SetRelativeLocation(temp_vec3:set(27.6432, -11.6162, 2.84382), false, reusable_hit_result, false)
            scene_capture_component:SetVisibility(false)
            actualFov = cameraFov
        end
        scene_capture_component.FOVAngle = actualFov
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
        --scope_plane_component:K2_SetRelativeRotation(temp_vec3:set(-90, 90, 90), false, reusable_hit_result, false)
        if isSniper then
            local test = temp_vec3:set(rotation.x + 90, rotation.y, rotation.z)
            scope_plane_component:K2_SetRelativeRotation(test, false, reusable_hit_result, false)
            scope_plane_component:K2_SetRelativeLocation(temp_vec3:set(5.94806 , -2.75068, 13.2024), false, reusable_hit_result, false)
            scope_plane_component:SetWorldScale3D(temp_vec3:set(0.032, 0.032, 0.000001))
            --scope_plane_component:SetWorldScale3D(temp_vec3:set(0.1, 0.1, 0.000001))
        else
            local test = temp_vec3:set(rotation.x + 90, rotation.y, rotation.z)
            scope_plane_component:K2_SetRelativeRotation(test, false, reusable_hit_result, false)
            scope_plane_component:K2_SetRelativeLocation(temp_vec3:set(3.8, -13, 1.7), false, reusable_hit_result, false)
            scope_plane_component:SetWorldScale3D(temp_vec3:set(0.06, 0.1, 0))
        end
       
        scope_plane_component:SetVisibility(false)
        print("Scope attached")
    end
end

-- local function on_level_changed(new_level)
--     -- All actors can be assumed to be deleted when the level changes
--     print("Level changed")
--     if new_level then
--         print("New level: " .. new_level:get_full_name())
--     end
--     scope_actor = nil
--     scene_capture = nil
--     plane_component = nil
-- end

local function fix_effects(world)
    if not KismetMaterialLibrary then
        print("KismetMaterialLibrary is nil")
        return
    end
    local game_encamera_manager = UEVR_UObjectHook.get_first_object_by_class(CameraManager_c)
    if not game_encamera_manager then return end
    local fov_mpc = game_encamera_manager.FovMPC
    if not fov_mpc then return end
    local fov_collection = fov_mpc.Collection
    if not fov_collection then return end
    KismetMaterialLibrary:SetScalarParameterValue(world, fov_collection, "IsFOVEnabled", 0.0)
end

local function is_scope_active(pawn)
    if not pawn then return false end
    local optical_scope = pawn.PlayerOpticScopeComponent
    if not optical_scope then return false end
    local scope_active = optical_scope:read_byte(0xA8, 1)
    if scope_active > 0 then
        return true
    end
    return false
end

local function switch_scope_state(state)
    if scene_capture_component ~= nil then
        scene_capture_component:SetVisibility(state)
    end
    if scope_plane_component ~= nil then
        scope_plane_component:SetVisibility(state)
    end
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
                local level = world.PersistentLevel
                if last_level ~= level then
                    print("Level changed .. Reseting")
                    destroy_actor(scope_actor)
                    scope_plane_component = nil
                    scene_capture_component = nil
                    render_target = nil
                    weapon_mesh = nil
                    reset_static_objects()

                    init_static_objects()
                end
                --fix_effects(world)
                last_level = level
            end
        end

        -- reset_scope_actor_if_deleted()
        local playerController = api:get_player_controller()
        -- print(playerController:get_full_name())
        local weapon_mesh = get_equipped_weapon(playerController)
        -- print(weapon_mesh:get_full_name())
        if weapon_mesh then
            -- fix_materials(weapon_mesh)
            local weapon_changed = not current_weapon or weapon_mesh.StaticMesh ~= current_weapon.StaticMesh

            if weapon_changed then
                if (scope_plane_component ~= nil) then
                    scope_plane_component:K2_DestroyComponent(scope_plane_component)
                    scope_plane_component = nil
                end
                print("Weapon changed")
                print("Previous weapon: " .. (current_weapon and current_weapon.StaticMesh:get_fname():to_string() or "none"))
                local currentWeaponString = weapon_mesh.StaticMesh:get_fname():to_string()
                print("New weapon: " .. currentWeaponString)
                
                -- Update current weapon reference
                current_weapon = weapon_mesh

                if currentWeaponString == "SM_camera" or currentWeaponString == "SM_sniper" then
                    local isSniper = currentWeaponString == "SM_sniper"
                    spawn_scope(engine, weapon_mesh, isSniper)
                    attach_components_to_weapon(weapon_mesh, isSniper)
                    switch_scope_state(true)
                else
                    switch_scope_state(false)
                end
                
                -- if (current_weapon.)
                -- Attempt to attach components
              
            end
        else
            -- Weapon was removed/unequipped
            if current_weapon then
                print("Weapon unequipped")
                current_weapon = nil
                weapon_mesh = nil
                --last_scope_state = false
            end
        end
        
    end
)


uevr.sdk.callbacks.on_script_reset(function()
    print("Resetting")
    destroy_actor(scope_actor)
    if scope_plane_component ~= nil then
        scope_plane_component:K2_DestroyComponent(scope_plane_component)
    end
    scope_plane_component = nil
    if scene_capture_component ~= nil then
        scene_capture_component:K2_DestroyComponent(scene_capture_component)
    end
    scene_capture_component = nil
    render_target = nil
    weapon_mesh = nil
    reset_static_objects()
end)

