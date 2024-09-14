@echo off

cd /D D:\Builds\LeagueController
call install.bat
explorer /e,D:\Builds\LeagueController\Install
xcopy "Install/settings.json" "R:/Install/settings.json" /E /Y
