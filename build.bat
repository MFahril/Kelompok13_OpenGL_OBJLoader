@echo off
echo ========================================
echo OBJ Viewer with Material Support
echo ========================================
echo.

cd /d "%~dp0"

echo Compiling from organized structure...
g++ -o ObjViewer.exe Core\main.cpp Core\ObjLoader.cpp -ICore -lopengl32 -lglu32 -lfreeglut -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Compilation successful!
    echo ===================================
    echo.
    echo Features:
    echo  - Material support (.mtl files)
    echo  - Texture support (PNG, JPG, BMP, etc.)
    echo  - Automatic material loading
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
    echo 1. MinGW/GCC is installed
    echo 2. FreeGLUT is installed
    echo 3. All files are in correct folders:
    echo    Core\   - Source files
    echo    Models\ - OBJ files
    echo    md\     - Documentation
    echo.
    pause
)
