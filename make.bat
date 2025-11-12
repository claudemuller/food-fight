clang ^
    -target x86_64-pc-windows-gnu ^
    -std=c11 -Wall -Wextra -DTINYFD_NO_POPEN -g ^
    --sysroot="C:/msys64/mingw64" ^
    -I"C:\lib\raylib-5.5-mingw_w64\include" ^
    -I"lib" ^
    src\*.c ^
    -o bin\foodfight.exe ^
    -L"C:\lib\raylib-5.5-mingw_w64\lib" ^
    -lraylib -lopengl32 -lgdi32 -lwinmm
