param(
    [Parameter(Mandatory)][Alias('Version')][ValidatePattern('^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$')][string]$zDKVersion,
    [Parameter(Mandatory)][Alias('NotesFile')][string]$zDKNotesFile
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'zDKGitHub.ps1')
$zDKRoot = Split-Path $PSScriptRoot -Parent
$zDKNotes = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $zDKNotesFile))
if (-not $zDKNotes.Trim()) { throw 'Release notes must not be empty.' }
$zDKHeaders = Get-zDKGitHubHeaders
$zDKRepository = 'fantastruco/zDKCalculator'
$zDKTag = "v$zDKVersion"
Push-Location $zDKRoot
try {
    if ((git branch --show-current) -ne 'main') { throw 'Publish from the main branch.' }
    if (git status --porcelain) { throw 'Commit source changes before publishing.' }
    if ((git remote get-url origin) -ne "https://github.com/$zDKRepository.git") { throw 'Unexpected source remote.' }
    $zDKRepositoryInfo = Invoke-zDKGitHub "https://api.github.com/repos/$zDKRepository" $zDKHeaders
    if ($zDKRepositoryInfo.private) { throw 'The repository must be public for anonymous application updates.' }
    $zDKVersionSource = [IO.File]::ReadAllText((Join-Path $zDKRoot 'src/zDKVersion.h'))
    if ($zDKVersionSource -notmatch 'zDKAppVersion\[\] = "([0-9]+\.[0-9]+\.[0-9]+)"') { throw 'Application version was not found.' }
    if ([version]$zDKVersion -lt [version]$Matches[1]) { throw 'A release cannot lower the application version.' }
    $zDKRelease = $null
    try { $zDKRelease = Invoke-zDKGitHub "https://api.github.com/repos/$zDKRepository/releases/tags/$zDKTag" $zDKHeaders }
    catch { if ([int]$_.Exception.Response.StatusCode -ne 404) { throw } }
    if ($zDKRelease -and -not $zDKRelease.draft) { throw 'This version is already published. Choose a new version.' }
    $zDKVersionFiles = @('src/zDKVersion.h','src/zDKResources.rc','src/zDKWindows.manifest','CMakeLists.txt')
    foreach ($zDKFile in $zDKVersionFiles) {
        $zDKPath = Join-Path $zDKRoot $zDKFile
        $zDKText = [IO.File]::ReadAllText($zDKPath)
        $zDKText = [regex]::Replace($zDKText,'\b[0-9]+\.[0-9]+\.[0-9]+(?=\.0|"| LANGUAGES)', $zDKVersion)
        $zDKText = [regex]::Replace($zDKText,'(?<=VERSION )[0-9]+,[0-9]+,[0-9]+,0', ($zDKVersion.Replace('.',',') + ',0'))
        [IO.File]::WriteAllText($zDKPath, $zDKText)
    }
    $zDKNotesTarget = "docs/releases/$zDKVersion.md"
    New-Item -ItemType Directory -Force -Path (Split-Path $zDKNotesTarget -Parent) | Out-Null
    [IO.File]::WriteAllText((Join-Path $zDKRoot $zDKNotesTarget),$zDKNotes)
    git add -- @zDKVersionFiles $zDKNotesTarget
    if ($LASTEXITCODE -ne 0) { throw 'Unable to stage version resources.' }
    git diff --cached --quiet
    if ($LASTEXITCODE -eq 1) {
        git commit -m "Release $zDKTag"
        if ($LASTEXITCODE -ne 0) { throw 'Version commit failed.' }
    }
    $zDKExistingTag = git tag --list $zDKTag
    if ($zDKExistingTag) {
        if ((git rev-list -n 1 $zDKTag) -ne (git rev-parse HEAD)) { throw 'Existing tag points to a different commit.' }
    } else {
        git tag -a $zDKTag -m "Dokkan Defense Calculator $zDKVersion"
        if ($LASTEXITCODE -ne 0) { throw 'Tag creation failed.' }
    }
    $zDKRemoteTag = git ls-remote origin "refs/tags/$zDKTag"
    git push -u origin main
    if ($LASTEXITCODE -ne 0) { throw 'Source push failed.' }
    git push origin $zDKTag
    if ($LASTEXITCODE -ne 0) { throw 'Tag push failed.' }
    if ($zDKRemoteTag) {
        Invoke-zDKGitHub "https://api.github.com/repos/$zDKRepository/actions/workflows/windows.yml/dispatches" $zDKHeaders 'Post' @{ref=$zDKTag} | Out-Null
    }
    Write-Host "Build and release publication queued: https://github.com/$zDKRepository/actions"
} finally {
    Pop-Location
    $zDKHeaders.Clear()
}
