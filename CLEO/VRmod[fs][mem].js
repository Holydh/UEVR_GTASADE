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

let footLocalOffsetX = IniFile.ReadFloat(configPath, configSection, 'foot_Local_Offset_X');
let footLocalOffsetY = IniFile.ReadFloat(configPath, configSection, 'foot_Local_Offset_Y');
let footLocalOffsetZ = IniFile.ReadFloat(configPath, configSection, 'foot_Local_Offset_Z');

var carLocalOffsetX = IniFile.ReadFloat(configPath, configSection, 'car_Local_Offset_X');
var carLocalOffsetY = IniFile.ReadFloat(configPath, configSection, 'car_Local_Offset_Y');
var carLocalOffsetZ = IniFile.ReadFloat(configPath, configSection, 'car_Local_Offset_Z');

var bikeLocalOffsetX = IniFile.ReadFloat(configPath, configSection, 'bike_Local_Offset_X');
var bikeLocalOffsetY = IniFile.ReadFloat(configPath, configSection, 'bike_Local_Offset_Y');
var bikeLocalOffsetZ = IniFile.ReadFloat(configPath, configSection, 'bike_Local_Offset_Z');

// var flyingVehicleLocalOffsetX = 0; // No offset along the X-axis
// var flyingVehicleLocalOffsetY = 0; // Slightly behind the player
// var flyingVehicleLocalOffsetZ = 1.0; // Slightly above the player

let actualOffsetX = 0;
let actualOffsetY = 0;
let actualOffsetZ = 0;

var offsetAmount = 0.005;

let actualWeapon = 0;
let heading = 0;

var object1;
var object2;

