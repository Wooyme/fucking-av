cmake_minimum_required(VERSION 3.16)
project(untitled1)
set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# cross compilers to use for C, C++ and Fortran
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_Fortran_COMPILER ${TOOLCHAIN_PREFIX}-gfortran)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff
-I${CMAKE_CURRENT_SOURCE_DIR} <SOURCE> <OBJECT>")
ENABLE_LANGUAGE(RC)
#add_compile_options(-fpermissive)
#add_link_options(-static)
add_link_options(-static -Wl,--subsystem,windows)
set(CMAKE_CXX_STANDARD 14)
add_executable(untitled1 main.cpp mjs.c aes.c log.c resource.rc)
#target_link_libraries(untitled1 ${CMAKE_DL_LIBS})
