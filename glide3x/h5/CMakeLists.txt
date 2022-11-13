CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Glide3x VERSION 1.0 DESCRIPTION "Glide3x API for Voodoo3/4/5" LANGUAGES C ASM_NASM)

OPTION(BUILD_32BIT "Build for 32-bit targets, otherwise x64" OFF)
OPTION(USE_DRI "no idea what DRI mode stands for" OFF)
OPTION(USE_DGA "no idea what DRI mode stands for" OFF)
OPTION(EMBED_TEXUS2 "Embed Texus2 functions into Glide3" ON)

# Set the configuration for the assembler
if (${BUILD_32BIT})
    if(MSVC)
        # Nothing to do - MSVC supports building 32bit using a different cmake invokation
    else()
        # Set this flag for the compiler and linker
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m32")
    endif()
        
    # tell the assembler to generate 32 bit files
    if (WIN32)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT win32)
    else()
        set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
    endif()
else()
    if (WIN32)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    else()
        set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    endif()
endif()

if (WIN32)
    set(CMAKE_ASM_NASM_DEFINITIONS ${CMAKE_ASM_NASM_DEFINITIONS}
        -D_NASM_BUILD_FOR_WIN32
    )
else()
    set(CMAKE_ASM_NASM_DEFINITIONS ${CMAKE_ASM_NASM_DEFINITIONS}
        -D_NASM_BUILD_FOR_LINUX
    )
endif()


set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
    <FLAGS> -DGLIDE_PACKED_RGB ${CMAKE_ASM_NASM_DEFINITIONS} -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
    

set (NEWPCI_LIB_DIR ${CMAKE_SOURCE_DIR}/../../swlibs/newpci/pcilib)
set (FXMISC_DIR ${CMAKE_SOURCE_DIR}/../../swlibs/fxmisc)
set (CINIT_DIR ${CMAKE_SOURCE_DIR}/cinit)

set (MINIHWC_DIR ${CMAKE_SOURCE_DIR}/minihwc)
set (TEXUS2_DIR ${CMAKE_SOURCE_DIR}/../../swlibs/texus2/lib)

set(GLIDE_SOURCES 
    ${CMAKE_SOURCE_DIR}/glide3/src/fifo.c
    ${CMAKE_SOURCE_DIR}/glide3/src/distate.c
    ${CMAKE_SOURCE_DIR}/glide3/src/gstrip.c
    ${CMAKE_SOURCE_DIR}/glide3/src/distrip.c
    ${CMAKE_SOURCE_DIR}/glide3/src/diget.c
    ${CMAKE_SOURCE_DIR}/glide3/src/gsplash.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/g3df.c  
    ${CMAKE_SOURCE_DIR}/glide3/src/gu.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gpci.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/diglide.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/disst.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/ditex.c 

    ${CMAKE_SOURCE_DIR}/glide3/src/gbanner.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gerror.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gaa.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gdraw.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gglide.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/glfb.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gsst.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gtex.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/gtexdl.c 
    #${CMAKE_SOURCE_DIR}/glide3/src/gutex.c 
    ${CMAKE_SOURCE_DIR}/glide3/src/xtexdl_def.c
    #${CMAKE_SOURCE_DIR}/glide3/src/cpudtect_stubs.c
    ${CMAKE_SOURCE_DIR}/glide3/src/gthread.c
)



set(GLIDE_SOURCES ${GLIDE_SOURCES} 
    # We always use the C routines for drawing triangles, never the asm or 3dnow ones
    ${CMAKE_SOURCE_DIR}/glide3/src/gxdraw.c
)

# platform specific compile files
if(WIN32)
    # Windows build
    set(GLIDE_SOURCES ${GLIDE_SOURCES}
        ${CMAKE_SOURCE_DIR}/glide3/src/gsfc.c
        ${CMAKE_SOURCE_DIR}/glide3/src/winsurf.c
    )
        
    set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
        ${MINIHWC_DIR}/hwcio.c
        ${MINIHWC_DIR}/gdebug.c
        ${MINIHWC_DIR}/minihwc.c
        ${MINIHWC_DIR}/win_mode.c
    )
    if (${BUILD_32BIT})
        # This code is anyway only relevant for Win9x, and can be excluded for x64 builds.
        set (GLIDE_ASM_SOURCES ${GLIDE_ASM_SOURCES}
            ${MINIHWC_DIR}/win9x.asm
        )
    endif()
else()
    # Linux build
    set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
        ${NEWPCI_LIB_DIR}/fxpci.c
        ${NEWPCI_LIB_DIR}/fxlinux.c
        
        ${MINIHWC_DIR}/hwcio.c
        ${MINIHWC_DIR}/gdebug.c
        
        ${FXMISC_DIR}/fxos.c
        ${FXMISC_DIR}/fximg.c
    )
    

    if (${USE_DRI})
        set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
            ${FXMISC_DIR}/linhwc.c
            ${FXMISC_DIR}/linutil.c
        )
    else()
        set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
            ${MINIHWC_DIR}/minihwc.c
            ${MINIHWC_DIR}/lin_mode.c
            ${MINIHWC_DIR}/gpio.c
            ${CINIT_DIR}/h3cinit.c
        )
    endif()
