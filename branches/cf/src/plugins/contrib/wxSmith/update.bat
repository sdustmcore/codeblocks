@echo off
cls
zip ..\..\..\devel\share\CodeBlocks\wxsmith.zip manifest.xml *.xrc
zip ..\..\..\output\share\CodeBlocks\wxsmith.zip manifest.xml *.xrc
md ..\..\..\devel\share\CodeBlocks\images\wxsmith > nul 2>&1
md ..\..\..\output\share\CodeBlocks\images\wxsmith > nul 2>&1
copy smithicons\*.png ..\..\..\devel\share\CodeBlocks\images\wxsmith\ > nul 2>&1
copy smithicons\*.png ..\..\..\output\share\CodeBlocks\images\wxsmith\ > nul 2>&1