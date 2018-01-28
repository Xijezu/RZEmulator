# Xijezu's Epic 4 Rappelz Emulator


This project is based on [Project Skyfire](https://github.com/ProjectSkyfire/SkyFire.548), a World of Warcraft emulator. I used this project to get some of the external stuff running, like Database handling, Networking and so on.

  I tried to keep the code as retail as possible (thanks to various matching .pdb's) and used Pyroks unfinished C# Emulator as a reference.
  
  Join us on the [Emulator Discord Server](https://discord.gg/CBGVkdU) for access to a public Testserver, bug reports and more! :) 
  
## Requirements
* Platform: Linux, Windows or Mac
* ACE = 6.1.4 (Windows / Linux)
* MySQL = 5.6.36 (Windows / Linux)
* Lua = 5.2 (Windows / Linux)
* A C++14 Compiler (GCC or MSVC)

## Current Status
This project is far from finished, but it's a huge base so far. The Emulator is pretty stable so far.
* Most of the non-default skills don't work yet (since they require special interactions)
* Riding (mounts) do not work
* Dungeon Sieges do not work
* Guilds
* Monster roaming

## Additional informations
* I'm not really a fan of how the networking works yet (see template'd version of WorldSocket) as well as the GameAuth session, may need a revamp some day
* If you have the possibility, feel free to contribute by reporting bugs or sending in fixes/additions
* No requests - I intend to keep the code base as retail as possible for now
 