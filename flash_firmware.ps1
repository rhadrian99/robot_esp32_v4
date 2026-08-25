<#
    flash_firmware.ps1
    Selecteaza si flashuieste un firmware din folderul release/ pe ESP32.

    Utilizare:
        .\flash_firmware.ps1                 # meniu interactiv (fisier + port)
        .\flash_firmware.ps1 -Port COM10     # forteaza portul
        .\flash_firmware.ps1 -File firmware66.bin -Port COM10   # fara meniu

    Imaginea aplicatiei se scrie la offset 0x10000 (bootloader-ul si tabela de
    partitii sunt deja pe placa dintr-un flash complet anterior via PlatformIO).
#>

param(
    [string]$File,
    [string]$Port,
    [int]$Baud = 460800
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$releaseDir = Join-Path $scriptDir "release"
$flashOffset = "0x10000"

# --- Localizeaza python-ul PlatformIO si esptool.py ---
$pioPython = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\python.exe"
$esptool   = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"

if (-not (Test-Path $pioPython)) { throw "Nu gasesc python-ul PlatformIO: $pioPython" }
if (-not (Test-Path $esptool))   { throw "Nu gasesc esptool.py: $esptool" }
if (-not (Test-Path $releaseDir)) { throw "Nu exista folderul release: $releaseDir" }

# --- Selecteaza fisierul firmware ---
$binFiles = @(Get-ChildItem -Path $releaseDir -Filter "*.bin" | Sort-Object Name)
if ($binFiles.Count -eq 0) { throw "Nu exista fisiere .bin in $releaseDir" }

if ($File) {
    $selected = $binFiles | Where-Object { $_.Name -eq $File }
    if (-not $selected) { throw "Fisierul '$File' nu exista in release/. Disponibile: $($binFiles.Name -join ', ')" }
} else {
    Write-Host ""
    Write-Host "Firmware disponibil in release/:" -ForegroundColor Cyan
    for ($i = 0; $i -lt $binFiles.Count; $i++) {
        $f = $binFiles[$i]
        $sizeKb = [math]::Round($f.Length / 1KB)
        Write-Host ("  [{0}] {1,-20} {2,6} KB  {3}" -f ($i + 1), $f.Name, $sizeKb, $f.LastWriteTime)
    }
    Write-Host ""
    do {
        $choice = Read-Host "Alege firmware-ul (1-$($binFiles.Count))"
    } while (-not ($choice -match '^\d+$') -or [int]$choice -lt 1 -or [int]$choice -gt $binFiles.Count)
    $selected = $binFiles[[int]$choice - 1]
}

# --- Selecteaza portul serial ---
if (-not $Port) {
    $ports = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
    if ($ports.Count -eq 0) {
        $Port = Read-Host "Nu am detectat porturi COM. Introdu manual (ex: COM10)"
    } elseif ($ports.Count -eq 1) {
        $Port = $ports[0]
        Write-Host "Port detectat automat: $Port" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "Porturi COM disponibile:" -ForegroundColor Cyan
        for ($i = 0; $i -lt $ports.Count; $i++) {
            Write-Host ("  [{0}] {1}" -f ($i + 1), $ports[$i])
        }
        do {
            $pChoice = Read-Host "Alege portul (1-$($ports.Count))"
        } while (-not ($pChoice -match '^\d+$') -or [int]$pChoice -lt 1 -or [int]$pChoice -gt $ports.Count)
        $Port = $ports[[int]$pChoice - 1]
    }
}

# --- Confirmare + flash ---
Write-Host ""
Write-Host "Flashuiesc: $($selected.Name)" -ForegroundColor Yellow
Write-Host "Port:       $Port" -ForegroundColor Yellow
Write-Host "Baud:       $Baud   Offset: $flashOffset" -ForegroundColor Yellow
Write-Host ""

& $pioPython $esptool --chip esp32 --port $Port --baud $Baud write_flash $flashOffset $selected.FullName

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Gata! $($selected.Name) flashuit cu succes pe $Port." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "Flash esuat (cod $LASTEXITCODE). Verifica portul/placa." -ForegroundColor Red
    exit $LASTEXITCODE
}
