!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O0 -g -comments-c++ -Wall
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
Debug\islsfx2.lib: islsfx2.obj
        del Debug\islsfx2.lib
        $(PSYLIB) /a Debug\islsfx2.lib islsfx2.obj
        del islsfx2.obj
!else
Release\islsfx2.lib: islsfx2.obj
        del Release\islsfx2.lib
        $(PSYLIB) /a Release\islsfx2.lib islsfx2.obj
        del islsfx2.obj
!endif

islsfx2.obj: islsfx2.c islsfx2.h



