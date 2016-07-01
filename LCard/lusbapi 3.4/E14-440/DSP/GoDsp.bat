@ECHO OFF

rem Building architecture file

rem compiling internal memory segment
asm21 inttable -2181
asm21 e440 -2181
if errorlevel==1 goto ERROR

rem linking all in exe file
ld21 inttable e440  -a e440 -g -e e440 -l
if errorlevel==1 goto ERROR

rem creating .bio file
bin3pci e440.exe
if errorlevel==1 goto ERROR

rem copying file
rem copy e440.bio ..\..\test\*.*
rem copy e440.bio ..\..\test1\*.*
rem copy e440.bio ..\..\test2\*.*
rem copy e440.bio ..\..\dll\*.*
rem copy e440.bio ..\..\e440test\*.*

if exist *.cde del *.cde
if exist *.int del *.int
if exist *.bak del *.bak
if exist *.obj del *.obj
if exist e440.sym del e440.sym
if exist inttable.sym del inttable.sym
if exist e440.exe del e440.exe
if exist inttable.exe del inttable.exe

ECHO -
ECHO #####################################################################
ECHO   	      S U C C E S S ! ! !  Файл E440.bio сформирован
ECHO #####################################################################
ECHO -
goto END

:ERROR
ECHO -
ECHO #####################################################################
ECHO          E R R O R ! ! !  Ошибка формирования файла E440.bio
ECHO #####################################################################
ECHO - 

:END
