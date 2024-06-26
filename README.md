## Just enough Doom to run timedemo 3
![Doomtd3](readme_imgs/doomtd3.png?raw=true)

Does your computer have 300 kB free contiguous memory?
And is there a C compiler for it?

Then you can port Doom to it.
Well, enough parts of Doom to run timedemo 3, in black and white.

32-bit CPU, 16-bit CPU.
Little-endian, big-endian.
It's all possible.
No FPU required.

## How to add other platforms
Implement `i_system.h`. Look at `i_ibm.c` and `i_mac.c` for examples.

|Platform         |Platform specific code |Compiler                                                                   |Set environment variables|Compile code    |Additional information                              |
|-----------------|-----------------------|---------------------------------------------------------------------------|-------------------------|----------------|----------------------------------------------------|
|Amiga 500        |`i_amiga.c`            |[amiga-gcc](https://github.com/bebbo/amiga-gcc)                            |n/a                      |`compa500.sh`   |Experimental, might not work on a real machine      |
|Amiga 500        |`i_amiga.c`,`i_amiga_chunky.c` |[vbcc](http://sun.hasenbraten.de/vbcc/)                            |n/a                      |`compvbcc.bat`  |68000 compile doesn't work. 68020 seems to be fine    |
|HP 95LX          |`i_hp95lx.c`, `i_ibma.asm`|[gcc-ia16](https://github.com/tkchia/gcc-ia16), [NASM](https://www.nasm.us)|n/a                      |`comphp95.sh`   |No status bar, also runs on HP 100LX and HP 200LX   |
|IBM PC 16-bit    |`i_ibm.c`,    `i_ibma.asm`|[gcc-ia16](https://github.com/tkchia/gcc-ia16), [NASM](https://www.nasm.us)|n/a                      |`compia16.sh`   |Use command line argument `lcd` to invert the colors|
|IBM PC 16-bit[^1]|`i_ibm.c`                 |[Watcom](https://github.com/open-watcom/open-watcom-v2)                    |`setenvwc.bat`           |`compwc16.sh`   |Use command line argument `lcd` to invert the colors|
|IBM PC 32-bit    |`i_ibm.c`                 |[DJGPP](https://github.com/andrewwutw/build-djgpp)                         |`setenvdj.bat`           |`compdj.bat`    |Use command line argument `lcd` to invert the colors|
|Macintosh Plus   |`i_mac.c`                 |[Retro68](https://github.com/autc04/Retro68)                               |n/a                      |`CMakeLists.txt`|Experimental, might not work on a real machine      |

[^1]: Two compilers can build the IBM PC 16-bit port. Gcc-ia16 produces faster code than Watcom. The static code analysers of both compilers detect different issues.

## Chunky Copper for Amiga

I've forked this (and added VBCC compiler support) to experiment with chunky copper methods. Currently I've added a very dumb "Zero bitplanes, hammer colour 0 as fast as the copper will allow" method, leading to 8x1 pixels. This would work with all of OCS/ECS/AGA, but I've been unable to get VBCC to produce a working 68000 executable (in fact, the compiler flat-out aborts for a 000 compile at higher optimization settings. 020 seems to work fine though.)

Stock A1200 in Winuae gets about 8-9 FPS. 

TODO:
- Re-add status panel
- Add 8 by Y max performance mode (need to read up on copper skips and loops)
- Add 4 by 4 mode

![thicc pixels](readme_imgs/chunky0.png?raw=true)


Swiz
