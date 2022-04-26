# Freya
Freya is a Gameboy emulator

## how complete is it? 
Resoundingly uncomplete. so far it can just about render the copyright screen of Tetris using some flawed approximations of how the gameboy PPU works.
No sound, no input, no interrupts, a lot of the core instructions are broken.

## How do i build it? 
- Install SDL2 (`apt install libsdl2-dev`)
- `mkdir build` 
- `cd build`
- `cmake -B. -H..`
- `./Freya`

## What can I do? 
I won't list all the debugging commands because they're liable to change, but if you hit 
`w` then enter, you can set a breakpoint for 2803, then hit `v` to draw the screen
you should hopefully see a beautiful tetris copyright screen like this.
