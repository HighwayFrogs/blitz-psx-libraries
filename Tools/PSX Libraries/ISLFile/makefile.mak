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
Debug\islfile.lib: islfile.obj
	del Debug\islfile.lib
        $(PSYLIB) /a Debug\islfile.lib islfile.obj
	del islfile.obj
!else
Release\islfile.lib: islfile.obj
	del Release\islfile.lib
        $(PSYLIB) /a Release\islfile.lib islfile.obj
	del islfile.obj
!endif

islfile.obj: islfile.c islfile.h