endif()

if (${EMBED_TEXUS2})
    set (TEXUS_SOURCES
        ${TEXUS2_DIR}/texuslib.c
        ${TEXUS2_DIR}/clamp.c
        ${TEXUS2_DIR}/read.c
        ${TEXUS2_DIR}/resample.c
        ${TEXUS2_DIR}/mipmap.c
        ${TEXUS2_DIR}/quantize.c
        ${TEXUS2_DIR}/ncc.c
        ${TEXUS2_DIR}/nccnnet.c
        ${TEXUS2_DIR}/pal256.c
        ${TEXUS2_DIR}/pal6666.c
        ${TEXUS2_DIR}/dequant.c
        ${TEXUS2_DIR}/view.c
        ${TEXUS2_DIR}/util.c
        ${TEXUS2_DIR}/diffuse.c
        ${TEXUS2_DIR}/write.c
        ${TEXUS2_DIR}/tga.c
        ${TEXUS2_DIR}/3df.c
        ${TEXUS2_DIR}/ppm.c
        ${TEXUS2_DIR}/rgt.c
        ${TEXUS2_DIR}/txs.c
        ${TEXUS2_DIR}/codec.c
        ${TEXUS2_DIR}/eigen.c
        ${TEXUS2_DIR}/bitcoder.c
    )
endif()

# Create a shared library 
add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${GLIDE_SOURCES}
    ${GLIDE_OUTSIDE_SOURCES}
    ${GLIDE_ASM_SOURCES}
    ${TEXUS_SOURCES}
    )

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/glide3/src
    ${CMAKE_SOURCE_DIR}/incsrc
    ${CMAKE_SOURCE_DIR}/init
    ${CMAKE_SOURCE_DIR}/minihwc
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmisc
    ${CMAKE_SOURCE_DIR}/../../swlibs/newpci/pcilib
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmemmap
    ${CINIT_DIR}
)

#if(MSVC)
#  # disable all warnings for now (too many)
#  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W0)
#else()
#  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
#endif()


target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    # general
    -DGLIDE3 
    -DGLIDE3_ALPHA 
    -DGLIDE_HW_TRI_SETUP=1 
    -DGLIDE_INIT_HWC 
    -DGLIDE_PACKED_RGB=0 
    -DGLIDE_PACKET3_TRI_SETUP=1 
    -DGLIDE_TRI_CULLING=1 
    -DUSE_PACKET_FIFO=1
    -DGLIDE_CHECK_CONTEXT

    # subsystem
    -DH3
    -DH4
    -DFX_GLIDE_H5_CSIM=1
    -DFX_GLIDE_NAPALM=1
    
    # other 
    -DGLIDE_PLUG
    -DGLIDE_SPLASH

    # Use MMX
    #-DGL_MMX
    #-DGL_SSE
    #-DGL_SSE2
    -DGL_X86

    # Do not use the ASM routines, but the C triangle setup routines instead
    -DGLIDE_USE_C_TRISETUP
)


if (${BUILD_32BIT})

else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGLIDE_BUILD_64BIT
    )

endif()


if(WIN32)
    # Windows-only defines
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -D__WIN32__
        -DFX_DLL_ENABLE
        -DHWC_ACCESS_DDRAW=1
        -DHWC_EXT_INIT=1
        -DGLIDE_ALT_TAB=1

        -DBETA=1 
        -DHWC_MINIVDD_HACK=1 
        -DWIN40COMPAT=1 
        -DWINXP_ALT_TAB_FIX=1 
        -DWINXP_SAFER_ALT_TAB_FIX=1 
        -DNEED_MSGFILE_ASSIGN
    )
    
    # Windows build uses DirectDraw to detect possible screen resolutions and prepare
    # the context, therefore we need to link the DirectDraw libs.
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        dxguid.lib
        ddraw.lib
    )
else()
    # Linux defines
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -D__linux__
    )

endif()


# Additional debug logs when building debug variant
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGDBG_INFO_ON 
        -DGLIDE_DEBUG
        -DGLIDE_SANITY_ASSERT
        -DGLIDE_SANITY_SIZE
    )
endif()

# OS-specific-specific defines
if(WIN32)
    
    # All attributes starting with "_NASM" are needed to be able to build the asm sources correctly.
    # The *.asm files contain os-specific sections, that are switched using the defines below.
    # NASM does not provide any _WIN32 or unix defines, so we have to add them ourselves.
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        _NASM_BUILD_FOR_WIN32
    )
    
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        _NASM_BUILD_FOR_LINUX
    )
endif()
