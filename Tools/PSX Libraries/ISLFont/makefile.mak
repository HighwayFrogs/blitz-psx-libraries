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
Debug\islfont.lib: islfont.obj
	del Debug\islfont.lib
        $(PSYLIB) /a Debug\islfont.lib islfont.obj
	del islfont.obj
!else
Release\islfont.lib: islfont.obj
	del Release\islfont.lib
        $(PSYLIB) /a Release\islfont.lib islfont.obj
	del islfont.obj
!endif

islfont.obj: islfont.c islfont.h



