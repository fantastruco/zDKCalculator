param(
    [Parameter(Mandatory)][ValidatePattern('^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$')][string]$zDKVersion,
    [Parameter(Mandatory)][string]$zDKNotesFile
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'zDKGitHub.ps1')
$zDKRoot = Split-Path $PSScriptRoot -Parent
$zDKVersionText = [IO.File]::ReadAllText((Join-Path $zDKRoot 'src/zDKVersion.h'))
if (-not $zDKVersionText.Contains('zDKAppVersion[] = "' + $zDKVersion + '"')) { throw 'Tag and application versions do not match.' }
$zDKNotes = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $zDKNotesFile))
$zDKHeaders = Get-zDKGitHubHeaders
$zDKReleaseApi = 'https://api.github.com/repos/fantastruco/zDKCalculator/releases'
$zDKTag = "v$zDKVersion"
try {
    $zDKRelease = $null
    try { $zDKRelease = Invoke-zDKGitHub "$zDKReleaseApi/tags/$zDKTag" $zDKHeaders }
    catch { if ([int]$_.Exception.Response.StatusCode -ne 404) { throw } }
    if ($zDKRelease -and -not $zDKRelease.draft) {
        Write-Host "Already published: $($zDKRelease.html_url)"
        return
    }
    if (-not $zDKRelease) {
        $zDKRelease = Invoke-zDKGitHub $zDKReleaseApi $zDKHeaders 'Post' @{
            tag_name=$zDKTag; name="Dokkan Defense Calculator $zDKVersion"; body=$zDKNotes; draft=$true; prerelease=$false
        }
    }
    foreach ($zDKName in @('DokkanDefenseCalculator.exe','DokkanDefenseCalculator-Windows-x64.zip','SHA256SUMS.txt')) {
        $zDKFile = Join-Path $zDKRoot ('dist\' + $zDKName)
        $zDKDigest = 'sha256:' + (Get-FileHash -LiteralPath $zDKFile -Algorithm SHA256).Hash.ToLowerInvariant()
        $zDKAsset = $zDKRelease.assets | Where-Object name -eq $zDKName
        if ($zDKAsset -and $zDKAsset.digest -eq $zDKDigest) { continue }
        if ($zDKAsset) { Invoke-zDKGitHub $zDKAsset.url $zDKHeaders 'Delete' | Out-Null }
        $zDKUploadUrl = $zDKRelease.upload_url.Split('{')[0] + '?name=' + [Uri]::EscapeDataString($zDKName)
        $zDKUploaded = Invoke-RestMethod -Uri $zDKUploadUrl -Headers $zDKHeaders -Method Post -InFile $zDKFile -ContentType 'application/octet-stream'
        if ($zDKUploaded.digest -ne $zDKDigest) { throw "GitHub digest verification failed for $zDKName." }
    }
    $zDKPublished = Invoke-zDKGitHub "$zDKReleaseApi/$($zDKRelease.id)" $zDKHeaders 'Patch' @{
        draft=$false; body=$zDKNotes; make_latest='true'
    }
    Write-Host "Published: $($zDKPublished.html_url)"
} finally { $zDKHeaders.Clear() }
