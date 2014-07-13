## Baka-MPlayer/etc

Here we will store various resources important for the project.

`/logo` - Baka MPlayer Logo  

`/lib` - Library dependencies
> `libmpv.a` - libmpv compiled for x86_64 windows environment with mingw-w64  

`/include` - Include dependencies (library headers)
> `mpv/client.h` - libmpv header file  

`/sbin` - Useful scripts for project
> `PKGBUILD` - Archlinux PKGBUILD script to generate mxe mingw-w64 toolchain with necessary libraries (mostly qt) for cross-compilation

**Note**: The Qt Project File is set to look here (etc/{include,lib}) for the mpv include/library when on windows.
