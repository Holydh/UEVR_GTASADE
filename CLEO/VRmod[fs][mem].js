/// <reference path="./.config/sa.d.ts" />
/// <reference path="./.config/unknown.d.ts" />
import { KeyCode } from './.config/enums';

var player = new Player(0);

const configPath = 'CLEO/VRmod.ini';
const configSection = 'CONFIG';

var char = player.getChar();

var firstPersonCamEnabled = true;//IniFile.ReadString(configPath, configSection, 'FPS_controls_enabled') === 'TRUE';
var firstPersonCamInitialized = false;
var memorySetup = false;

let actualWeapon = 0;
let heading = 0;

var object1;
// var object2;

while (true) {
    wait(0);

    // if (!memorySetup && player.isPlaying()) {
    //     //nop intructions writing to camera matrix
    //     // Matrix is standard 4x4 DirectX format
    //     // Matrix values 0 to 11 = rotation
    //     // Matrix values 12 to 15 = position
    //     // nop Camera Matrix value 0
    //     Memory.Write(0x111DE7E, 7, 0x90, true, true);
    //     Memory.Write(0x111DE85, 1, 0x90, true, true);

    //     //nop Camera Matrix value 1
    //     Memory.Write(0x111DECC, 7, 0x90, true, true);
    //     Memory.Write(0x111DED3, 1, 0x90, true, true);

    //     //nop Camera Matrix value 2
    //     Memory.Write(0x111DED9, 7, 0x90, true, true);
    //     Memory.Write(0x111DEE0, 1, 0x90, true, true);

    //     //no instructions writing to Camera Matrix value 3

    //     //nop Camera Matrix value 4
    //     Memory.Write(0x111DE5C, 7, 0x90, true, true);
    //     Memory.Write(0x111DE63, 1, 0x90, true, true);
    //     Memory.Write(0x111DE64, 1, 0x90, true, true);

    //     //nop Camera Matrix value 5
    //     Memory.Write(0x111DE3B, 7, 0x90, true, true);
    //     Memory.Write(0x111DE42, 1, 0x90, true, true);

    //     //nop Camera Matrix value 6
    //     Memory.Write(0x111DE68, 7, 0x90, true, true);
    //     Memory.Write(0x111DE6F, 1, 0x90, true, true);

    //     //no instructions writing to Camera Matrix value 7

    //     //nop Camera Matrix value 8
    //     Memory.Write(0x111DE75, 7, 0x90, true, true);
    //     Memory.Write(0x111DE7C, 1, 0x90, true, true);
    //     Memory.Write(0x111DE7D, 1, 0x90, true, true);

    //     //nop Camera Matrix value 9
    //     Memory.Write(0x111DE8F, 7, 0x90, true, true);
    //     Memory.Write(0x111DE96, 1, 0x90, true, true);
    //     Memory.Write(0x111DE97, 1, 0x90, true, true);

    //     //nop Camera Matrix value 10
    //     Memory.Write(0x111DE98, 7, 0x90, true, true);
    //     Memory.Write(0x111DE9F, 1, 0x90, true, true);
    //     Memory.Write(0x111DEA0, 1, 0x90, true, true);

    //     //no instructions writing to Camera Matrix value 11

    //     //nop Camera Matrix 12 value - x Position
    //     Memory.Write(0x111DEA5, 7, 0x90, true, true);
    //     Memory.Write(0x111DEAC, 1, 0x90, true, true);
    //     Memory.Write(0x111DEAD, 1, 0x90, true, true);

    //     Memory.Write(0x111DF57, 7, 0x90, true, true);
    //     Memory.Write(0x111DF5E, 1, 0x90, true, true);

    //     //nop Camera Matrix 13 value - z Position 
    //     Memory.Write(0x111DEB3, 7, 0x90, true, true);
    //     Memory.Write(0x111DEBA, 1, 0x90, true, true);

    //     Memory.Write(0x111DF72, 7, 0x90, true, true);
    //     Memory.Write(0x111DF79, 1, 0x90, true, true);

    //     //nop Camera Matrix 14 value - y Position
    //     Memory.Write(0x111DEBE, 7, 0x90, true, true);
    //     Memory.Write(0x111DEC5, 1, 0x90, true, true);
    //     Memory.Write(0x111DEC6, 1, 0x90, true, true);

    //     Memory.Write(0x111DF8D, 7, 0x90, true, true);
    //     Memory.Write(0x111DF94, 1, 0x90, true, true);

    //     //nothing writes to Camera Matrix 15



    //     //nop intructions writing to ingame Camera position (source of the line of sight trace for shooting bullet)
    //     Memory.Write(0x1109F20, 3, 0x90, true, true);
    //     Memory.Write(0x1109F23, 1, 0x90, true, true);

    //     Memory.Write(0x1109F96, 3, 0x90, true, true);
    //     Memory.Write(0x1109F99, 1, 0x90, true, true);

    //     Memory.Write(0x110A28E, 3, 0x90, true, true);
    //     Memory.Write(0x110A291, 1, 0x90, true, true);

    //     Memory.Write(0x11255AB, 3, 0x90, true, true);
    //     Memory.Write(0x11255AE, 1, 0x90, true, true);

    //     Memory.Write(0x11070E2, 3, 0x90, true, true);
    //     Memory.Write(0x11070E5, 1, 0x90, true, true);

    //     Memory.Write(0x110A3BD, 3, 0x90, true, true);
    //     Memory.Write(0x110A3C0, 1, 0x90, true, true);

    //     Memory.Write(0x11080C6, 7, 0x90, true, true); // nop 2 intructions, 2nd starts at 0x11080CA

    //     Memory.Write(0x1109F24, 3, 0x90, true, true);

    //     Memory.Write(0x1109FBC, 5, 0x90, true, true);

    //     Memory.Write(0x110A252, 5, 0x90, true, true);
    //     Memory.Write(0x110A257, 1, 0x90, true, true);

    //     Memory.Write(0x110A2C0, 5, 0x90, true, true);

    //     Memory.Write(0x11255B4, 5, 0x90, true, true);

    //     Memory.Write(0x11070FF, 5, 0x90, true, true);

    //     Memory.Write(0x110A3DD, 5, 0x90, true, true);

    //     Memory.Write(0x1108165, 5, 0x90, true, true);
    //     Memory.Write(0x110816A, 1, 0x90, true, true);

    //     Memory.Write(0x1109FA4, 5, 0x90, true, true);

    //     Memory.Write(0x110A29C, 5, 0x90, true, true);

    //     Memory.Write(0x11255B3, 5, 0x90, true, true);

    //     Memory.Write(0x11070F0, 5, 0x90, true, true);

    //     Memory.Write(0x110A3CB, 5, 0x90, true, true);



    //     //nop intructions writing to aiming vector (direction of the shoot)
    //     Memory.Write(0x11090E8, 5, 0x90, true, true);

    //     Memory.Write(0xAE0410, 5, 0x90, true, true);

    //     Memory.Write(0x1109EA5, 5, 0x90, true, true);

    //     Memory.Write(0x1105AAC, 7, 0x90, true, true);
    //     Memory.Write(0x1105AB3, 1, 0x90, true, true);

    //     Memory.Write(0x1107E3B, 7, 0x90, true, true);
    //     Memory.Write(0x1107E42, 1, 0x90, true, true);

    //     Memory.Write(0x1108E75, 5, 0x90, true, true);

    //     Memory.Write(0xAE0406, 5, 0x90, true, true);

    //     Memory.Write(0x11090ED, 3, 0x90, true, true);
    //     Memory.Write(0x11090F0, 1, 0x90, true, true);

    //     Memory.Write(0xAE040B, 5, 0x90, true, true);

    //     Memory.Write(0x1109EAA, 3, 0x90, true, true);
    //     Memory.Write(0x1109EAD, 1, 0x90, true, true);

    //     Memory.Write(0x1105AC9, 5, 0x90, true, true);
    //     Memory.Write(0x1105ACE, 1, 0x90, true, true);

    //     Memory.Write(0x1107E43, 5, 0x90, true, true);
    //     Memory.Write(0x1107E48, 1, 0x90, true, true);

    //     Memory.Write(0x1108E7A, 3, 0x90, true, true);
    //     Memory.Write(0x1108E7D, 1, 0x90, true, true);

    //     // nop rocket launcher specific instruction writing to aiming vector :
    //     Memory.Write(0x110E71D, 5, 0x90, true, true);
    //     Memory.Write(0x110E722, 1, 0x90, true, true);

    //     Memory.Write(0x110E70B, 7, 0x90, true, true);
    //     Memory.Write(0x110E712, 1, 0x90, true, true);

    //     // nop sniper specific instruction writing to aiming vector :
    //     Memory.Write(0x110E19E, 5, 0x90, true, true);
    //     Memory.Write(0x110E1A3, 1, 0x90, true, true);

    //     Memory.Write(0x110E196, 7, 0x90, true, true);
    //     Memory.Write(0x110E19D, 1, 0x90, true, true);

    //     // nop car specific instruction writing to aiming vector :
    //     Memory.Write(0x110BB78, 3, 0x90, true, true);
    //     Memory.Write(0x110BB7B, 1, 0x90, true, true);

    //     Memory.Write(0x110C5A4, 3, 0x90, true, true);
    //     Memory.Write(0x110C5A7, 1, 0x90, true, true);

    //     Memory.Write(0x110C59E, 5, 0x90, true, true);
    //     Memory.Write(0x110C5A3, 1, 0x90, true, true);

    //     Memory.Write(0x110BB68, 5, 0x90, true, true);
    //     Memory.Write(0x110BB6D, 1, 0x90, true, true);

    //     // log(char.isInAnyCar())
    //     // log(char.isOnAnyBike())
    //     // log(char.isInFlyingVehicle())
    //     memorySetup = true;
    // }
    
    actualWeapon = char.getCurrentWeapon().valueOf();
    //char.setHeading(180);
    //Camera.Restore()
    
    // let playerPosition = char.getCoordinates();
    // let camForwardX = playerPosition.x + Memory.ReadFloat(0x53E2668, true, true) * 10;
    // let camForwardY = playerPosition.y + Memory.ReadFloat(0x53E266C, true, true) * 10;
    // let camForwardZ = playerPosition.z + Memory.ReadFloat(0x53E2670, true, true) * 10;

    //Camera.PointAtChar(char, 1, 1);
    //Camera.PointAtPoint(camForwardX, camForwardY, camForwardZ, 1);
    //Camera.AttachToVehicle(char.getCarIsUsing(), 0, 0, 0, 0, 0, 0, 0, 1);
    
    //Camera.Restore()
    //Object spawn debug
    // let x = Memory.ReadFloat(0x53E2674, true, true);
    // let y = Memory.ReadFloat(0x53E2678, true, true);
    // let z = Memory.ReadFloat(0x53E267C, true, true);

    // if (object1 == null && player.isPlaying())
    // {
    //     let modelID = 8168;
    //     Streaming.RequestModel(modelID);
    //     //Streaming.LoadAllModelsNow();

    //     object1 = ScriptObject.Create(modelID, camForwardX, camForwardY, camForwardZ)
    //     // object2 = ScriptObject.Create(modelID, x + Memory.ReadFloat(0x53E2668, true, true) * 0.5, y + Memory.ReadFloat(0x53E266C, true, true) * 0.5, z + Memory.ReadFloat(0x53E2670, true, true) * 0.5);
    //     log(object1.getModel())
    // }

    // if (object1 != null)
    //     object1.setCoordinates(camForwardX, camForwardY, camForwardZ);
     
    // if (object2 != null)
    //     object2.setCoordinates( x + Memory.ReadFloat(0x53E2668, true, true) * 0.25, y + Memory.ReadFloat(0x53E266C, true, true) * 0.25, z + Memory.ReadFloat(0x53E2670, true, true) * 0.25);
    
    //Writing the firstPersonCam state to reset the camera heading after cinematics.
    Memory.WriteI8(0x53DACC6, firstPersonCamInitialized ? 1 : 0, true, true);

    //Writing the equipped weapon to memory for UEVR mod use. To apply weapon specific adjustments.
    Memory.WriteI8(0x53DACC7, actualWeapon, true, true);

    //Writing the crouch status to memory for UEVR mod use. To handle camera adjustements.
    Memory.WriteI8(0x53DAD11, char.isDucking() ? 1 : 0, true, true);

    if (player.isPlaying() && Pad.IsKeyPressed(KeyCode.T)) {
        Camera.SetLerpFov(99, 1, 1, true);
        Camera.PersistFov(true);
    }

    // Memory 0x53DACDA written by UEVR dll, "currentDuckOffset"

    Memory.WriteI8(0x53DACCE, char.isInAnyCar() ? 1 : 0, true, true);
    // if (char.isInAnyCar())
    //     Camera.SetBehindPlayer();

    Memory.WriteI8(0x53DAD01, char.isGettingInToACar()? 1 : 0, true, true);
    
    Memory.WriteI8(0x53DACE1, char.isShooting() ? 1 : 0, true, true);

    heading = char.getHeading();
    //Writing the heading to memory for UEVR mod use. To handle car camera.
    Memory.WriteFloat(0x53DACF1, heading, true, true);



    // if (player.isPlaying() && Pad.IsKeyPressed(KeyCode.T)) {

    if (firstPersonCamEnabled && !firstPersonCamInitialized) {
        if (!Camera.GetFadingStatus() && player.isControlOn() && !char.isInFlyingVehicle()) // && actualWeapon != 35 && actualWeapon != 36
            toggleFirstPersonControls(true);
    }

    if (!firstPersonCamEnabled && firstPersonCamInitialized) {
        if (!Camera.GetFadingStatus() && player.isControlOn())
            toggleFirstPersonControls(false);
        // else {
        //     //wait(50);
        // }
    }

    

    if (firstPersonCamEnabled && firstPersonCamInitialized) {
        if (Camera.GetFadingStatus()) {
            firstPersonCamInitialized = false;
        };

        if (!player.isControlOn()) {
            firstPersonCamInitialized = false;
        };

        // if (char.isInFlyingVehicle()) {
        //     firstPersonCamInitialized = false;
        //     Camera.Restore();
        // };            


        if (!firstPersonCamInitialized) {
            toggleFirstPersonControls(false);
        }

    }

    function toggleFirstPersonControls(enable) {
        if (enable) {
            firstPersonCamInitialized = true;
        }
        else {
            firstPersonCamInitialized = false;
            log("fps controls deactivated");
        }
    }
}

