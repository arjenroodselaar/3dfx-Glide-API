if(WIN32)
    set(FXPCI_OS_SPECIFIC_SOURCES
        pcilib/fxw32.c
    )
else ()
    set(FXPCI_OS_SPECIFIC_SOURCES
        pcilib/fxlinux.c
    )
endif()


add_library(pcilib
    ${FXPCI_OS_SPECIFIC_SOURCES}
    pcilib/fxpci.c

)
target_include_directories(pcilib PUBLIC pcilib)

target_link_libraries(pcilib PUBLIC glide_common fxmemmap fxmisc)
