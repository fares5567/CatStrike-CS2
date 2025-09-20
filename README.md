# Catstrike External

A simple external project for CSGO written in C++. This is my personal project and it's mainly for learning and experimenting with game hacking stuff. Please don't expect super advanced features or a polished product, but it works for me!

## Features
- External memory reading/writing
- Skinchanger basics
- **Offset/struct database**
  - Auto Updating
- Simple config system
- No UI/Menu (removed for privacy)

<details>
<summary><strong>Show full feature list</strong></summary>


### Aimbot
- Aimbot (psilent, mouse, angle)
- Visual aimbot features: FOV circle, aim line, aim dot
- RCS (Recoil Control System)

### Visuals
- **Player ESP**
  - Box ESP
  - Player Box Glow
  - Rainbow Box Glow
  - Health Based Box Glow
  - Skeleton
  - Health Bar
  - Player Names
  - Weapon Names
  - Distance
  - Off-Screen Arrows
- **World ESP**
  - Smoke ESP
  - Molotov ESP
  - Item Drops
  - Bomb ESP
- **Radar Settings**
  - Enable Radar
  - Show Distance
  - Show Bomb
- **ESP Settings**
  - Glow Intensity
  - Rainbow Speed

### Misc
- **Movement**
  - Local Trail
  - Enemy Trail
- **Spectator**
  - Spectator List
  - Spectator Names
- **Exploits**
  - Night Mode
- **General**
  - Team Check
  - Watermark

</details>

## Preview

<img width="1920" height="1080" alt="grafik" src="https://github.com/user-attachments/assets/c51254d1-b991-44a1-826a-1a4b42fa2083" />

## How to Build
1. Open `catstrike-external.sln` in Visual Studio 2022 or newer.
2. Make sure you have the right Windows SDK and C++ toolset (v143).
3. Build the project (Debug or Release, x64 recommended).
4. The output will be in the `build` folder as `catstrike.exe`.

**Info:** If you build in Debug mode, you'll get a debug version with debug output, no timer, and without unnecessary stuff like in the Release build.

## Notes
- This is for educational purposes only!
- You need basic C++ knowledge to understand or modify the code (I'm still learning myself).
- No ImGui or menu included, only core logic.
- Offsets might need updating for new CSGO versions.

## Credits
- Some code inspired by open-source CSGO projects on GitHub
- Thanks to the C++ community for tutorials and help!
- Auto Update System is used by a user from unknowncheats (really thanks for that, idk the name anymore sry), also the skinload system but haven't used it yet

---

If you have questions or want to contribute, feel free to open an issue or PR, but please keep it friendly and constructive. :)

---

**Note:** This is just a small part of my bigger Catstrike project. There is already software for alt:V, FiveM, Marvel Rivals, Apex, and Fortnite, as well as a website. Maybe I will post more of it in the future!
