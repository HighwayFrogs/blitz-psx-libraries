!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O0 -comments-c++ -Wall
!else
OPT = -c -O2 -comments-c++ -Wall
!endif

CCPSX = C:\Psx\bin\ccpsx
ASMPSX = C:\Psx\bin\asmpsx
PSYLIB = C:\Psx\bin\psylib

#
# DEFAULT BUILD RULES
#

.c.obj:
        $(CCPSX) $(OPT) $&.c -o$&.obj

.s.obj:
        $(ASMPSX) /zd /l $&.s,$&.obj

!if $(DEBUG) == 1
Debug\islutil.lib: islutil.obj islex.obj islex2.obj incbin.obj
        del Debug\islutil.lib
        $(PSYLIB) /a Debug\islutil.lib islutil.obj islex.obj islex2.obj incbin.obj
        del islutil.obj
	del islex.obj
	del islex2.obj
	del incbin.obj
!else
Release\islutil.lib: islutil.obj islex.obj islex2.obj incbin.obj
        del Release\islutil.lib
        $(PSYLIB) /a Release\islutil.lib islutil.obj islex.obj islex2.obj incbin.obj
        del islutil.obj
	del islex.obj
	del islex2.obj
	del incbin.obj
!endif

islutil.obj: islutil.c islutil.h
islex.obj: islex.c islex.h
islex2.obj: islex2.s
incbin.obj: incbin.s sqrtable.bin asintabl.bin
