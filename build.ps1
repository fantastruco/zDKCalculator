param([Alias('Run')][switch]$zDKRun)
$ErrorActionPreference = 'Stop'
$zDKProjectRoot = $PSScriptRoot
$zDKVSWhereExecutable = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $zDKVSWhereExecutable)) { throw 'Install Visual Studio Build Tools with Desktop development with C++ and Windows SDK.' }
$zDKVisualStudioPath = & $zDKVSWhereExecutable -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $zDKVisualStudioPath) { throw 'Visual Studio C++ compiler was not found.' }
$zDKDeveloperShell = Join-Path $zDKVisualStudioPath 'Common7\Tools\Launch-VsDevShell.ps1'
& $zDKDeveloperShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation | Out-Null
Push-Location $zDKProjectRoot
try {
    New-Item -ItemType Directory -Force -Path build,dist | Out-Null
    & rc.exe /nologo /fo build\resources.res /i src src\zDKResources.rc
    if ($LASTEXITCODE -ne 0) { throw 'Resource compilation failed.' }
    $zDKSourceFiles = @('src\zDKWindows.cpp','third_party\imgui\imgui.cpp','third_party\imgui\imgui_draw.cpp','third_party\imgui\imgui_tables.cpp','third_party\imgui\imgui_widgets.cpp','third_party\imgui\backends\imgui_impl_win32.cpp','third_party\imgui\backends\imgui_impl_dx11.cpp')
    & cl.exe /nologo /std:c++17 /O2 /MT /EHsc /utf-8 /W4 /DUNICODE /D_UNICODE /Ithird_party\imgui /Ithird_party\imgui\backends @zDKSourceFiles /Fobuild\ /Fedist\DokkanDefenseCalculator.exe /link /SUBSYSTEM:WINDOWS /MANIFEST:NO build\resources.res d3d11.lib dxgi.lib d3dcompiler.lib dwmapi.lib windowscodecs.lib ole32.lib shell32.lib comdlg32.lib user32.lib gdi32.lib imm32.lib winhttp.lib bcrypt.lib
    if ($LASTEXITCODE -ne 0) { throw 'Application compilation failed.' }
    & cl.exe /nologo /std:c++17 /O2 /MT /EHsc /utf-8 /W4 /Isrc tests\zDKCalculatorTests.cpp /Fobuild\zDKCalculatorTests.obj /Febuild\zDKCalculatorTests.exe
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    & .\build\zDKCalculatorTests.exe
    if ($LASTEXITCODE -ne 0) { throw 'Calculator tests failed.' }
    & cl.exe /nologo /std:c++17 /O2 /MT /EHsc /utf-8 /W4 /DUNICODE /D_UNICODE /Isrc tests\zDKUpdaterTests.cpp /Fobuild\zDKUpdaterTests.obj /Febuild\zDKUpdaterTests.exe /link winhttp.lib bcrypt.lib shell32.lib ole32.lib user32.lib
    if ($LASTEXITCODE -ne 0) { throw 'Updater test compilation failed.' }
    & .\build\zDKUpdaterTests.exe
    if ($LASTEXITCODE -ne 0) { throw 'Updater checks failed.' }
    $zDKUIReportPath = Join-Path $zDKProjectRoot 'build\ui-smoke.log'
    $zDKUIProcess = Start-Process -FilePath (Join-Path $zDKProjectRoot 'dist\DokkanDefenseCalculator.exe') -ArgumentList @('--scale','1','--width','540','--height','980','--ui-smoke',('"' + $zDKUIReportPath + '"')) -PassThru -WindowStyle Hidden
    if (-not $zDKUIProcess.WaitForExit(60000)) { $zDKUIProcess.Kill(); throw 'Interface checks timed out.' }
    Get-Content -LiteralPath $zDKUIReportPath | Select-Object -Last 1 | Write-Host
    if ($zDKUIProcess.ExitCode -ne 0) { Get-Content -LiteralPath $zDKUIReportPath; throw 'Interface checks failed.' }
    Copy-Item -LiteralPath third_party\imgui\LICENSE.txt -Destination dist\Dear-ImGui-LICENSE.txt
    Copy-Item -LiteralPath third_party\json\LICENSE.MIT -Destination dist\nlohmann-json-LICENSE.txt
    Copy-Item -LiteralPath README.md -Destination dist
    New-Item -ItemType Directory -Force -Path dist\docs | Out-Null
    Copy-Item -LiteralPath docs\zDKCodingStandards.md,docs\zDKDefenseResearch.md,docs\zDKRoundingResearch.md,docs\zDKSupportEffects.md,docs\zDKUpdates.md,docs\zDKTurnDefense.md -Destination dist\docs
    Write-Host "Ready: $zDKProjectRoot\dist\DokkanDefenseCalculator.exe"
    if ($zDKRun) { Start-Process -FilePath "$zDKProjectRoot\dist\DokkanDefenseCalculator.exe" }
} finally { Pop-Location }
