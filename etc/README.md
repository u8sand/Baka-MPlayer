## Baka-MPlayer/etc

Here we will store various resources important for the project.

`/doc` - Baka MPlayer documentation

`/font` - Noto Sans font (Baka MPlayer uses this font)

`/logo` - Baka MPlayer logo

`/sbin` - Useful scripts for project
- `mxe.sh` - Build mxe mingw-w64 toolchain for compilation for windows on linux
- `mpv.sh` - Build mpv/libmpv for windows on linux with mxe toolchain
- `bakamplayer.sh` - Build baka mplayer for windows on linux with mxe toolchain

**Note**: The Qt Project File is set to look here (etc/{include,lib}) for the mpv include/library when on windows.