while (true) {
    wait(0);

    if (!memorySetup && player.isPlaying()) {
        //log(Camera.GetActiveCoordinates());
        // log(char.getCoordinates());

        //nop intructions writing to camera matrix
        // Matrix is standard 4x4 DirectX format
        // Matrix values 0 to 11 = rotation
        // Matrix values 12 to 15 = position
        // nop Camera Matrix value 0
        Memory.Write(0x111DE7E, 7, 0x90, true, true);
        Memory.Write(0x111DE85, 1, 0x90, true, true);

        //nop Camera Matrix value 1
        Memory.Write(0x111DECC, 7, 0x90, true, true);
        Memory.Write(0x111DED3, 1, 0x90, true, true);

        //nop Camera Matrix value 2
        Memory.Write(0x111DED9, 7, 0x90, true, true);
        Memory.Write(0x111DEE0, 1, 0x90, true, true);

        //no instructions writing to Camera Matrix value 3

        //nop Camera Matrix value 4
        Memory.Write(0x111DE5C, 7, 0x90, true, true);
        Memory.Write(0x111DE63, 1, 0x90, true, true);
        Memory.Write(0x111DE64, 1, 0x90, true, true);

        //nop Camera Matrix value 5
        Memory.Write(0x111DE3B, 7, 0x90, true, true);
        Memory.Write(0x111DE42, 1, 0x90, true, true);

        //nop Camera Matrix value 6
        Memory.Write(0x111DE68, 7, 0x90, true, true);
        Memory.Write(0x111DE6F, 1, 0x90, true, true);

        //no instructions writing to Camera Matrix value 7

        //nop Camera Matrix value 8
        Memory.Write(0x111DE75, 7, 0x90, true, true);
        Memory.Write(0x111DE7C, 1, 0x90, true, true);
        Memory.Write(0x111DE7D, 1, 0x90, true, true);

        //nop Camera Matrix value 9
        Memory.Write(0x111DE8F, 7, 0x90, true, true);
        Memory.Write(0x111DE96, 1, 0x90, true, true);
        Memory.Write(0x111DE97, 1, 0x90, true, true);

        //nop Camera Matrix value 10
        Memory.Write(0x111DE98, 7, 0x90, true, true);
        Memory.Write(0x111DE9F, 1, 0x90, true, true);
        Memory.Write(0x111DEA0, 1, 0x90, true, true);

        //no instructions writing to Camera Matrix value 11

        // //nop Camera Matrix 12 value - x Position
        // Memory.Write(0x111DEA5, 7, 0x90, true, true);
        // Memory.Write(0x111DEAC, 1, 0x90, true, true);
        // Memory.Write(0x111DEAD, 1, 0x90, true, true);

        // Memory.Write(0x111DF57, 7, 0x90, true, true);
        // Memory.Write(0x111DF5E, 1, 0x90, true, true);

        // //nop Camera Matrix 13 value - z Position 
        // Memory.Write(0x111DEB3, 7, 0x90, true, true);
        // Memory.Write(0x111DEBA, 1, 0x90, true, true);

        // Memory.Write(0x111DF72, 7, 0x90, true, true);
        // Memory.Write(0x111DF79, 1, 0x90, true, true);

        //nop Camera Matrix 14 value - y Position
        Memory.Write(0x111DEBE, 7, 0x90, true, true);
        Memory.Write(0x111DEC5, 1, 0x90, true, true);
        Memory.Write(0x111DEC6, 1, 0x90, true, true);

        Memory.Write(0x111DF8D, 7, 0x90, true, true);
        Memory.Write(0x111DF94, 1, 0x90, true, true);

        //nothing writes to Camera Matrix 15



        //nop intructions writing to ingame Camera position (source of the line of sight trace for shooting bullet)
        Memory.Write(0x1109F20, 3, 0x90, true, true);
        Memory.Write(0x1109F23, 1, 0x90, true, true);

        Memory.Write(0x1109F96, 3, 0x90, true, true);
        Memory.Write(0x1109F99, 1, 0x90, true, true);

        Memory.Write(0x110A28E, 3, 0x90, true, true);
        Memory.Write(0x110A291, 1, 0x90, true, true);

        Memory.Write(0x11255AB, 3, 0x90, true, true);
        Memory.Write(0x11255AE, 1, 0x90, true, true);

        Memory.Write(0x11070E2, 3, 0x90, true, true);
        Memory.Write(0x11070E5, 1, 0x90, true, true);

        Memory.Write(0x110A3BD, 3, 0x90, true, true);
        Memory.Write(0x110A3C0, 1, 0x90, true, true);

        Memory.Write(0x11080C6, 7, 0x90, true, true); // nop 2 intructions, 2nd starts at 0x11080CA

        Memory.Write(0x1109F24, 3, 0x90, true, true);

        Memory.Write(0x1109FBC, 5, 0x90, true, true);

        Memory.Write(0x110A252, 5, 0x90, true, true);
        Memory.Write(0x110A257, 1, 0x90, true, true);

        Memory.Write(0x110A2C0, 5, 0x90, true, true);

        Memory.Write(0x11255B4, 5, 0x90, true, true);

        Memory.Write(0x11070FF, 5, 0x90, true, true);

        Memory.Write(0x110A3DD, 5, 0x90, true, true);

        Memory.Write(0x1108165, 5, 0x90, true, true);
        Memory.Write(0x110816A, 1, 0x90, true, true);

        Memory.Write(0x1109FA4, 5, 0x90, true, true);

        Memory.Write(0x110A29C, 5, 0x90, true, true);

        Memory.Write(0x11255B3, 5, 0x90, true, true);

        Memory.Write(0x11070F0, 5, 0x90, true, true);

        Memory.Write(0x110A3CB, 5, 0x90, true, true);



        //nop intructions writing to aiming vector (direction of the shoot)
        Memory.Write(0x11090E8, 5, 0x90, true, true);

        Memory.Write(0xAE0410, 5, 0x90, true, true);

        Memory.Write(0x1109EA5, 5, 0x90, true, true);

        Memory.Write(0x1105AAC, 7, 0x90, true, true);
        Memory.Write(0x1105AB3, 1, 0x90, true, true);

        Memory.Write(0x1107E3B, 7, 0x90, true, true);
        Memory.Write(0x1107E42, 1, 0x90, true, true);

        Memory.Write(0x1108E75, 5, 0x90, true, true);

        Memory.Write(0xAE0406, 5, 0x90, true, true);

        Memory.Write(0x11090ED, 3, 0x90, true, true);
        Memory.Write(0x11090F0, 1, 0x90, true, true);

        Memory.Write(0xAE040B, 5, 0x90, true, true);

        Memory.Write(0x1109EAA, 3, 0x90, true, true);
        Memory.Write(0x1109EAD, 1, 0x90, true, true);

        Memory.Write(0x1105AC9, 5, 0x90, true, true);
        Memory.Write(0x1105ACE, 1, 0x90, true, true);

        Memory.Write(0x1107E43, 5, 0x90, true, true);
        Memory.Write(0x1107E48, 1, 0x90, true, true);

        Memory.Write(0x1108E7A, 3, 0x90, true, true);
        Memory.Write(0x1108E7D, 1, 0x90, true, true);


        // log(char.isInAnyCar())
        // log(char.isOnAnyBike())
        // log(char.isInFlyingVehicle())
        memorySetup = true;
    }
    
    let playerCoords = char.getCoordinates();
    actualWeapon = char.getCurrentWeapon().valueOf();

    // let x = Memory.ReadFloat(0x53E2674, true, true);
    // let y = Memory.ReadFloat(0x53E2678, true, true);
    // let z = Memory.ReadFloat(0x53E267C, true, true);

    // Object spawner for debug
    // if (object1 == null && player.isPlaying())
    // {
    //     let modelID = 3027;
    //     Streaming.RequestModel(modelID);
    //     //Streaming.LoadAllModelsNow();

    //     object1 = ScriptObject.Create(modelID, x, y, z)
    //     object2 = ScriptObject.Create(modelID, x + Memory.ReadFloat(0x53E2668, true, true) * 0.5, y + Memory.ReadFloat(0x53E266C, true, true) * 0.5, z + Memory.ReadFloat(0x53E2670, true, true) * 0.5);
    //     log(object1.getModel())
    // }

    // if (object1 != null)
    //     object1.setCoordinates(x, y, z);
     
    // if (object2 != null)
    //     object2.setCoordinates( x + Memory.ReadFloat(0x53E2668, true, true) * 0.25, y + Memory.ReadFloat(0x53E266C, true, true) * 0.25, z + Memory.ReadFloat(0x53E2670, true, true) * 0.25);
    
    //Writing the equipped weapon to memory for UEVR mod use. To apply weapon specific adjustments.
    Memory.WriteI8(0x53DACC6, actualWeapon, true, true);
    
    heading = char.getHeading();
    //Writing the heading to memory for UEVR mod use. To handle car camera.
    Memory.WriteFloat(0x53DACCA, heading, true, true);

    //Writing the crouch status to memory for UEVR mod use. To handle camera adjustements.
    Memory.WriteI8(0x53DACD2, char.isDucking() ? 1 : 0, true, true);

    if (player.isPlaying() && Pad.IsKeyPressed(KeyCode.T)) {
        if (!firstPersonCamInitialized) {
            firstPersonCamEnabled = true;
            IniFile.WriteString(firstPersonCamEnabled ? 'TRUE' : 'FALSE', configPath, configSection, 'FPS_controls_enabled');
        }
        else {
            firstPersonCamEnabled = false;
            IniFile.WriteString(firstPersonCamEnabled ? 'TRUE' : 'FALSE', configPath, configSection, 'FPS_controls_enabled');
        }
        wait(300);
    };

   
    // if (firstPersonCamEnabled)
    // {
    // //     //log(playerCoords);
    // //     //writing camera matrix position to player coordinates.
    // //     // Memory.WriteFloat(0x53E2C30, playerCoords.x + actualOffsetX, true, true);
    // //     // Memory.WriteFloat(0x53E2C34, playerCoords.y + actualOffsetY, true, true);
    //     Memory.WriteFloat(0x53E2C38, Memory.ReadFloat(0x58013E0, true, true) , true, true); 
    // //     //Memory.ReadFloat(0x58013E0, true, true)  + actualOffsetZ
        
    // }


    if (firstPersonCamEnabled && !firstPersonCamInitialized) {
        if (!Camera.GetFadingStatus() && player.isControlOn() && !char.isInFlyingVehicle() && actualWeapon != 35 && actualWeapon != 36)
            toggleFirstPersonControls(true);
    }

    if (!firstPersonCamEnabled && firstPersonCamInitialized) {
        if (!Camera.GetFadingStatus() && player.isControlOn())
            toggleFirstPersonControls(false);
        else {
            wait(50);
        }
    }

    

    if (firstPersonCamEnabled && firstPersonCamInitialized) {
        if (Camera.GetFadingStatus()) {
            firstPersonCamInitialized = false;
        };

        if (!player.isControlOn()) {
            firstPersonCamInitialized = false;
        };

        if (char.isInFlyingVehicle()) {
            firstPersonCamInitialized = false;
            Camera.Restore();
        };

        if (actualWeapon == 35 || actualWeapon == 36){
            firstPersonCamInitialized = false;
        }
            

        if (firstPersonCamEnabled) {
            // Get the player's heading in degrees
            let headingDegrees = char.getHeading();

            // Convert heading to radians
            let headingRadians = headingDegrees * (Math.PI / 180);

            // First-person view offset when on foot
            let localOffsetX; // No offset along the X-axis
            let localOffsetY; // Slightly behind the player
            let localOffsetZ; // Slightly above the player

            if (char.isInAnyCar()) {
                let speed = char.getCarIsUsing().getSpeed() * 0.015;
                
                //send the isInAnyCar state to memory for UEVR mod use
                Memory.WriteI8(0x53DACCE, 1, true, true);
                // if(char.isInFlyingVehicle)
                if (char.isOnAnyBike()) {
                    if (Pad.IsKeyPressed(KeyCode.NumPad1)) {
                        bikeLocalOffsetX -= offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetX, configPath, configSection, 'bike_Local_Offset_X');
                    }
                    if (Pad.IsKeyPressed(KeyCode.NumPad3)) {
                        bikeLocalOffsetX += offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetX, configPath, configSection, 'bike_Local_Offset_X');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad5)) {
                        bikeLocalOffsetY += offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetY, configPath, configSection, 'bike_Local_Offset_Y');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad2)) {
                        bikeLocalOffsetY -= offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetY, configPath, configSection, 'bike_Local_Offset_Y');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad4)) {
                        bikeLocalOffsetZ += offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetZ, configPath, configSection, 'bike_Local_Offset_Z');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad0)) {
                        bikeLocalOffsetZ -= offsetAmount;
                        IniFile.WriteFloat(bikeLocalOffsetZ, configPath, configSection, 'bike_Local_Offset_Z');
                    }


                    // Define the local offsets (relative to the player's orientation)
                    localOffsetX = bikeLocalOffsetX; // No offset along the X-axis
                    localOffsetY = bikeLocalOffsetY + speed; // Slightly behind the player
                    localOffsetZ = bikeLocalOffsetZ; // Slightly above the player
                }
                else {
                    if (Pad.IsKeyPressed(KeyCode.NumPad1)) {
                        carLocalOffsetX -= offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetX, configPath, configSection, 'car_Local_Offset_X');
                    }
                    if (Pad.IsKeyPressed(KeyCode.NumPad3)) {
                        carLocalOffsetX += offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetX, configPath, configSection, 'car_Local_Offset_X');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad5)) {
                        carLocalOffsetY += offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetY, configPath, configSection, 'car_Local_Offset_Y');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad2)) {
                        carLocalOffsetY -= offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetY, configPath, configSection, 'car_Local_Offset_y');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad4)) {
                        carLocalOffsetZ += offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetZ, configPath, configSection, 'car_Local_Offset_Z');
                    }

                    if (Pad.IsKeyPressed(KeyCode.NumPad0)) {
                        carLocalOffsetZ -= offsetAmount;
                        IniFile.WriteFloat(carLocalOffsetZ, configPath, configSection, 'car_Local_Offset_Z');
                    }

                    localOffsetX = carLocalOffsetX; // No offset along the X-axis
                    localOffsetY = carLocalOffsetY + speed; // Slightly behind the player
                    localOffsetZ = carLocalOffsetZ; // Slightly above the player
                }
            }
            else {
                Memory.WriteI8(0x53DACCE, 0, true, true);
                if (Pad.IsKeyPressed(KeyCode.NumPad1)) {
                    footLocalOffsetX -= offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetX, configPath, configSection, 'foot_Local_Offset_X');
                }
                if (Pad.IsKeyPressed(KeyCode.NumPad3)) {
                    footLocalOffsetX += offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetX, configPath, configSection, 'foot_Local_Offset_X');
                }

                if (Pad.IsKeyPressed(KeyCode.NumPad5)) {
                    footLocalOffsetY += offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetY, configPath, configSection, 'foot_Local_Offset_Y');
                }

                if (Pad.IsKeyPressed(KeyCode.NumPad2)) {
                    footLocalOffsetY -= offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetY, configPath, configSection, 'foot_Local_Offset_Y');
                }

                if (Pad.IsKeyPressed(KeyCode.NumPad4)) {
                    footLocalOffsetZ += offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetZ, configPath, configSection, 'foot_Local_Offset_Z');
                }

                if (Pad.IsKeyPressed(KeyCode.NumPad0)) {
                    footLocalOffsetZ -= offsetAmount;
                    IniFile.WriteFloat(footLocalOffsetZ, configPath, configSection, 'foot_Local_Offset_Z');
                }

                // First-person view offset when on foot
                localOffsetX = footLocalOffsetX; // No offset along the X-axis
                localOffsetY = footLocalOffsetY; // Slightly behind the player
                localOffsetZ = footLocalOffsetZ; // Slightly above the player
                // log(char.getHeading());
            }

            // Convert local offsets to world offsets using the heading
            actualOffsetX = localOffsetX * Math.cos(headingRadians) - localOffsetY * Math.sin(headingRadians);
            actualOffsetY = localOffsetX * Math.sin(headingRadians) + localOffsetY * Math.cos(headingRadians);
            actualOffsetZ = localOffsetZ;
            //writing camera position to player coordinates.
            // Memory.WriteFloat(0x53E2674, playerCoords.x + worldOffsetX, true, true);
            // Memory.WriteFloat(0x53E2678, playerCoords.y + worldOffsetY, true, true);
            // Memory.WriteFloat(0x53E267C, playerCoords.z + localOffsetZ, true, true); 


            //Writing camera position to player coordinates.
            // Memory.WriteFloat(0x53E2674, playerCoords.x + worldOffsetX, true, true);
            // Memory.WriteFloat(0x53E2678, playerCoords.y + worldOffsetY, true, true);
            // Memory.WriteFloat(0x53E267C, playerCoords.z + localOffsetZ, true, true);
        }
        if (!firstPersonCamInitialized) {
            toggleFirstPersonControls(false);
        }

    }

    

    //log(char.isOnAnyBike())


    function toggleFirstPersonControls(enable) {
        if (enable) {
            //nop the instructions writing to the camera position

            //VR test
            Camera.Restore();
            wait(10);
            var actualCar;
            if (!char.isOnFoot()) {
                log("fps controls activated");
                actualCar = char.getCarIsUsing()
                char.warpFromCarToCoord(actualCar.getCoordinates().x, actualCar.getCoordinates().y, actualCar.getCoordinates().z + 5)
                //Camera.PointAtChar(char, 53, 1);
                Camera.PointAtChar(char, 53, 1);
                char.warpIntoCar(actualCar)
            }
            else {
                log("fps controls activated");
                Camera.PointAtChar(char, 53, 1);
            }
            //VR test end

            //Memory.Write(0x11080C6, 7, 0x90, true, true);
            firstPersonCamInitialized = true;

        }
        else {
            //VR test
            Camera.Restore();
            //VR test end
            firstPersonCamInitialized = false;
            log("fps controls deactivated");
        }
    }
}

