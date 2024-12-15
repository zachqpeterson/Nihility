# Nihility

This is a game engine made from scratch, winner of Neumont College of Computer Science's Capstone Invitational 2023. I've made my own data structures, memory allocator, job system, and math library. I try to get as low level as possible by making my own file stream and using Raw Input. This is still an ongoing project that I plan to make games in.

## Current Features
### Compile-Time Math Library
My math library includes general use math, vectors, matrices, quaternions, and splines, all available at compile-time because why not.
### Fast Memory Allocator
My memory allocator is both thread-safe (without locks/mutexes) and much faster than malloc, it has chunk/region-styled allocations and linear allocations.
### Custom String Type
My string implementation supports easy formating and can convert any type into a string, you can also define string conversion operators for any class which will be used by the string parser.

## Future Plans
- [ ] Networking
- [ ] Physics System
- [ ] Particle System
- [ ] Animation System
- [ ] Multithreaded Rendering
- [ ] Project Creation/Management System

And more...

## Setup
- install the Vulkan SDK: https://vulkan.lunarg.com/sdk/home, leaving all setting default is sufficient
- Clone the repository
- Open the solution in Visual Studio and set the startup project to any of the demo projects

## Current 3rd party libraries
assimp - https://github.com/assimp/assimp, used to convert models to Nihility assets

LunarG Vulkan SDK - https://www.lunarg.com/vulkan-sdk/, used for Vulkan binaries, shader compilation, and the Vulkan Memory Allocator

msdf-c - https://github.com/solenum/msdf-c, used to generate Nihlity font assets, rewritten to be optimised for this engine (Resources/Font)

stb - https://github.com/nothings/stb, used to convert some asset types to Nihility assets

wyhash - https://github.com/wangyi-fudan/wyhash, used to generate hashes and random numbers, rewritten to be optimised for this engine (Math/Hash and Math/Random)
