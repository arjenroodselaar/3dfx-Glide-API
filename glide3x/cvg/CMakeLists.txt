CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Glide3x VERSION 1.0 DESCRIPTION "Template solution by Clemens Zangl" LANGUAGES C ASM_NASM)

OPTION(BUILD_32BIT "Build for 32-bit targets (default is x64" OFF)
OPTION(USE_X86 "Buld for x86 target" ON)
OPTION(USE_3DNOW "Support 3DNow" OFF)

set (NEWPCI_LIB_DIR ${CMAKE_SOURCE_DIR}/../../swlibs/newpci/pcilib)

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
    # Does not exists for Voodoo2 variant ${CMAKE_SOURCE_DIR}/glide3/src/xtexdl_def.c
)

set (GLIDE_ASM_SOURCE
   #${CMAKE_SOURCE_DIR}/glide3/src/xdraw2.S
   #${CMAKE_SOURCE_DIR}/glide3/src/xdraw3.asm
)

if (${USE_X86})
    set(GLIDE_SOURCES ${GLIDE_SOURCES} 
        ${CMAKE_SOURCE_DIR}/glide3/src/cpuid.c
        #${CMAKE_SOURCE_DIR}/glide3/src/xdraw2_def.c
        #${CMAKE_SOURCE_DIR}/glide3/src/xdraw3_def.c
    )
endif()

if (${USE_3DNOW})
    set(GLIDE_SOURCES ${GLIDE_SOURCES} 
        ${CMAKE_SOURCE_DIR}/glide3/src/xdraw2_3dnow.c
        ${CMAKE_SOURCE_DIR}/glide3/src/xdraw3_3dnow.c
        ${CMAKE_SOURCE_DIR}/glide3/src/xtexdl_3dnow.c
    )
else()
    set(GLIDE_SOURCES ${GLIDE_SOURCES} 
        ${CMAKE_SOURCE_DIR}/glide3/src/gxdraw.c
        ${CMAKE_SOURCE_DIR}/glide3/src/xtexdl.c
    )
endif()


set(GLIDE_OUTSIDE_SOURCES
    ${CMAKE_SOURCE_DIR}/glide3/src/digutex.c
    ${NEWPCI_LIB_DIR}/fxpci.c
    ${NEWPCI_LIB_DIR}/fxmsr.c
    ${CMAKE_SOURCE_DIR}/init/canopus.c
    ${CMAKE_SOURCE_DIR}/init/dac.c
    ${CMAKE_SOURCE_DIR}/init/gamma.c
    ${CMAKE_SOURCE_DIR}/init/gdebug.c
    ${CMAKE_SOURCE_DIR}/init/info.c
    ${CMAKE_SOURCE_DIR}/init/parse.c
    ${CMAKE_SOURCE_DIR}/init/print.c
    ${CMAKE_SOURCE_DIR}/init/sli.c
    ${CMAKE_SOURCE_DIR}/init/sst1init.c
    ${CMAKE_SOURCE_DIR}/init/util.c
    ${CMAKE_SOURCE_DIR}/init/video.c
    ${CMAKE_SOURCE_DIR}/init/fxremap.c
)

# platform specific compile files
set(WINDOWS_DEPENDENCIES
    ${NEWPCI_LIB_DIR}/fxnt.c
    ${NEWPCI_LIB_DIR}/fxw32.c
    ${NEWPCI_LIB_DIR}/fxvxd.c
)

set(LINUX_DEPENDENCIES
    ${NEWPCI_LIB_DIR}/fxlinux.c
)

if(WIN32)
    set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
       ${WINDOWS_DEPENDENCIES} 
    )
else()
    set(GLIDE_OUTSIDE_SOURCES ${GLIDE_OUTSIDE_SOURCES}
       ${LINUX_DEPENDENCIES} 
    )
endif()

# Create a shared library 
add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${GLIDE_SOURCES}
    
    ${GLIDE_OUTSIDE_SOURCES}
    ${GLIDE_ASM_SOURCE})


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/glide3/src
    ${CMAKE_SOURCE_DIR}/incsrc
    ${CMAKE_SOURCE_DIR}/init
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmisc
    ${CMAKE_SOURCE_DIR}/../../swlibs/newpci/pcilib
    ${CMAKE_SOURCE_DIR}/../../swlibs/fxmemmap
)

if(MSVC)
  # disable all warnings for now (too many)
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W0)
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    -DDIRECTX 
    -DFX_DLL_ENABLE 
    -D__3Dfx_PCI_CFG__ 
    -DGLIDE3 
    -DGLIDE3_ALPHA 
    -DGLIDE3_VERTEX_LAYOUT=1 
    -DGLIDE_HW_TRI_SETUP=1 
    -DGLIDE_PACKED_RGB=0 
    -DGLIDE_PACKET3_TRI_SETUP=1 
    -DGLIDE_TRI_CULLING=1 
    -DUSE_PACKET_FIFO=1 
    -DGLIDE_DISPATCH_SETUP=1 
    -DGLIDE_DISPATCH_DOWNLOAD=1 
    -DGLIDE_CHIP_BROADCAST=1 
    -DGLIDE_BLIT_CLEAR=1 
    -DCVG 
    -DGLIDE_PLUG 
    -DGLIDE_SPLASH 
    #-G6 
    #-O2 
    #-DGL_MMX do not use any assembler optimization, just let the compiler figure out what is the fastest 
    -DGL_X86
    #-DGL_AMD3D
    # For the assembler environment
    #-DXOS XOS_WIN32
    #-DSTDCALL 1
    
    # Use C triangle drawing routines, not the assembler ones (the asm won't compile under x64)
    -DGLIDE_USE_C_TRISETUP
)

if (${BUILD_32BIT})
    if(MSVC)
        set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    else()
        set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    endif()
    
    #target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -m32)
endif()

#gcc -o fxgasm -Wall -I. -I../../incsrc -I../../init -I../../../swlibs/fxmisc -I../../../swlibs/newpci/pcilib -I../../../swlibs/fxmemmap #-I../../../swlibs/texus2/lib -DGLIDE3 -DGLIDE3_ALPHA -DGLIDE_HW_TRI_SETUP=1 -DGLIDE_PACKED_RGB=0 -DGLIDE_PACKET3_TRI_SETUP=1 -DGLIDE_TRI_CULLING=1# -DUSE_PACKET_FIFO=1 -DGLIDE_DISPATCH_SETUP -DGLIDE_DISPATCH_DOWNLOAD -DGLIDE_CHIP_BROADCAST=1 -DGLIDE_BLIT_CLEAR=1 -DCVG -DGL_MMX -DGL_AMD3D -DGL_X86 -O1 -ffast-math fxgasm.c