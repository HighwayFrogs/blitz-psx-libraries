!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O0 -comments-c++ -Wall
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
Debug\islcard.lib: islcard.obj
        del Debug\islcard.lib
        $(PSYLIB) /a Debug\islcard.lib islcard.obj
        del islcard.obj
!else
Release\islcard.lib: islcard.obj
        del Release\islcard.lib
        $(PSYLIB) /a Release\islcard.lib islcard.obj
        del islcard.obj
!endif

islcard.obj: islcard.c islcard.h
