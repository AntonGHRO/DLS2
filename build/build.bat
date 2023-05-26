@echo off

:: Compiler and linker location
:: C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x86\cl.exe
:: C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x86\link.exe

:: Compiler flags
set cl_flags=                       /Od /std:c11

set cl_includes_gl=                 /I "C:\GL_libs\include"
set cl_includes_msvc=               /I "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.35.32215\include"
set cl_includes_winkits_um=         /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.20348.0\um"
set cl_includes_winkits_shared=     /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.20348.0\shared"
set cl_includes_winkits_ucrt=       /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.20348.0\ucrt"

set cl_includes_all=                %cl_includes_gl% %cl_includes_msvc% %cl_includes_winkits_um% %cl_includes_winkits_shared% %cl_includes_winkits_ucrt%

:: Linker flags
set ln_libs_gl=                     /libpath:"C:\GL_libs\lib\x86"
set ln_libs_msvc=                   /libpath:"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.35.32215\lib\x86"
set ln_libs_winkits_um=             /libpath:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.20348.0\um\x86"
set ln_libs_winkits_ucrt=           /libpath:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.20348.0\ucrt\x86"

set ln_libs_all=                    %ln_libs_gl% %ln_libs_msvc% %ln_libs_winkits_um% %ln_libs_winkits_ucrt%

set ln_libs=                        glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib
set ln_flags=                       /defaultlib:msvcrt.lib /subsystem:console /machine:x86

:: Sources and objects
set sources_graphics=               ..\graphics\main.c ..\graphics\math.c ..\graphics\files.c ..\graphics\render.c ..\graphics\glad.c
set sources_simulator=              ..\simulator\scene.c ..\simulator\core.c
set objects=                        .\main.obj .\math.obj .\files.obj .\render.obj .\glad.obj .\core.obj .\scene.obj

:: Compile
cl.exe %cl_flags% %cl_includes_all% -c %sources_graphics% %sources_simulator%

:: Linking
link.exe %ln_libs_all% %ln_libs% %ln_flags% %objects% /out:../SLD.exe
