rem *** Remove old versions ***

deltree /Y Distribution\Include
deltree /Y Distribution\Lib
deltree /Y Distribution\Source
md Distribution\Include
md Distribution\Lib
md Distribution\Lib\Debug
md Distribution\Lib\Release
md Distribution\Source

rem *** Build ISLMem Library ***
cd islmem
psymake /dDEBUG=1
psymake
cd ..
copy islmem\islmem.h Distribution\Include
copy islmem\Debug\islmem.lib Distribution\Lib\Debug
copy islmem\Release\islmem.lib Distribution\Lib\Release

rem *** Built ISLUtil Library ***
cd islutil
psymake /dDEBUG=1
psymake
cd ..
copy islutil\islutil.h Distribution\Include
copy islutil\Debug\islutil.lib Distribution\Lib\Debug
copy islutil\Release\islutil.lib Distribution\Lib\Release

rem *** Build ISLFile Library ***
cd islfile
psymake /dDEBUG=1
psymake
cd ..
copy islfile\islfile.h Distribution\Include
copy islfile\Debug\islfile.lib Distribution\Lib\Debug
copy islfile\Release\islfile.lib Distribution\Lib\Release

rem *** Build ISLPad Library ***
cd islpad
psymake /dDEBUG=1
psymake
cd ..
copy islpad\islpad.h Distribution\Include
copy islpad\Debug\islpad.lib Distribution\Lib\Debug
copy islpad\Release\islpad.lib Distribution\Lib\Release

rem *** Build ISLTex Library ***
cd isltex
psymake /dDEBUG=1
psymake
cd ..
copy isltex\isltex.h Distribution\Include
copy isltex\Debug\isltex.lib Distribution\Lib\Debug
copy isltex\Release\isltex.lib Distribution\Lib\Release

rem *** Build ISLFont Library ***
cd islfont
psymake /dDEBUG=1
psymake
cd ..
copy islfont\islfont.h Distribution\Include
copy islfont\Debug\islfont.lib Distribution\Lib\Debug
copy islfont\Release\islfont.lib Distribution\Lib\Release

rem *** Build ISLXa Library ***
cd islxa
psymake /dDEBUG=1
psymake
cd ..
copy islxa\islxa.h Distribution\Include
copy islxa\Debug\islxa.lib Distribution\Lib\Debug
copy islxa\Release\islxa.lib Distribution\Lib\Release

rem *** Build ISLLocal Library ***
cd isllocal
psymake /dDEBUG=1
psymake
cd ..
copy isllocal\isllocal.h Distribution\Include
copy isllocal\Debug\isllocal.lib Distribution\Lib\Debug
copy isllocal\Release\isllocal.lib Distribution\Lib\Release

rem *** Build ISLCard Library ***
cd islcard
psymake /dDEBUG=1
psymake
cd ..
copy islcard\islcard.h Distribution\Include
copy islcard\Debug\islcard.lib Distribution\Lib\Debug
copy islcard\Release\islcard.lib Distribution\Lib\Release

rem *** Build ISLSound Library ***
cd islsound
psymake /dDEBUG=1
psymake
cd ..
copy islsound\islsound.h Distribution\Include
copy islsound\Debug\islsound.lib Distribution\Lib\Debug
copy islsound\Release\islsound.lib Distribution\Lib\Release

rem *** Build ISLVideo Library ***
cd islvideo
psymake /dDEBUG=1
psymake
cd ..
copy islvideo\islvideo.h Distribution\Include
copy islvideo\Debug\islvideo.lib Distribution\Lib\Debug
copy islvideo\Release\islvideo.lib Distribution\Lib\Release

rem *** Build ISLPsi Library ***
cd islpsi
psymake /dDEBUG=1
psymake
cd ..
copy islpsi\islpsi.h Distribution\Include
copy islpsi\psitypes.h Distribution\Include
copy islpsi\Debug\islpsi.lib Distribution\Lib\Debug
copy islpsi\Release\islpsi.lib Distribution\Lib\Release
copy islpsi\actor.c Distribution\Source
copy islpsi\actor.h Distribution\Source
copy islpsi\custom.c Distribution\Source

rem *** Copy shell headers ***
copy shell\shell.h Distribution\Include
