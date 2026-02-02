@echo off
set JLINK_VM_OPTIONS=
set DIR=%~dp0
"%DIR%\java" %JLINK_VM_OPTIONS% -m pong.main/com.almasb.fxglgames.pong.PongApp %*
