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
Debug\isllocal.lib: isllocal.obj
        del Debug\isllocal.lib
        $(PSYLIB) /a Debug\isllocal.lib isllocal.obj
        del isllocal.obj
!else
Release\isllocal.lib: isllocal.obj
        del Release\isllocal.lib
        $(PSYLIB) /a Release\isllocal.lib isllocal.obj
        del isllocal.obj
!endif

isllocal.obj: isllocal.c isllocal.h
