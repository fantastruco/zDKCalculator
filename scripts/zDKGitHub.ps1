function Get-zDKGitHubHeaders {
    $zDKToken = $env:GH_TOKEN
    if (-not $zDKToken) { $zDKToken = $env:GITHUB_TOKEN }
    if (-not $zDKToken) {
        $zDKCredentialLines = "protocol=https`nhost=github.com`nusername=fantastruco`n`n" | git credential fill
        if ($LASTEXITCODE -ne 0) { throw 'Sign in to GitHub through Git Credential Manager before publishing.' }
        foreach ($zDKCredentialLine in $zDKCredentialLines) {
            if ($zDKCredentialLine.StartsWith('password=')) { $zDKToken = $zDKCredentialLine.Substring(9) }
        }
        $zDKCredentialLines = $null
    }
    if (-not $zDKToken) { throw 'No GitHub publishing credential is available.' }
    return @{ Authorization = "Bearer $zDKToken"; Accept = 'application/vnd.github+json'; 'X-GitHub-Api-Version' = '2022-11-28' }
}

function Invoke-zDKGitHub {
    param([string]$zDKUri, [hashtable]$zDKHeaders, [string]$zDKMethod = 'Get', $zDKBody)
    $zDKRequest = @{ Uri = $zDKUri; Headers = $zDKHeaders; Method = $zDKMethod; ErrorAction = 'Stop' }
    if ($null -ne $zDKBody) {
        $zDKRequest.Body = $zDKBody | ConvertTo-Json -Depth 12
        $zDKRequest.ContentType = 'application/json; charset=utf-8'
    }
    Invoke-RestMethod @zDKRequest
}
