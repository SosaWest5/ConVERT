param([string]$Destination = "$PSScriptRoot\..\tools")
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force $Destination | Out-Null
$Destination = (Resolve-Path $Destination).Path
$headers = @{ 'User-Agent'='ConVERT-build'; 'Accept'='application/vnd.github+json' }
function Release($repo) { Invoke-RestMethod "https://api.github.com/repos/$repo/releases/latest" -Headers $headers }
function Download($url,$path) { Invoke-WebRequest $url -OutFile $path -Headers @{ 'User-Agent'='ConVERT-build' } }
$yt = Release 'yt-dlp/yt-dlp'
$binary = $yt.assets | Where-Object name -eq 'yt-dlp.exe' | Select-Object -First 1
$sums = $yt.assets | Where-Object name -eq 'SHA2-256SUMS' | Select-Object -First 1
if (!$binary -or !$sums) { throw 'Official yt-dlp binary/checksums are unavailable.' }
Download $binary.browser_download_url "$Destination\yt-dlp.exe"
Download $sums.browser_download_url "$Destination\yt-dlp-checksums.txt"
$line = Get-Content "$Destination\yt-dlp-checksums.txt" | Where-Object { $_ -match '\s+\*?yt-dlp\.exe$' } | Select-Object -First 1
$expected = ($line -split '\s+')[0]
if (!$expected -or (Get-FileHash "$Destination\yt-dlp.exe" -Algorithm SHA256).Hash -ne $expected) { throw 'yt-dlp checksum mismatch.' }
$deno = Release 'denoland/deno'
$d = $deno.assets | Where-Object name -eq 'deno-x86_64-pc-windows-msvc.zip' | Select-Object -First 1
if (!$d) { throw 'Official Deno Windows x64 binary is unavailable.' }
Download $d.browser_download_url "$Destination\deno.zip"
Expand-Archive "$Destination\deno.zip" "$Destination\deno-extracted" -Force
Copy-Item "$Destination\deno-extracted\deno.exe" "$Destination\deno.exe"
$ff = Release 'yt-dlp/FFmpeg-Builds'
$f = $ff.assets | Where-Object { $_.name -match '^ffmpeg.*win64.*gpl.*\.zip$' -and $_.name -notmatch 'shared|lgpl' } | Sort-Object name | Select-Object -First 1
if (!$f) { throw 'Official FFmpeg Windows x64 ZIP is unavailable. Inspect current release asset names.' }
Download $f.browser_download_url "$Destination\ffmpeg.zip"
$ffSums = $ff.assets | Where-Object name -eq 'checksums.sha256' | Select-Object -First 1
if (!$ffSums) { throw 'FFmpeg checksums are unavailable.' }
Download $ffSums.browser_download_url "$Destination\ffmpeg-checksums.txt"
$ffLine = Get-Content "$Destination\ffmpeg-checksums.txt" | Where-Object { ($_ -split '\s+')[-1].TrimStart('*') -eq $f.name } | Select-Object -First 1
$ffHash = ($ffLine -split '\s+')[0]
if (!$ffHash -or (Get-FileHash "$Destination\ffmpeg.zip" -Algorithm SHA256).Hash -ne $ffHash) { throw 'FFmpeg checksum mismatch.' }
Expand-Archive "$Destination\ffmpeg.zip" "$Destination\ffmpeg-extracted" -Force
foreach ($exe in @('ffmpeg.exe','ffprobe.exe')) { $found=Get-ChildItem "$Destination\ffmpeg-extracted" -Recurse -Filter $exe | Select-Object -First 1; if (!$found) {throw "$exe not found"}; Copy-Item $found.FullName "$Destination\$exe" }
# Retain release digest metadata where upstream provides it. Record exact upstream versions and hashes.
foreach ($item in @(@{asset=$d;path="$Destination\deno.zip"},@{asset=$f;path="$Destination\ffmpeg.zip"})) {
 if ($item.asset.digest -and $item.asset.digest.StartsWith('sha256:')) { $actual=(Get-FileHash $item.path -Algorithm SHA256).Hash.ToLower(); if ($actual -ne $item.asset.digest.Substring(7).ToLower()) { throw 'Upstream asset digest mismatch.' } }
}
@{yt_dlp=$yt.tag_name;deno=$deno.tag_name;ffmpeg=$ff.tag_name;ffmpeg_asset=$f.name;yt_dlp_source="https://github.com/yt-dlp/yt-dlp/tree/$($yt.tag_name)";deno_source="https://github.com/denoland/deno/tree/$($deno.tag_name)";ffmpeg_build_source="https://github.com/yt-dlp/FFmpeg-Builds/tree/$($ff.tag_name)";hashes=(Get-ChildItem "$Destination\*.exe" | ForEach-Object { @{name=$_.Name;sha256=(Get-FileHash $_.FullName -Algorithm SHA256).Hash} })} | ConvertTo-Json -Depth 5 | Set-Content "$Destination\versions.json" -Encoding utf8
& "$Destination\yt-dlp.exe" --version
& "$Destination\ffmpeg.exe" -version | Select-Object -First 4 | Set-Content "$Destination\ffmpeg-version.txt"
& "$Destination\deno.exe" --version
# Ship upstream notices, including FFmpeg's bundled license notices and source/build references.
New-Item -ItemType Directory -Force "$Destination\licenses" | Out-Null
Get-ChildItem "$Destination\ffmpeg-extracted" -Recurse -File | Where-Object { $_.Name -match '^(LICENSE|COPYING|README)' } | ForEach-Object {Copy-Item $_.FullName "$Destination\licenses\ffmpeg-$($_.Name)" -Force}
Download "https://raw.githubusercontent.com/yt-dlp/yt-dlp/$($yt.tag_name)/THIRD_PARTY_LICENSES.txt" "$Destination\licenses\yt-dlp-third-party.txt"
Download "https://raw.githubusercontent.com/yt-dlp/yt-dlp/$($yt.tag_name)/LICENSE" "$Destination\licenses\yt-dlp-license.txt"
Download "https://raw.githubusercontent.com/denoland/deno/$($deno.tag_name)/LICENSE.md" "$Destination\licenses\deno-license.md"
Download 'https://raw.githubusercontent.com/juce-framework/JUCE/8.0.6/LICENSE.md' "$Destination\licenses\juce-license.md"
Remove-Item "$Destination\deno.zip","$Destination\ffmpeg.zip","$Destination\deno-extracted","$Destination\ffmpeg-extracted" -Recurse -Force
