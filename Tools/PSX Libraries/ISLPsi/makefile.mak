!if $(DEBUG) == 1
OPT = -D_DEBUG -c -O2 -comments-c++ -Wall
!else
OPT = -c -O2 -comments-c++ -Wall
!endif

CCPSX = C:\Psx\bin\ccpsx
ASMPSX = C:\Psx\bin\asmpsx
PSYLIB = C:\Psx\bin\psylib
DMPSX = C:\Psx\bin\dmpsx

#
# DEFAULT BUILD RULES
#

.c.obj:
        $(CCPSX) $(OPT) $&.c -o$&.obj
        $(DMPSX) -b $&.obj

.s.obj:
        $(ASMPSX) /zd /l $&.s,$&.obj

!if $(DEBUG) == 1
Debug\islpsi.lib: psi.obj asm.obj quatern.obj
        del Debug\islpsi.lib
        $(PSYLIB) /a Debug\islpsi.lib psi.obj asm.obj quatern.obj
        del psi.obj
        del asm.obj
	del quatern.obj
!else
Release\islpsi.lib: psi.obj asm.obj quatern.obj
        del Release\islpsi.lib
        $(PSYLIB) /a Release\islpsi.lib psi.obj asm.obj quatern.obj
        del psi.obj
        del asm.obj
	del quatern.obj
!endif

psi.obj: psi.c psi.h psitypes.h
asm.obj: asm.s
quatern.obj: quatern.c quatern.h

