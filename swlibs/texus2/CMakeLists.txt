add_library(texus2
    lib/texuslib.c
    lib/clamp.c
    lib/read.c
    lib/resample.c
    lib/mipmap.c
    lib/quantize.c
    lib/ncc.c
    lib/nccnnet.c
    lib/pal256.c
    lib/pal6666.c
    lib/dequant.c
    lib/view.c
    lib/util.c
    lib/diffuse.c
    lib/write.c
    lib/tga.c
    lib/3df.c
    lib/ppm.c
    lib/rgt.c
    lib/txs.c
    lib/codec.c
    lib/eigen.c
    lib/bitcoder.c
)

target_link_libraries(texus2 PUBLIC fxmisc)
