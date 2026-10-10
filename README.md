# ConVERT 1.0

Windows 64-bit VST3 utility for FL Studio Mixer slots, plus a standalone application. Paste one YouTube video URL, select MP3 bitrate, choose an output folder, and convert. Drag the completed MP3 card into the Playlist. Existing mixer audio passes through unchanged; downloading and conversion run on a background thread. No other FL plug-in, Python installation, FFmpeg installation, or Deno installation is needed **after installing the complete generated installer**. It bundles these support tools automatically at build time.

## Get the Windows installer on GitHub

1. Create a repository named **ConVERT**. Private is fine.
2. Extract `ConVERT-source.zip`. Open the extracted **ConVERT** folder.
3. Upload its contents to the repository root: `CMakeLists.txt`, `Source`, `Tests`, `scripts`, `installer`, `.github`, and the documentation. Do not upload the ZIP itself, and do not put everything inside a second `ConVERT` folder. On Windows, enable **View → Show → Hidden items** if `.github` is not visible.
4. Commit to `main`. Open **Actions → Build ConVERT for Windows**. It should run automatically; alternatively select **Run workflow**.
5. Open the successful run and download **ConVERT-Windows-x64-Installer** under **Artifacts**. Extract that artifact ZIP and run **ConVERT-Setup-Windows-x64.exe**. Close FL Studio first. The installer is unsigned; check its source/origin rather than turning off antivirus protections.
6. In FL Studio open **Options → Manage plugins → Find installed plugins**. ConVERT installs to `C:\Program Files\Common Files\VST3\ConVERT.vst3` (system drive may vary). Open a Mixer insert and select ConVERT in an effect slot. If missing, add the Common Files VST3 folder to the search paths and scan again.
7. Paste a YouTube video link, pick the MP3 bitrate, and press **Convert to MP3**. When ready, drag the lime MP3 card into the Playlist. Or use **Open folder** and drag the file from Explorer.

## Controls and behavior

- Watch, youtu.be, Shorts, live-video replay, and embed links are normalized to a single video. Playlist/channel bulk downloading and active live streams are deliberately excluded.
- Output is real MP3 encoded by FFmpeg: 128/192/256/320 kbps. Higher bitrate cannot restore quality missing from the source.
- The default folder is Music/ConVERT. A video ID suffix avoids title collisions; existing files are not overwritten. Re-converting the same video will reuse its existing MP3 rather than regenerate at a different quality; use a new output folder if you want another version.
- Progress measures download progress; encoding is a separate stage and may take longer. Error details are shown, not silently swallowed. Internet is required. No conversion runs automatically just because a project loads.
- URL, folder, and bitrate are saved in the FL project. Closing the editor keeps the job running; unloading the plugin cancels it. Cancel stops the Windows child-process job including FFmpeg. Partial downloads can remain in the destination folder for a retry; no unrelated files are deleted.
- Windows long-path/host drag-and-drop behavior depends on the host. External file drag is implemented through JUCE. Do not run FL as administrator if Explorer runs without elevation, as Windows can prevent cross-elevation drops.
- YouTube can reject downloads, require sign-in, rate-limit, or change its extraction behavior. This version does not import browser cookies, bypass DRM, or promise every video will download. Re-run the GitHub build to package current upstream engines if extraction breaks.
- Only download material you own or have permission to download. Check the source's terms. This is not affiliated with YouTube or Image-Line.

## Build and validation

JUCE is pinned to 8.0.6. C++17, CMake 3.22+, Windows x64 MSVC. `cmake -S . -B build -A x64 -DCONVERT_NATIVE_TESTS=ON`, `cmake --build build --config Release`, `ctest --test-dir build -C Release --output-on-failure`. `scripts/fetch-tools.ps1` fetches official engine releases, checks yt-dlp hashes and GitHub release asset digests where available, and records exact versions and SHA-256 hashes in the installed `tools/versions.json`. Never manually run user text through a shell.

This delivery is **source code and an automatic build workflow**, not a prebuilt Windows installer. Local Linux checks passed for URL handling, full native C++ compilation, host-state restoration, bit-identical audio pass-through, background process output parsing, failure handling, and cancellation with a controlled test downloader. FFmpeg also produced and identified an actual 192 kbps MP3 from a generated test tone. Mock downloader tests do not verify live YouTube extraction. The Windows workflow compiles the actual VST3/application/installer and runs native tests. Live YouTube extraction and FL Studio drag/drop have not been verified in Windows here. Review the workflow log if upstream releases or downloads change.

Support tools are not other audio plugins. They are separate executable processes installed with ConVERT. Tools live under Program Files/ConVERT/tools. For development only, `CONVERT_TOOLS_DIR` can override that directory. No credentials, payment, cloud server, or background auto-update service is used.
