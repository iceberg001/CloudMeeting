@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build

if not exist %BUILD_DIR%\CMakeCache.txt (
    echo [CloudMeeting] First-time configure...
    if not exist %BUILD_DIR% mkdir %BUILD_DIR%
    cd %BUILD_DIR%
    cmake .. -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        echo [CloudMeeting] CMake configuration failed!
        cd ..
        exit /b 1
    )
) else (
    cd %BUILD_DIR%
    echo [CloudMeeting] Reusing existing configure (incremental build)
)

echo [CloudMeeting] Building CloudMeeting only (DuiLib links prebuilt lib)...
cmake --build . --config Release --target CloudMeeting
if errorlevel 1 (
    echo [CloudMeeting] Build failed!
    cd ..
    exit /b 1
)

echo [CloudMeeting] Build succeeded!
echo [CloudMeeting] Output: %BUILD_DIR%\Release\CloudMeeting.exe
cd ..

endlocal
