## Baka-MPlayer/etc

Here we will store various resources important for the project.

`/doc` - Baka MPlayer documentation

`/logo` - Baka MPlayer logo  

`/lib` - Library dependencies
- `libmpv.{i686,x86_64}.dll` - libmpv compiled for i686 and x86_64 windows environment with mingw-w64  

`/include` - Include dependencies (library headers)
- `mpv/client.h` - libmpv header file  

`/sbin` - Useful scripts for project
- `mxe.sh` - Build mxe mingw-w64 toolchain for compilation for windows on linux
- `mpv.sh` - Build mpv/libmpv for windows on linux with mxe toolchain
- `bakamplayer.sh` - Build baka mplayer for windows on linux with mxe toolchain

**Note**: The Qt Project File is set to look here (etc/{include,lib}) for the mpv include/library when on windows.
