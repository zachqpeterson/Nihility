# Nihility

This is a game engine made from scratch, winner of Neumont College of Computer Science's capstone invitational 2023. I've made my own data structures, memory allocator, job system, and math library. I try to get as low level as possible by making my own file stream and using Raw Input. This is still an ongoing project that I plan to make games in.

## Future Plan
- [ ] Networking
- [ ] Physics System
- [ ] Particle System
- [ ] Animation System
- [ ] Multithreaded Rendering

## Current 3rd party libraries
assimp - https://github.com/assimp/assimp, used to convert models to Nihility assets

LunarG Vulkan SDK - https://www.lunarg.com/vulkan-sdk/, used for Vulkan binaries, shader compilation, and the Vulkan Memory Allocator

msdf-c - https://github.com/solenum/msdf-c, used to generate Nihlity font assets, rewritten to be optimised for this engine (Resources/Font)

stb - https://github.com/nothings/stb, used to convert some asset types to Nihility assets

wyhash - https://github.com/wangyi-fudan/wyhash, used to generate hashes and random numbers, rewritten to be optimised for this engine (Math/Hash and Math/Random)
