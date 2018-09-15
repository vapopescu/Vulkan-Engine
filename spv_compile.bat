@echo off

::ECHO "%~dp0build\data\shaders"
IF NOT EXIST "%~dp0build\data\shaders" MD "%~dp0build\data\shaders"

FOR %%f IN ("*.vert" "*.tesc" "*.tese" "*.geom" "*.frag" "*.comp") DO (
	IF EXIST %~dp0build\data\shaders\%%~nxf.spv DEL %~dp0build\data\shaders\%%~nxf.spv
	::ECHO %~dp0build\data\shaders\%%~nxf.spv
	glslangvalidator -V -o build\data\shaders\%%~nxf.spv %%~nxf
	IF NOT EXIST %~dp0build\data\shaders\%%~nxf.spv EXIT /B 2
)

EXIT /B %ERRORLEVEL%
