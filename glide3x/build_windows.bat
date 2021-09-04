

rem DXE_LD_LIBRARY_PATH=/usr/local/cross-djgpp/i586-pc-msdosdjgpp/lib

rem export PATH
rem export DXE_LD_LIBRARY_PATH

make -f Makefile.win32 FX_GLIDE_HW=cvg USE_X86=1 USE_MMX=1 USE_3DNOW=1 USE_SSE=1 USE_SSE2=1