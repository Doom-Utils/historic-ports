@echo off
d:
cd\games\project
copy dosdoom.exe backup.exe
copy c:\files\project\djgpp\obj\dosdoom.exe *.* /y
call doom.bat
c:

