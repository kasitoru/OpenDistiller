@REM    OpenDistiller Project
@REM    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
@REM    URL: https://github.com/kasitoru/OpenDistiller

CHCP 65001 > nul

CALL libraries/update.bat
CALL build.bat
CALL flash.bat
