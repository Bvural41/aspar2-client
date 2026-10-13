@echo off
title Mobil Ikon Uretici
cd /d "%~dp0"
echo ========================================================
echo   Metin2 Mobil Ikon Uretici (Android & iOS)
echo ========================================================
echo.
echo Kaynak: android_ios\common\branding\icon_1024.png
echo.
python generate_icons.py
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [HATA] Ikonlar uretilemedi! Python veya Pillow yuklu oldugundan emin olun.
    pause
    exit /b %ERRORLEVEL%
)
echo.
pause
