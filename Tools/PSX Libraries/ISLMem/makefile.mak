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
Debug\islmem.lib: islmem.obj
	del Debug\islmem.lib
	$(PSYLIB) /a Debug\islmem.lib islmem.obj
	del islmem.obj
!else
Release\islmem.lib: islmem.obj
	del Release\islmem.lib
	$(PSYLIB) /a Release\islmem.lib islmem.obj
	del islmem.obj
!endif

islmem.obj: islmem.c islmem.h
