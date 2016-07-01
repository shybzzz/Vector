echo off
@echo ##### Delete unused files #####
del *.bak /s
del *.obj /s
del *.~* /s
del *.obr /s
rem del *.res /s
del *.rws /s
del *.csm /s
del *.ilc /s
del *.ild /s
del *.ilf /s
del *.ils /s
del *.mbt /s
del *.mrt /s
del *.r$p /s
del *.tds /s
del *.dat /s

@echo ##### for Delphi ####
del *.dcu /s

@echo ##### Work complete #####
