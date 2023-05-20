# NihilityEngine

This is game engine made from scratch, winner of my college's capstone invitational. I've made my own data structures, memory allocator, and math library. I try to get as low level as possible by making my own file stream, using Raw Input, and job system. This engine is currently going through a major 
refactor (Refactor branch) to make it more easily expandible.

## Current 3rd party libraries
wyhash/wyrand - https://github.com/wangyi-fudan/wyhash, rewritten to be optimised for this engine

json - https://github.com/nlohmann/json, likely to be replaced by my own parser

VulkanMemoryAllocator - https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator, likely to be replaced by my memory allocator

tracy - https://github.com/wolfpld/tracy, not currently in use, may be removed
