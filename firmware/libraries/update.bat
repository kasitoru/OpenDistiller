@REM    OpenDistiller Project
@REM    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
@REM    URL: https://github.com/kasitoru/OpenDistiller

@ECHO off
CHCP 65001 > nul

ECHO Обновление библиотек...
git pull --recurse-submodules
git submodule update --remote --recursive

pause
