@echo off
md ..\..\..\devel\share\CodeBlocks\images\ThreadSearch > nul 2>&1
md ..\..\..\output\share\CodeBlocks\images\ThreadSearch > nul 2>&1
md ..\..\..\devel\share\CodeBlocks\images\settings > nul 2>&1
md ..\..\..\output\share\CodeBlocks\images\settings > nul 2>&1
copy .\resources\images\*.png ..\..\..\devel\share\CodeBlocks\images\ThreadSearch\ > nul 2>&1
copy .\resources\images\*.png ..\..\..\output\share\CodeBlocks\images\ThreadSearch\ > nul 2>&1
copy .\resources\*.png ..\..\..\devel\share\CodeBlocks\images\settings\ > nul 2>&1
copy .\resources\*.png ..\..\..\output\share\CodeBlocks\images\settings\ > nul 2>&1
exit 0

