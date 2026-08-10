set(CMAKE_SYSTEM_NAME AmigaOS)
set(CMAKE_SYSTEM_PROCESSOR m68k)
set(CMAKE_EXECUTABLE_SUFFIX "")

set(
    OPENVN_AMIGA_TARGET_FLAGS
    "-m68000 -msoft-float -noixemul"
    CACHE STRING
    "Compiler/linker flags for the classic Amiga target ABI"
)

if(DEFINED ENV{OPENVN_AMIGA_GCC} AND NOT "$ENV{OPENVN_AMIGA_GCC}" STREQUAL "")
    set(CMAKE_C_COMPILER "$ENV{OPENVN_AMIGA_GCC}" CACHE FILEPATH "Amiga GCC" FORCE)
else()
    find_program(OPENVN_AMIGA_GCC_EXECUTABLE NAMES m68k-amigaos-gcc REQUIRED)
    set(CMAKE_C_COMPILER "${OPENVN_AMIGA_GCC_EXECUTABLE}" CACHE FILEPATH "Amiga GCC" FORCE)
endif()

if(DEFINED ENV{OPENVN_AMIGA_SDK} AND NOT "$ENV{OPENVN_AMIGA_SDK}" STREQUAL "")
    set(OPENVN_AMIGA_SDK "$ENV{OPENVN_AMIGA_SDK}" CACHE PATH "Amiga SDK/NDK root")
endif()

if(OPENVN_AMIGA_SDK)
    if(EXISTS "${OPENVN_AMIGA_SDK}/Include_H/proto/exec.h")
        set(OPENVN_AMIGA_INCLUDE_DIR "${OPENVN_AMIGA_SDK}/Include_H")
    elseif(EXISTS "${OPENVN_AMIGA_SDK}/include/proto/exec.h")
        set(OPENVN_AMIGA_INCLUDE_DIR "${OPENVN_AMIGA_SDK}/include")
    elseif(EXISTS "${OPENVN_AMIGA_SDK}/m68k-amigaos/ndk-include/proto/exec.h")
        set(OPENVN_AMIGA_INCLUDE_DIR "${OPENVN_AMIGA_SDK}/m68k-amigaos/ndk-include")
    else()
        message(FATAL_ERROR "OPENVN_AMIGA_SDK has no supported Amiga include layout")
    endif()

    if(EXISTS "${OPENVN_AMIGA_SDK}/lib/amiga.lib")
        set(OPENVN_AMIGA_LIBRARY_DIR "${OPENVN_AMIGA_SDK}/lib")
    elseif(EXISTS "${OPENVN_AMIGA_SDK}/lib/libamiga.a")
        set(OPENVN_AMIGA_LIBRARY_DIR "${OPENVN_AMIGA_SDK}/lib")
    elseif(EXISTS "${OPENVN_AMIGA_SDK}/m68k-amigaos/lib/libamiga.a")
        set(OPENVN_AMIGA_LIBRARY_DIR "${OPENVN_AMIGA_SDK}/m68k-amigaos/lib")
    else()
        message(FATAL_ERROR "OPENVN_AMIGA_SDK has neither amiga.lib nor libamiga.a")
    endif()

    string(APPEND CMAKE_C_FLAGS_INIT " -I${OPENVN_AMIGA_INCLUDE_DIR}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -L${OPENVN_AMIGA_LIBRARY_DIR}")
endif()

string(APPEND CMAKE_C_FLAGS_INIT " -D__AMIGA__")
string(APPEND CMAKE_C_FLAGS_INIT " ${OPENVN_AMIGA_TARGET_FLAGS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${OPENVN_AMIGA_TARGET_FLAGS}")
