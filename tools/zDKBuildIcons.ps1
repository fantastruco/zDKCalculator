param(
    [string]$zDKOriginalPath = (Join-Path $PSScriptRoot '..\asset\icon.png'),
    [string]$zDKMasterPath = (Join-Path $PSScriptRoot '..\asset\icons\zDKIconMaster.png'),
    [string]$zDKOutputDirectory = (Join-Path $PSScriptRoot '..\asset\icons'),
    [string]$zDKIconPath = (Join-Path $PSScriptRoot '..\asset\icon.ico')
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$zDKSizes = @(16,20,24,28,30,32,36,40,42,48,56,60,64,72,80,84,96,112,120,128,144,160,168,192,224,240,256,512,1024)
$zDKOriginal = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $zDKOriginalPath).Path)
$zDKMaster = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $zDKMasterPath).Path)
try {
    if ($zDKMaster.Width -ne $zDKMaster.Height -or $zDKMaster.Width -lt 1024) {
        throw 'The icon master must be square and at least 1024 pixels wide.'
    }
    New-Item -ItemType Directory -Path $zDKOutputDirectory -Force | Out-Null
    $zDKFrames = [System.Collections.Generic.List[object]]::new()
    foreach ($zDKSize in $zDKSizes) {
        # Keep the supplied drawing exact at small sizes; use the restored master for large renditions.
        $zDKSource = if ($zDKSize -le $zDKOriginal.Width) { $zDKOriginal } else { $zDKMaster }
        $zDKBitmap = [System.Drawing.Bitmap]::new($zDKSize,$zDKSize,[System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $zDKGraphics = [System.Drawing.Graphics]::FromImage($zDKBitmap)
            $zDKAttributes = [System.Drawing.Imaging.ImageAttributes]::new()
            try {
                $zDKGraphics.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
                $zDKGraphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                $zDKGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                $zDKGraphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                $zDKAttributes.SetWrapMode([System.Drawing.Drawing2D.WrapMode]::TileFlipXY)
                $zDKGraphics.DrawImage($zDKSource,[System.Drawing.Rectangle]::new(0,0,$zDKSize,$zDKSize),
                    0,0,$zDKSource.Width,$zDKSource.Height,[System.Drawing.GraphicsUnit]::Pixel,$zDKAttributes)
            } finally { $zDKGraphics.Dispose(); $zDKAttributes.Dispose() }
            $zDKPath = Join-Path $zDKOutputDirectory ('zDKIcon-{0}.png' -f $zDKSize)
            $zDKBitmap.Save($zDKPath,[System.Drawing.Imaging.ImageFormat]::Png)
            if ($zDKSize -le 256) {
                $zDKFrames.Add(@{zDKSize=$zDKSize;zDKBytes=[System.IO.File]::ReadAllBytes($zDKPath)})
            }
        } finally { $zDKBitmap.Dispose() }
    }
    $zDKStream = [System.IO.File]::Create($zDKIconPath)
    $zDKWriter = [System.IO.BinaryWriter]::new($zDKStream)
    try {
        $zDKWriter.Write([uint16]0)
        $zDKWriter.Write([uint16]1)
        $zDKWriter.Write([uint16]$zDKFrames.Count)
        $zDKOffset = 6 + 16 * $zDKFrames.Count
        foreach ($zDKFrame in $zDKFrames) {
            $zDKDimension = if ($zDKFrame.zDKSize -eq 256) {0} else {$zDKFrame.zDKSize}
            $zDKWriter.Write([byte]$zDKDimension)
            $zDKWriter.Write([byte]$zDKDimension)
            $zDKWriter.Write([uint16]0)
            $zDKWriter.Write([uint16]1)
            $zDKWriter.Write([uint16]32)
            $zDKWriter.Write([uint32]$zDKFrame.zDKBytes.Length)
            $zDKWriter.Write([uint32]$zDKOffset)
            $zDKOffset += $zDKFrame.zDKBytes.Length
        }
        foreach ($zDKFrame in $zDKFrames) { $zDKWriter.Write([byte[]]$zDKFrame.zDKBytes) }
    } finally { $zDKWriter.Dispose(); $zDKStream.Dispose() }
    Write-Host ('Created {0} PNG sizes and {1} ICO frames: {2}' -f $zDKSizes.Count,$zDKFrames.Count,$zDKIconPath)
} finally { $zDKOriginal.Dispose(); $zDKMaster.Dispose() }
