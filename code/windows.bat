@echo off
setlocal

if not exist "..\build" (mkdir "..\build")

pushd "..\build" > nul

call ..\base\renderer\windows_wgl.bat debug

SET compiler_flags=-nologo -W4 -wd4201 -wd4244 -wd4505 -wd4305 -I "..\base"
SET linker_flags=user32.lib kernel32.lib gdi32.lib pathcch.lib shell32.lib shcore.lib ole32.lib

cl %compiler_flags% -DLUDUM_INTERNAL=1 -Od -Zi "..\code\win32_ludum.cpp" -Fe"ludum_debug.exe" -link %linker_flags%

popd > nul
