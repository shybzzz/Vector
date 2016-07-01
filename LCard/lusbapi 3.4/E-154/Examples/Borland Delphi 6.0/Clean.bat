echo off
@echo ##### Delete unused files #####
del *.bak /s
del *.~* /s
@if exist *.obj del *.obj /s
@if exist *.obr del *.obr /s
@if exist *.res del *.res /s
@if exist *.rws del *.rws /s
@if exist *.csm del *.csm /s
@if exist *.ilc del *.ilc /s
@if exist *.ild del *.ild /s
@if exist *.ilf del *.ilf /s
@if exist *.ils del *.ils /s
@if exist *.mbt del *.mbt /s
@if exist *.mrt del *.mrt /s
@if exist *.r$p del *.r$p /s
@if exist *.tds del *.tds /s
del *.dat /s

@echo ##### for VxD #####
@if exist *.map del *.map /s
@if exist *.pel del *.pel /s

@echo ##### for Delphi ####
del *.dcu /s

@echo ##### Work complete #####
