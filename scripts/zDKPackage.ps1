param([string]$zDKVersion)
$ErrorActionPreference = 'Stop'
$zDKRoot = Split-Path $PSScriptRoot -Parent
if (-not $zDKVersion) {
    $zDKVersionText = [IO.File]::ReadAllText((Join-Path $zDKRoot 'src\zDKVersion.h'))
    if ($zDKVersionText -notmatch 'zDKAppVersion\[\] = "([0-9]+\.[0-9]+\.[0-9]+)"') { throw 'Application version was not found.' }
    $zDKVersion = $Matches[1]
}
$zDKStage = Join-Path $zDKRoot ('build\packages\' + $zDKVersion + '-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $zDKStage | Out-Null
$zDKFiles = @('dist\DokkanDefenseCalculator.exe', 'README.md')
foreach ($zDKFile in $zDKFiles) { Copy-Item -LiteralPath (Join-Path $zDKRoot $zDKFile) -Destination $zDKStage }
Copy-Item -LiteralPath (Join-Path $zDKRoot 'third_party\imgui\LICENSE.txt') -Destination (Join-Path $zDKStage 'Dear-ImGui-LICENSE.txt')
Copy-Item -LiteralPath (Join-Path $zDKRoot 'third_party\json\LICENSE.MIT') -Destination (Join-Path $zDKStage 'nlohmann-json-LICENSE.txt')
$zDKDocs = Join-Path $zDKStage 'docs'
New-Item -ItemType Directory -Path $zDKDocs | Out-Null
foreach ($zDKDoc in @('zDKDefenseResearch.md','zDKRoundingResearch.md','zDKSupportEffects.md','zDKUpdates.md','zDKTurnDefense.md')) {
    Copy-Item -LiteralPath (Join-Path $zDKRoot ('docs\' + $zDKDoc)) -Destination $zDKDocs
}
$zDKZip = Join-Path $zDKRoot 'dist\DokkanDefenseCalculator-Windows-x64.zip'
Compress-Archive -Path (Join-Path $zDKStage '*') -DestinationPath $zDKZip -Force
$zDKChecksums = foreach ($zDKArtifact in @((Join-Path $zDKRoot 'dist\DokkanDefenseCalculator.exe'), $zDKZip)) {
    (Get-FileHash -LiteralPath $zDKArtifact -Algorithm SHA256).Hash.ToLowerInvariant() + '  ' + (Split-Path $zDKArtifact -Leaf)
}
$zDKChecksums | Set-Content -LiteralPath (Join-Path $zDKRoot 'dist\SHA256SUMS.txt') -Encoding ascii
Write-Host "Packaged version $zDKVersion"
