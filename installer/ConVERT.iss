#define AppVersion "1.0.0"
[Setup]
AppId={{D7C2BE02-93E6-4597-979A-56D15CE60CDD}
AppName=ConVERT
AppVersion={#AppVersion}
AppPublisher=SosaWest
DefaultDirName={autopf}\ConVERT
DefaultGroupName=ConVERT
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=ConVERT-Setup-Windows-x64
Compression=lzma2/fast
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayIcon={app}\ConVERT.exe
CloseApplications=yes
[Files]
Source: "..\build\ConVERT_artefacts\Release\VST3\ConVERT.vst3\*"; DestDir: "{commoncf64}\VST3\ConVERT.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\ConVERT_artefacts\Release\Standalone\ConVERT.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\tools\*"; DestDir: "{app}\tools"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\THIRD-PARTY-NOTICES.txt"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\ConVERT"; Filename: "{app}\ConVERT.exe"
Name: "{group}\Uninstall ConVERT"; Filename: "{uninstallexe}"
