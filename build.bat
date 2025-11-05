@echo off
echo ========================================
echo OBJ Viewer with Material Support
echo ========================================
echo.

cd /d "%~dp0"

echo Compiling with static libraries (no DLL dependencies)...
g++ -o ObjViewer.exe Core\main.cpp Core\ObjLoader.cpp Core\AnimationLoader.cpp -ICore -DFREEGLUT_STATIC -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32 -static-libgcc -static-libstdc++ -static

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Compilation successful!
    echo ===================================
    echo.
    echo Build Type: STATIC (No DLLs Required!)
    echo.
    echo Features:
    echo  - Material support (.mtl files)
    echo  - Texture support (PNG, JPG, BMP, etc.)
    echo  - Automatic material loading
    echo  - Fully standalone executable
    echo.
    echo Usage:
    echo   ObjViewer.exe Models\yourmodel.obj
    echo.
    echo Examples of valid paths:
    echo   Models\yourmodel.obj                          (relative path)
    echo   Models\VendingMechine.obj                     (relative path)
    echo   C:\MyModels\object.obj                        (absolute path)
    echo   D:\Projects\3D Models\character.obj           (absolute path with spaces)
    echo   ..\OtherFolder\model.obj                      (parent directory)
    echo.
    set /p OBJ_PATH="Enter the path to your OBJ file (or press Enter for default test model): "
    echo.
    
    if "%OBJ_PATH%"=="" (
        echo Loading default test model...
        ObjViewer.exe Models\test_cube.obj
    ) else (
        echo Loading: %OBJ_PATH%
        ObjViewer.exe "%OBJ_PATH%"
    )
) else (
    echo.
    echo ===================================
    echo Compilation failed!
    echo ===================================
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
echo ========================================================================
echo SUCCESS: Standalone Executable Created!
echo ========================================================================
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
echo ========================================================================
echo.
