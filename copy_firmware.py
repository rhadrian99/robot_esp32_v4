Import("env")
import shutil, os, re

def copy_firmware(source, target, env):
    firmware_src = str(target[0])
    release_dir = os.path.join(env["PROJECT_DIR"], "release")
    os.makedirs(release_dir, exist_ok=True)

    # Read FW_VERSION from robot_tenis_v7.cpp or common.h
    version = "unknown"
    main_cpp = os.path.join(env["PROJECT_DIR"], "src", "robot_tenis_v7.cpp")
    common_h = os.path.join(env["PROJECT_DIR"], "src", "common.h")
    
    try:
        # Try robot_tenis_v7.cpp first
        with open(main_cpp, "r", encoding="utf-8") as f:
            content = f.read()
            # Match: const char FW_VERSION[] = "X.X.X"; or "X.X.XS" etc
            m = re.search(r'const\s+char\s+FW_VERSION\[\]\s*=\s*"([^"]+)"', content)
            if m:
                version = m.group(1)
                print(f"[POST-BUILD] Found FW_VERSION: {version}")
            else:
                # Fallback: try old format #define FW_VERSION in robot_tenis_v7.cpp
                m = re.search(r'#define\s+FW_VERSION\s+"([^"]+)"', content)
                if m:
                    version = m.group(1)
                    print(f"[POST-BUILD] Found FW_VERSION (legacy in robot_tenis_v7.cpp): {version}")
    except Exception as e:
        print(f"[POST-BUILD] INFO reading {main_cpp}: {e}")
    
    # If not found, try common.h
    if version == "unknown":
        try:
            with open(common_h, "r", encoding="utf-8") as f:
                content = f.read()
                m = re.search(r'#define\s+FW_VERSION\s+"([^"]+)"', content)
                if m:
                    version = m.group(1)
                    print(f"[POST-BUILD] Found FW_VERSION in common.h: {version}")
                else:
                    print(f"[POST-BUILD] WARNING: Could not find FW_VERSION in {main_cpp} or {common_h}")
        except Exception as e:
            print(f"[POST-BUILD] ERROR reading {common_h}: {e}")
    
    # Strip dots: "6.1" -> "61", "6.1S" -> "61S"
    suffix = re.sub(r'\.', '', version)
    
    if os.path.exists(firmware_src):
        dst = os.path.join(release_dir, f"firmware{suffix}.bin")
        shutil.copy(firmware_src, dst)
        print(f"[POST-BUILD] Firmware copied -> release/firmware{suffix}.bin (v{version})")
    else:
        print(f"[POST-BUILD] ERROR: Source firmware not found at {firmware_src}")

env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)
