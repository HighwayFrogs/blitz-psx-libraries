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
Debug\islpad.lib: islpad.obj
	del Debug\islpad.lib
        $(PSYLIB) /a Debug\islpad.lib islpad.obj
	del islpad.obj
!else
Release\islpad.lib: islpad.obj
	del Release\islpad.lib
        $(PSYLIB) /a Release\islpad.lib islpad.obj
	del islpad.obj
!endif

islpad.obj: islpad.c islpad.h



