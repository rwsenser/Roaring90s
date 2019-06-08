@echo off

rem set ICUHOME=c:\InternalProductsMisc-Nobackup\FromJohnH\TrueChange\ICU\v1.8.1\icu
set ICUHOME=c:\InternalProductsMisc-Nobackup\ICU\BuildAtSag20020227

echo ICUHOME is
echo %ICUHOME%
pause

set ICU_DATA=%ICUHOME%\source\data\
set ICU_BIN=%ICUHOME%\bin\
set ICU_INC=%ICUHOME%\include
set ICU_LIB=%ICUHOME%\lib\
set TZ=PST8PDT
set PATH=%ICU_BIN%;%PATH%

echo done

@echo on
