:: Server Release
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\AccountServer\AccountServer.exe" "C:\Users\amin\Documents\talesofpirates\Server\AccountServer\Bin\AccountServer.exe"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\AccountServer\AccountServer.pdb" "C:\Users\amin\Documents\talesofpirates\Server\AccountServer\Bin\AccountServer.pdb"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GroupServer\GroupServer.exe" "C:\Users\amin\Documents\talesofpirates\Server\GroupServer\Bin\GroupServer.exe"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GroupServer\GroupServer.pdb" "C:\Users\amin\Documents\talesofpirates\Server\GroupServer\Bin\GroupServer.pdb"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GateServer\GateServer.exe" "C:\Users\amin\Documents\talesofpirates\Server\GateServer\Bin\GateServer.exe"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GateServer\GateServer.pdb" "C:\Users\amin\Documents\talesofpirates\Server\GateServer\Bin\GateServer.pdb"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GameServer\GameServer.exe" "C:\Users\amin\Documents\talesofpirates\Server\GameServer\Bin\GameServer.exe"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\server\GameServer\GameServer.pdb" "C:\Users\amin\Documents\talesofpirates\Server\GameServer\Bin\GameServer.pdb"

:: Client Release
call :MKLINK "C:\Users\amin\Desktop\Top Project File\client\system\Game.exe" "C:\Users\amin\Documents\talesofpirates\Client\bin\system\Game.exe"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\client\system\Game.pdb" "C:\Users\amin\Documents\talesofpirates\Client\bin\system\Game.pdb"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\client\system\MindPower3D_D8R.dll" "C:\Users\amin\Documents\talesofpirates\Engine\lib\MindPower3D_D8R.dll"
call :MKLINK "C:\Users\amin\Desktop\Top Project File\client\system\MindPower3D_D8R.pdb" "C:\Users\amin\Documents\talesofpirates\Engine\lib\MindPower3D_D8R.pdb"

pause

:MKLINK
if exist "%~1" del /F /Q "%~1"
mklink "%~1" "%~2"
EXIT/B 0