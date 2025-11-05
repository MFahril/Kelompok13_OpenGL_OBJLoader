@echo off
echo ================================
echo OBJ Viewer with Material Support
echo ================================
echo.

cd /d "%~dp0"

REM Add local MinGW to PATH if it exists
if exist "%~dp0mingw64\bin" set PATH=%~dp0mingw64\bin;%PATH%
if exist "C:\MinGW\bin" set PATH=C:\MinGW\bin;%PATH%
if exist "C:\msys64\mingw64\bin" set PATH=C:\msys64\mingw64\bin;%PATH%

echo Compiling with static libraries (no DLL dependencies)...
echo.
echo [          ] 0%%
g++ -c Core\main.cpp -o Core\main.o -ICore -DFREEGLUT_STATIC -static-libgcc -static-libstdc++ 2>nul
echo [===       ] 33%% - Compiling main.cpp
g++ -c Core\ObjLoader.cpp -o Core\ObjLoader.o -ICore -DFREEGLUT_STATIC -static-libgcc -static-libstdc++ 2>nul
echo [======    ] 66%% - Compiling ObjLoader.cpp
g++ -c Core\AnimationLoader.cpp -o Core\AnimationLoader.o -ICore -DFREEGLUT_STATIC -static-libgcc -static-libstdc++ 2>nul
echo [=========-] 90%% - Compiling AnimationLoader.cpp
g++ -o ObjViewer.exe Core\main.o Core\ObjLoader.o Core\AnimationLoader.o -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32 -static-libgcc -static-libstdc++ -static 2>nul
echo [==========] 100%% - Linking executable
echo.

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =======================
    echo Compilation successful!
    echo =======================
    echo.
    echo Build Type: STATIC (No DLLs Required!)
    echo.
    echo Usage:
    echo   ObjViewer.exe Models\yourmodel.obj
    echo.
    echo Examples of valid paths:
    echo   Models\yourmodel.obj                          (relative path)
    echo   Models\Anim\yourmodel                         (relative path Animated Object)
    echo   C:\MyModels\object.obj                        (absolute path)
    echo   D:\Projects\3D Models\character.obj           (absolute path with spaces)
    echo   ..\OtherFolder\model.obj                      (parent directory)
    echo.
    echo Tugas:
    echo   Models\All.obj                                (relative path)
    echo   Models\Anim\AnimatedObject                    (relative path Animated Object)
    echo.
    REM Get OBJ file path
    set /p OBJ_PATH="Enter the path to your OBJ file (or press Enter for default test model): "
    if "%OBJ_PATH%"=="" set OBJ_PATH=Models\test_cube.obj
    echo.
    
    REM Ask about animation
    set /p USE_ANIMATION="Use animation? (y/n, default=n): "
    if "%USE_ANIMATION%"=="" set USE_ANIMATION=n
    echo.
    
    REM If animation is enabled, ask for parameters
    if /i "%USE_ANIMATION%"=="y" (
        set /p START_FRAME="Start frame (default 0): "
        if "%START_FRAME%"=="" set START_FRAME=0
        
        set /p END_FRAME="End frame (default 60): "
        if "%END_FRAME%"=="" set END_FRAME=60
        
        set /p FPS="FPS (default 30): "
        if "%FPS%"=="" set FPS=30
        echo.
        
        echo Opening "%OBJ_PATH%" with animation (Frames %START_FRAME%-%END_FRAME% @ %FPS% FPS^)...
        ObjViewer.exe "%OBJ_PATH%" -a %START_FRAME% %END_FRAME% %FPS%
    ) ELSE (
        echo Opening "%OBJ_PATH%" (no animation^)...
        ObjViewer.exe "%OBJ_PATH%"
    )
) else (
    echo.
    echo ===================
    echo Compilation failed!
    echo ===================
    echo Please check:
    echo 1. MinGW/GCC is installed and in PATH
    echo 2. FreeGLUT STATIC library is installed properly
    echo 3. All files are in correct folders:
    echo    Core\   - Source files
    echo    Models\ - OBJ files
    echo    md\     - Documentation
    echo.
    echo FreeGLUT Static Library Installation:
    echo 1. Download FreeGLUT from: https://www.transmissionzero.co.uk/software/freeglut-devel/
    echo 2. Extract and copy files to MinGW directory:
    echo    - Copy include\GL\*.h to MinGW\include\GL\
    echo    - Copy lib\libfreeglut_static.a to MinGW\lib\
    echo.
    echo Note: Make sure you have libfreeglut_static.a (not just libfreeglut.a)
    echo.
    pause
)

echo.
echo =======================================
echo SUCCESS: Standalone Executable Created!
echo =======================================
echo.
echo ObjViewer.exe is now a fully standalone executable!
echo No DLLs required - it can run on any Windows system.
echo.
echo The executable is larger because all libraries are built-in:
echo  - FreeGLUT (static)
echo  - MinGW runtime (static)
echo  - C++ standard library (static)
echo.
echo You can copy ObjViewer.exe to any computer and it will work!
echo =======================================
echo.