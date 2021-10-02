CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Glide2x VERSION 1.0 DESCRIPTION "Glide2x API for Voodoo3" LANGUAGES C ASM_NASM)

OPTION(BUILD_32BIT "Build for 32-bit targets, otherwise x64" OFF)
OPTION(USE_3DNOW "Support 3DNow" OFF) # this probably anyway doesn't work
OPTION(USE_FIFO "Enable FIFO support" ON)

# Set the configuration for the assembler
if (${BUILD_32BIT})
    if(MSVC)
        # Nothing to do - MSVC supports building 32bit using a different cmake invokation
    else()
        # Set this flag for the compiler and linker
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m32")
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
set (MINIHWC_DIR ${CMAKE_SOURCE_DIR}/minihwc)

set(GLIDE_SOURCES 
    ${CMAKE_SOURCE_DIR}/glide/src/fifo.c
    ${CMAKE_SOURCE_DIR}/glide/src/gsplash.c 
    ${CMAKE_SOURCE_DIR}/glide/src/g3df.c  
    ${CMAKE_SOURCE_DIR}/glide/src/gu.c 
    ${CMAKE_SOURCE_DIR}/glide/src/guclip.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gpci.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gump.c 
    ${CMAKE_SOURCE_DIR}/glide/src/diglide.c 
    ${CMAKE_SOURCE_DIR}/glide/src/disst.c 
    ${CMAKE_SOURCE_DIR}/glide/src/ditex.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gbanner.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gerror.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gmovie.c 
    ${CMAKE_SOURCE_DIR}/glide/src/digutex.c 
    ${CMAKE_SOURCE_DIR}/glide/src/ddgump.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gaa.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gdraw.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gglide.c 
    ${CMAKE_SOURCE_DIR}/glide/src/glfb.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gsst.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gtex.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gtexdl.c 
    ${CMAKE_SOURCE_DIR}/glide/src/gutex.c 
    ${CMAKE_SOURCE_DIR}/glide/src/xtexdl_def.c
    ${CMAKE_SOURCE_DIR}/glide/src/cpudtect_stubs.c
)



set(GLIDE_SOURCES ${GLIDE_SOURCES} 
    # We always use the C routines for drawing triangles, never the asm or 3dnow ones
    ${CMAKE_SOURCE_DIR}/glide/src/gxdraw.c
)

# platform specific compile files
if(WIN32)
    # Windows build
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
endif()

# Create a shared library 
add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${GLIDE_SOURCES}
    
    ${GLIDE_OUTSIDE_SOURCES}
    ${GLIDE_ASM_SOURCES})


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/glide/src
    ${CMAKE_SOURCE_DIR}/incsrc
    ${CMAKE_SOURCE_DIR}/init
    ${CMAKE_SOURCE_DIR}/minihwc
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmisc
    ${CMAKE_SOURCE_DIR}/../../swlibs/newpci/pcilib
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmemmap
)

#if(MSVC)
#  # disable all warnings for now (too many)
#  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W0)
#else()
#  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
#endif()


target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    # general
    -D__3Dfx_PCI_CFG__ 
    -DGLIDE_HW_TRI_SETUP=1
    -DGLIDE_INIT_HWC    
    -DGLIDE_PACKED_RGB=0
    -DGLIDE_TRI_CULLING=1 
    -DGLIDE_DEFAULT_GAMMA=1.3f
    -DGLIDE_LIB=1
    -DHAL_HW=1
    
    # subsystem
    -DH3
    
    # other 
    -DGLIDE_PLUG 
    -DGLIDE_SPLASH 
    
    # forgot where this came from
    #-DGLIDE_DISPATCH_SETUP=1 // for asm only
    -DGLIDE_DISPATCH_DOWNLOAD=1 
    -DGLIDE_CHIP_BROADCAST=1 
    -DGLIDE_BLIT_CLEAR=1 
    -DCVG 

    -DGL_X86
    
    # Do not use the ASM routines, but the C triangle setup routines instead
    -DGLIDE_USE_C_TRISETUP=1
)
# Use FIFO?
if (${USE_FIFO})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DUSE_PACKET_FIFO=1
        -DGLIDE_PACKET3_TRI_SETUP=1
    )
endif()


if (${BUILD_32BIT})

else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGLIDE_BUILD_64BIT
    )

endif()


if(WIN32)
    # Windows-only defines
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DHWC_EXT_INIT=1
        -DFX_DLL_ENABLE 
        -DHWC_ACCESS_DDRAW=1
        
        -DWINNT
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


#gcc -o fxgasm -Wall -I. -I../../incsrc -I../../init -I../../../swlibs/fxmisc -I../../../swlibs/newpci/pcilib -I../../../swlibs/fxmemmap #-I../../../swlibs/texus2/lib -DGLIDE3 -DGLIDE3_ALPHA -DGLIDE_HW_TRI_SETUP=1 -DGLIDE_PACKED_RGB=0 -DGLIDE_PACKET3_TRI_SETUP=1 -DGLIDE_TRI_CULLING=1# -DUSE_PACKET_FIFO=1 -DGLIDE_DISPATCH_SETUP -DGLIDE_DISPATCH_DOWNLOAD -DGLIDE_CHIP_BROADCAST=1 -DGLIDE_BLIT_CLEAR=1 -DCVG -DGL_MMX -DGL_AMD3D -DGL_X86 -O1 -ffast-math fxgasm.c