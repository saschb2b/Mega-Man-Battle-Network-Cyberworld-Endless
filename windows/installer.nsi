; The Windows installer (build.py windows, NSIS): for the user who runs it,
; no administrator, into %LOCALAPPDATA%\Programs\Cyberworld Endless, with a
; Start menu entry, a desktop shortcut if wanted, and an uninstaller in
; Settings > Apps. The saves stay in %LOCALAPPDATA%\cyberworld-endless.
; build.py passes VERSION, FILEVERSION (four numbers), STAGE (the files) and OUT.
Unicode true
!include "MUI2.nsh"

!define NAME "Cyberworld Endless"
!define ID "io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless"
!define UNINST "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ID}"
!define EXE "cyberworld-endless.exe"

Name "${NAME}"
OutFile "${OUT}"
InstallDir "$LOCALAPPDATA\Programs\${NAME}"
InstallDirRegKey HKCU "Software\${ID}" "InstallDir"
RequestExecutionLevel user
SetCompressor /SOLID lzma
BrandingText "${NAME} ${VERSION}"

VIProductVersion "${FILEVERSION}"
VIFileVersion "${FILEVERSION}"
VIAddVersionKey "ProductName" "${NAME}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "FileDescription" "${NAME} setup"
VIAddVersionKey "CompanyName" "Sascha Becker"
VIAddVersionKey "LegalCopyright" "MIT license. Mega Man Battle Network is (c) Capcom; unofficial, not affiliated."

!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TEXT "A roguelike on Mega Man Battle Network 6.$\r$\n$\r$\nIt runs your own copy of Mega Man Battle Network 6: Cybeast Gregar (USA), an unmodified .gba file; no game data is included. The first start asks for the file.$\r$\n$\r$\nClick Next to continue."
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${NAME}"
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Add a shortcut to the desktop"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION DesktopShortcut

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section
	SetOutPath "$INSTDIR"
	File "${STAGE}\${EXE}"
	File "${STAGE}\README.txt"
	File "${STAGE}\LICENSE.txt"
	SetOutPath "$INSTDIR\licenses"
	File "${STAGE}\licenses\*.txt"
	SetOutPath "$INSTDIR"
	CreateShortcut "$SMPROGRAMS\${NAME}.lnk" "$INSTDIR\${EXE}"
	WriteUninstaller "$INSTDIR\uninstall.exe"
	WriteRegStr HKCU "Software\${ID}" "InstallDir" "$INSTDIR"
	WriteRegStr HKCU "${UNINST}" "DisplayName" "${NAME}"
	WriteRegStr HKCU "${UNINST}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKCU "${UNINST}" "Publisher" "Sascha Becker"
	WriteRegStr HKCU "${UNINST}" "DisplayIcon" "$INSTDIR\${EXE}"
	WriteRegStr HKCU "${UNINST}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKCU "${UNINST}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr HKCU "${UNINST}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
	WriteRegStr HKCU "${UNINST}" "URLInfoAbout" "https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/"
	WriteRegDWORD HKCU "${UNINST}" "NoModify" 1
	WriteRegDWORD HKCU "${UNINST}" "NoRepair" 1
SectionEnd

Function DesktopShortcut
	CreateShortcut "$DESKTOP\${NAME}.lnk" "$INSTDIR\${EXE}"
FunctionEnd

; (only what it installed: the folder may have been chosen by hand)
Section "Uninstall"
	Delete "$INSTDIR\${EXE}"
	Delete "$INSTDIR\README.txt"
	Delete "$INSTDIR\LICENSE.txt"
	Delete "$INSTDIR\licenses\*.txt"
	RMDir "$INSTDIR\licenses"
	Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"
	Delete "$SMPROGRAMS\${NAME}.lnk"
	Delete "$DESKTOP\${NAME}.lnk"
	DeleteRegKey HKCU "${UNINST}"
	DeleteRegKey HKCU "Software\${ID}"
SectionEnd
