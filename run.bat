call .\build.bat
for /f %%A in ('echo prompt $E ^| cmd') do set "\\033=%%A"

if %ERRORLEVEL% NEQ 0 (
    echo %\\033%[38;2;255;80;80m[ERROR] Build failed, skipping execution.%\\033%[0m
    exit /b %ERRORLEVEL%
)
echo %\\033%[38;2;232;75;133m[INFO] Running the executable...%\\033%[38;2;117;181;255m
.\.build\.executables\Neoluma.exe check --project TestProject/TestProject.nlp

if %ERRORLEVEL% EQU 0 (
    echo %\\033%[38;2;75;255;75m[SUCCESS] Execution completed successfully with exit code: %ERRORLEVEL%.%\\033%[0m
) else (
    echo %\\033%[38;2;255;75;75m[ERROR] Execution failed with exit code: %ERRORLEVEL%.%\\033%[0m
)

exit /b %ERRORLEVEL%