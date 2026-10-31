# The PS5's cross build for CMake (docker/Dockerfile.ps5: mGBA's): the native-app
# template's compiler (clang 18 for x86_64-sie-ps5, the payload SDK's headers),
# through /usr/local/bin/ps5-cc and ps5-cxx. Its checks compile without linking, as an app
# links only through the template's own script; what they wrongly find is
# preset off where it matters (docker/mgba.sh).
set(CMAKE_SYSTEM_NAME FreeBSD)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER /usr/local/bin/ps5-cc)
set(CMAKE_CXX_COMPILER /usr/local/bin/ps5-cxx)
set(CMAKE_AR /usr/bin/llvm-ar-18 CACHE FILEPATH "")
set(CMAKE_RANLIB /usr/bin/llvm-ranlib-18 CACHE FILEPATH "")
set(CMAKE_C_FLAGS_INIT "-ffunction-sections -fdata-sections")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH /opt/ps5app/.deps/native/ps5-payload-sdk/target)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
