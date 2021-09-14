CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(fxgasm VERSION 1.0 DESCRIPTION "Template solution by Clemens Zangl" LANGUAGES C)

# Create an executable with the main
add_executable(${CMAKE_PROJECT_NAME} src/fxgasm.c)


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/../
    ${CMAKE_SOURCE_DIR}/../../../incsrc
    ${CMAKE_SOURCE_DIR}/../../../init
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/fxmisc
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/newpci/pcilib
    ${CMAKE_SOURCE_DIR}/../../../../../swlibs/fxmemmap
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    -DDIRECTX 
    -DFX_DLL_ENABLE 
    -D__3Dfx_PCI_CFG__ 
    -DGLIDE_HW_TRI_SETUP=1 
    -DGLIDE_PACKED_RGB=1 
    -DGLIDE_PACKET3_TRI_SETUP=1 
    -DGLIDE_TRI_CULLING=1 
    -DGLIDE_DEFAULT_GAMMA=1.3f
    -DGLIDE_LIB=1
    
    -DUSE_PACKET_FIFO=1 
    -DGLIDE_DISPATCH_SETUP=1 
    -DGLIDE_DISPATCH_DOWNLOAD=1 
    -DGLIDE_CHIP_BROADCAST=1 
    -DGLIDE_BLIT_CLEAR=1 
    -DCVG 
    -DGLIDE_PLUG 
    -DGLIDE_SPLASH 
    -DGL_X86
    -DGLIDE_USE_C_TRISETUP
)
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
        -DGDBG_INFO_ON 
        -DGLIDE_DEBUG
        -DGLIDE_SANITY_ASSERT
        -DGLIDE_SANITY_SIZE
    )
endif()


#gcc -o fxgasm -Wall -I. -I../../incsrc -I../../init -I../../../swlibs/fxmisc -I../../../swlibs/newpci/pcilib -I../../../swlibs/fxmemmap #-I../../../swlibs/texus2/lib -DGLIDE3 -DGLIDE3_ALPHA -DGLIDE_HW_TRI_SETUP=1 -DGLIDE_PACKED_RGB=0 -DGLIDE_PACKET3_TRI_SETUP=1 -DGLIDE_TRI_CULLING=1# -DUSE_PACKET_FIFO=1 -DGLIDE_DISPATCH_SETUP -DGLIDE_DISPATCH_DOWNLOAD -DGLIDE_CHIP_BROADCAST=1 -DGLIDE_BLIT_CLEAR=1 -DCVG -DGL_MMX -DGL_AMD3D -DGL_X86 -O1 -ffast-math fxgasm.c