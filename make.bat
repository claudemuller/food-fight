clang ^
    -target x86_64-pc-windows-gnu ^
    -std=c11 -Wall -Wextra -DTINYFD_NO_POPEN -g ^
    --sysroot="C:/msys64/mingw64" ^
    -I"C:\lib\raylib-5.5-mingw_w64\include" ^
    -I"lib" ^
    src\*.c ^
    lib\raygui\*.c ^
    -o bin\foodfight.exe ^
    -L"C:\lib\raylib-5.5-mingw_w64\lib" ^
    -L"C:\msys64\mingw64\lib" ^
    -lraylib -lopengl32 -lgdi32 -lwinmm

@echo off
REM cl ^
REM     /std:c11 /W4 /D TINYFD_NO_POPEN /Zi ^
REM     /I "C:\lib\raylib-5.5-mingw_w64\include" ^
REM     /I "C:\Users\lukefilewalker\repos\3rd-party\nativefiledialog-extended\src\include" ^
REM     /I "lib" ^
REM     src\*.c ^
REM     /link ^
REM     /OUT:bin\foodfight.exe ^
REM     /LIBPATH:"C:\lib\raylib-5.5-mingw_w64\lib" ^
REM     raylib.lib opengl32.lib gdi32.lib winmm.lib ^
REM     ole32.lib uuid.lib comdlg32.lib shlwapi.lib shell32.lib oleaut32.lib ^
REM     shobjidl_core.lib comctl32.lib ^
REM     "C:\Users\lukefilewalker\repos\3rd-party\nativefiledialog-extended\build\src\nfd.lib" ^
REM     /SUBSYSTEM:WINDOWS

