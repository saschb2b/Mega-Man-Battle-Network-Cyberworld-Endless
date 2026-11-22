# The PS4's cross build for CMake (docker/Dockerfile.ps4: mGBA's, mbedTLS's):
# OpenOrbis's clang for its FreeBSD-like system and its libc; CMake's
# checks link as the game does (lld, the toolchain's script, its libc and
# libkernel), so a function its libc lacks is found missing.
set(CMAKE_SYSTEM_NAME FreeBSD)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(OO $ENV{OO_PS4_TOOLCHAIN})
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER_TARGET x86_64-pc-freebsd12-elf)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-freebsd12-elf)
set(CMAKE_SYSROOT ${OO})
set(CMAKE_C_FLAGS_INIT "-fPIC -funwind-tables -D_GNU_SOURCE -isystem ${OO}/include")
set(CMAKE_AR /usr/bin/ar CACHE FILEPATH "")
set(CMAKE_RANLIB /usr/bin/ranlib CACHE FILEPATH "")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -nostdlib -Wl,-m,elf_x86_64 -Wl,-pie -Wl,--script,${OO}/link.x -Wl,--eh-frame-hdr ${OO}/lib/crt1.o -L${OO}/lib -lc -lkernel")
set(CMAKE_FIND_ROOT_PATH ${OO})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
