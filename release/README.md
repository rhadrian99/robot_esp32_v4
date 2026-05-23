# Release Firmware Builds

This folder contains compiled firmware binaries ready for OTA deployment.

Each build is automatically copied here with version suffix:
- `firmware61.bin` - version 6.1
- `firmware62.bin` - version 6.2
- etc.

These files can be uploaded to the ESP32 via the web interface at `/settings` page using the **UPLOAD FIRMWARE** button.

## Naming Convention

- Dots are stripped from version: `6.1` → `61`, `6.1.5` → `615`
- Format: `firmware{VERSION_WITHOUT_DOTS}.bin`

## Usage for OTA

1. Build project: `pio run`
2. Firmware automatically copied to `release/firmware{VERSION}.bin`
3. Upload to device via web interface or programmatically
