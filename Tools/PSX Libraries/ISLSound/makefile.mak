!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O0 -X0$00018000 -comments-c++ -Wall
!else
OPT = -c -O2 -X0$00018000 -comments-c++ -Wall
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
Debug\islsound.lib: islsound.obj
        del Debug\islsound.lib
        $(PSYLIB) /a Debug\islsound.lib islsound.obj
        del islsound.obj
!else
Release\islsound.lib: islsound.obj
        del Release\islsound.lib
        $(PSYLIB) /a Release\islsound.lib islsound.obj
        del islsound.obj
!endif

islsound.obj: islsound.c islsound.h



