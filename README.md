# GTA San Andreas Definitive Edition – 6DoF UEVR Plugin

> ⚠️ **Steam version only.** This mod does **not** work with the Epic Games Store version.
 <br>

## ✅ Features
- VR Camera controller handling :
  - All on foot movements including crouching
  - All vehicle driving
  - UEVR integration allowing various camera comfort tweaks
  - Auto handling of UEVR camera settings for cutscenes (all fully rendered in 3D) and flying vehicles

- 6dof weapon handling :
  - Full motion controlled aiming for all ranged weapons even for scripted shooting sequences in vehicles as a passenger
  - Specific recoil animation for every ranged weapons
  - Corrected muzzle flash size for VR
  - Functional Sniper Scope and Camera Screen with zoom in and out function hooked to the game allowing related missions to be completed in VR.
 - Other various game fixes allowing for a full VR playthrough

 <br>
 
## Requirements

- [UEVR Nightly Release (01042)](https://github.com/praydog/UEVR-nightly/releases/tag/nightly-01042-9d6d66496524cdcfa6a022e79b40f1d87669efb4)
- [Plugin build available on Nexus mod](https://github.com/praydog/UEVR-nightly/releases/tag/nightly-01042-9d6d66496524cdcfa6a022e79b40f1d87669efb4)
 <br>
 
## 🛠️ Installation

1. **Launch the game once** (if you haven't already), then **close it**.
2. Copy the included `Gameface` folder into your GTA SA DE root folder.  
   This merges with the existing folder without replacing any files. You can remove the mod at any time.
3. Copy the `GTA San Andreas Definitive Edition` folder into:  
   `C:\Users\YourUserName\Documents\Rockstar Games`  
   > ⚠️ Replace the existing folder and config file. Make a backup if you want to keep your existing ones.
4. Import the UEVR profile:
   - Option 1: Use the **"Import Config"** button in UEVR.
   - Option 2: Manually place the `SanAndreas` folder into:  
     `C:\Users\YourUsername\AppData\Roaming\UnrealVRMod`
5. **Important:** Launch the game and inject UEVR **while in the main menu** (before loading a save file).  
   > This is crucial for proper sniper scope behavior.
6. Adjust graphical settings. Everything is set to the max by default, lower as needed.
7. In the game's **Gameplay Settings**, set **Targeting** to `Free Aim`.
8. Recommended **Gamepad Setting**:  
   Set to `Modern` for easier shooting while driving (left grip to shoot + right stick to rotate the camera changing the shoot direction).
 <br>
 
## 🎮 Usage

The plugin activates **automatically** when UEVR is injected (press both joystick at the same time or "insert" key on keyboard to toggle UEVR menu).  
You can adjust **rotation speed** in :
  `UEVR > Input > Aim Method > Speed`

#### Camera Settings

Default settings prioritize **comfort**.  
**For experienced VR users** looking for a more immersive experience:
  - Go to `UEVR > Camera Settings`
  - **Uncheck** `Pitch Decoupled`  
    → The camera will follow all vehicle rotations with smoothing.
  - Adjust the **Lerp** (smoothing) amount to your liking, or disable it entirely.

I really recommend it if you're not prone to motion sickness. Vehicles are really fun to drive that way.

#### Auto Camera Behavior

The plugin automatically adjusts camera behavior in:

- **Cutscenes**:  
  - `Pitch Decoupled` is enabled to prevent movement when the camera switches.
- **Flying Vehicles**:  
  - `Pitch Decoupled` is disabled and smoothing is off so the camera fully follows rotation.

You can disable this automatic behavior by editing:  
`C:\Users\YourUsername\AppData\Roaming\UnrealVRMod\SanAndreas\UEVR_GTASADE_config.txt`  
Set the relevant values to `false`.
<br>
<br>

## 🚀 Performance

The mod is highly optimized, averaging **0.08 ms per frame** (measured on 14700k), which makes it very lightweight.  

However, the **camera screen** and **sniper scope** can be demanding on your GPU when active.  
You can reduce their performance impact by adjusting their individual render resolutions by opening this file with a text editor :  
`C:\Users\YourUsername\AppData\Roaming\UnrealVRMod\SanAndreas\scripts\GTASADE_Scope.lua`

You can also try using [**DLSS Swapper**](https://github.com/beeradmoore/dlss-swapper) to upgrade to the latest **DLSS version**. In my tests, this noticeably reduced ghosting and improved overall performance.
 <br>
 <br>
 
## ⚠️ Gameplay Notes

- Keep in mind GTA:SA is a **third-person** game :
  - Hold **Aim** before shooting (just like in the base game). If you try to shoot without aiming, you’ll need to **hold the shoot button** and wait for CJ's aiming animation to end before the shoot is allowed.
  - CJ must be **facing your target** for single-handed weapons. You **can’t shoot forward** while walking or running backwards.
- Shooting from vehicle isn't 6DOF. Devs deleted the cheat code for free aiming in cars and I haven't found any way to enable it back. 6DOF aiming is enabled when in vehicle as a passenger during shooting scripted events though.
- Melee weapons and the camera can't be used with motion controls either due to how the game handles them.
 <br>
 <br>
 
## 🎉 Have Fun!

The game should be playable from start to finish with the mod. If you encounter problems, please submit a github issue or report it on the [nexus mode page](https://github.com/praydog/UEVR-nightly/releases/tag/nightly-01042-9d6d66496524cdcfa6a022e79b40f1d87669efb4).
 <br>
 <br>

 ## 🙏 Thanks to
- Praydog for his universal UE VR injector.  
- ⒹⒿ, Pande4360, markmon, Mutar and Ashok for sharing knowledge. 💪  
- Mutar for his Stalker 2 Scope lua script and kind help troubleshooting.  
- teddybear082 and Lord Beardsteak for the playtests and feedbacks.  
 <br>
 <br>
