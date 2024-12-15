set (GLIDE_H5_DIR ${CMAKE_SOURCE_DIR}/glide3x/h5)

set (CINIT_DIR ${GLIDE_H5_DIR}/cinit)

set (MINIHWC_DIR ${GLIDE_H5_DIR}/minihwc)

set(GLIDE_SOURCES 
    ${GLIDE_H5_DIR}/glide3/src/fifo.c
    ${GLIDE_H5_DIR}/glide3/src/distate.c
    ${GLIDE_H5_DIR}/glide3/src/gstrip.c
    ${GLIDE_H5_DIR}/glide3/src/distrip.c
    ${GLIDE_H5_DIR}/glide3/src/diget.c
    ${GLIDE_H5_DIR}/glide3/src/gsplash.c 
    ${GLIDE_H5_DIR}/glide3/src/g3df.c  
    ${GLIDE_H5_DIR}/glide3/src/gu.c 
    ${GLIDE_H5_DIR}/glide3/src/gpci.c 
    ${GLIDE_H5_DIR}/glide3/src/diglide.c 
    ${GLIDE_H5_DIR}/glide3/src/disst.c 
    ${GLIDE_H5_DIR}/glide3/src/ditex.c 

    ${GLIDE_H5_DIR}/glide3/src/gbanner.c 
    ${GLIDE_H5_DIR}/glide3/src/gerror.c 
    ${GLIDE_H5_DIR}/glide3/src/gaa.c 
    ${GLIDE_H5_DIR}/glide3/src/gdraw.c 
    ${GLIDE_H5_DIR}/glide3/src/gglide.c 
    ${GLIDE_H5_DIR}/glide3/src/glfb.c 
    ${GLIDE_H5_DIR}/glide3/src/gsst.c 
    ${GLIDE_H5_DIR}/glide3/src/gtex.c 
    ${GLIDE_H5_DIR}/glide3/src/gtexdl.c 
    #${GLIDE_H5_DIR}/glide3/src/gutex.c 
    ${GLIDE_H5_DIR}/glide3/src/xtexdl_def.c
    #${GLIDE_H5_DIR}/glide3/src/cpudtect_stubs.c
    ${GLIDE_H5_DIR}/glide3/src/gthread.c
)



set(GLIDE_SOURCES ${GLIDE_SOURCES} 
    # We always use the C routines for drawing triangles, never the asm or 3dnow ones
    ${GLIDE_H5_DIR}/glide3/src/gxdraw.c
)

# platform specific compile files
if(WIN32)
    # Windows build
    set(GLIDE_SOURCES ${GLIDE_SOURCES}
        ${GLIDE_H5_DIR}/glide3/src/gsfc.c
        ${GLIDE_H5_DIR}/glide3/src/winsurf.c
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



# Create a shared library 
add_library(${CMAKE_PROJECT_NAME} SHARED 
    ${GLIDE_SOURCES}
    ${GLIDE_OUTSIDE_SOURCES}
    ${GLIDE_ASM_SOURCES}
    )

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${GLIDE_H5_DIR}/glide3/src
    ${GLIDE_H5_DIR}/incsrc
    ${GLIDE_H5_DIR}/init
    ${GLIDE_H5_DIR}/minihwc
    ${CINIT_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    pcilib
)

if (${EMBED_TEXUS2})
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        texus2
    )
endif()

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
    #-DGL_X86
)


if (${BUILD_32BIT})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGL_X86
    )
else()
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGLIDE_BUILD_64BIT
        # Do not use the ASM routines, but the C triangle setup routines instead
        -DGLIDE_USE_C_TRISETUP
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
