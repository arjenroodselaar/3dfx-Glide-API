CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(fxgasm VERSION 1.0 DESCRIPTION "Glide2x FXGASM Tool for Voodoo3" LANGUAGES C)

OPTION(USE_FIFO "Enable FIFO support" ON)

# Create an executable with the main
add_executable(${CMAKE_PROJECT_NAME} src/fxgasm.c)


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/../../../incsrc
    ${CMAKE_SOURCE_DIR}/../../../init
    ${CMAKE_SOURCE_DIR}/../../../minihwc
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/fxmisc
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/newpci/pcilib
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/fxmemmap
)

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
    -DGLIDE_DISPATCH_SETUP=1 
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

if(WIN32)
    # Windows-only defines
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DHWC_EXT_INIT=1
        -DFX_DLL_ENABLE 
        -DHWC_ACCESS_DDRAW=1
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
