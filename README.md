# GTA San Andreas Definitive Edition – 6DoF UEVR Plugin

> ⚠️ **Steam version only.** This mod does **not** work with the Epic Games Store version.
 <br>

## ✨ Features

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
5. Launch the game.
6. **Important:** Inject UEVR **while in the main menu** (before loading a save file).  
   > This is crucial for proper sniper scope behavior.
7. In the game's **Gameplay Settings**, set **Targeting** to `Free Aim`.
8. Adjust graphical settings as needed:
   - Disable **Ambient Occlusion** and **World Shadows** — they only render in one eye.
9. Recommended **Gamepad Setting**:  
   Set to `Modern` for easier shooting while driving (left grip to shoot + right stick to rotate the camera changing the shoot direction).
 <br>
 
## 🎮 Usage

- The plugin activates **automatically** when UEVR is injected (press both joystick at the same time or "insert" key on keyboard to toggle UEVR menu).
- You can adjust **rotation speed** in :
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
 <br>
 <br>
 
## 🎉 Have Fun!

- The game should be playable from start to finish with the mod. If you encounter problems, please submit a github issue or report it on the [nexus mode page](https://github.com/praydog/UEVR-nightly/releases/tag/nightly-01042-9d6d66496524cdcfa6a022e79b40f1d87669efb4).
 <br>
 <br>
 
Changelog V1.3
- Auto handling of UEVR camera settings during cutscenes and in flying vehicles
- Fix camera position not updating correctly under specific situations
- Bullet traces attached to CJ animations are now hidden
- Code & git cleanup for release

Changelog V1.2
- Fixed aggressive velocity of spawning of cars
- Added another shoot detection method for recoil animations. Should fix all recoil issues
- Rocket launcher aim fixed
- Sniper scope and camera screen hooked into the original zoom function (you can now zoom in and out like usual)
- Camera Screen now usable for missions requiring to take photos (although not in full 6dof)

Changelog
V1.16
- Camera weapon controls added
- Camera weapon screen added
- Sniper aim fixed
- Sniper scope added (Credits to Mutar for the initial script)
- Attempt to fix bugged spawns of cars
- All crosshairs now hidden via created texture mods

V1.151
- Better weapon mesh visibility handling. Night googles visibility not always toggled during cutscene fixed.
- Thermal googles meshes hidden when not in cutscene
- (for devs) Code cleaned up (wip) and completely restructured for clarity. I'll set the git to public soon.

V1.15
- Blurry underwater view fixed
- Melee weapons unhooked from motion controllers for better usability (true VR melee won't be possible without better modding capabilities for this game)
- All tattoos and some more clothes hidden when not in cutscene
- Night Google mesh hidden

V1.14
- Fixed wrong shoot detection triggering the recoil when character ends reloading.
- Aim from vehicle fixed. You can now use the right joystick to rotate the camera toward the direction you want to shoot (left, front, right)
- Fixed a rare occurrence of the weapon not correctly detaching from motion controller when entering a vehicle

V1.12
- More reliable shoot and crouch detection (for recoil and crouch animation)
- Improved VR crouch. It now follows CJ's head height
- Weapon mesh position & rotation now reset to their original position during cutscenes and when driving, no more floating weapons.

V1.1
- Better Memory Management. Mod can now be enabled/disabled at runtime without crashing.
- All spray weapons aiming fixed (spray can, fire extinguisher, flame thrower)
- Rotation speed hooked to the UEVR settings. You can modify it anytime by moving the slide in UEVR > Input > Aim Method > Speed
- no more Cleo dependency

Initial Release V1.0 :
- Homemade Camera controller added
- Camera following vehicle rotation added
- Cutscene camera handling added
- 6dof weapon handling added
- VR crouch handler added
- Weapon visibility fixed
- Weapon recoil added
- Texture mod to remove all the crosshair added
- Texture mod to reduce the size of the blinding vanilla muzzle flash added
