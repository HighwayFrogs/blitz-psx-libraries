!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O2 -g -comments-c++ -Wall
!else
OPT = -c -O2 -comments-c++ -Wall
!endif

CCPSX = C:\Psx\bin\ccpsx
ASMPSX = C:\Psx\bin\asmpsx
PSYLIB = C:\Psx\bin\psylib

#
# DEFAULT BUILD RULE
#

.c.obj:
        $(CCPSX) $(OPT) $&.c -o$&.obj

!if $(DEBUG) == 1
Debug\islvideo.lib: islvideo.obj
        del Debug\islvideo.lib
        $(PSYLIB) /a Debug\islvideo.lib islvideo.obj
        del islvideo.obj
!else
Release\islvideo.lib: islvideo.obj
        del Release\islvideo.lib
        $(PSYLIB) /a Release\islvideo.lib islvideo.obj
        del islvideo.obj
!endif

islvideo.obj: islvideo.c islvideo.h
