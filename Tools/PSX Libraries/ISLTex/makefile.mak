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
Debug\isltex.lib: isltex.obj
	del Debug\isltex.lib
        $(PSYLIB) /a Debug\isltex.lib isltex.obj
	del isltex.obj
!else
Release\isltex.lib: isltex.obj
	del Release\isltex.lib
        $(PSYLIB) /a Release\isltex.lib isltex.obj
	del isltex.obj
!endif

isltex.obj: isltex.c isltex.h



