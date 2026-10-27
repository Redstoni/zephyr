Activate zephyr terminal/environment
& "$HOME\zephyrproject\.venv\Scripts\Activate.ps1"

Build project
west build -p always -b m5stack_fire/esp32/procpu -d build/task1 assignments/task1-shell

Set workspace:
$env:ZEPHYR_BASE = "$HOME\zephyrproject\zephyr"
$env:Path += ";C:\Program Files\CMake\bin"
Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Filter ninja.exe -Recurse -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName