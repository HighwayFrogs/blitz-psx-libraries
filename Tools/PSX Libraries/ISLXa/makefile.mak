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
Debug\islxa.lib: islxa.obj
        del Debug\islxa.lib
        $(PSYLIB) /a Debug\islxa.lib islxa.obj
        del islxa.obj
!else
Release\islxa.lib: islxa.obj
        del Release\islxa.lib
        $(PSYLIB) /a Release\islxa.lib islxa.obj
        del islxa.obj
!endif

islxa.obj: islxa.c islxa.h
