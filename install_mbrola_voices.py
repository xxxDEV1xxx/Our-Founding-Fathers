# =============================================================================
#  Fireside Reader â€” install_mbrola_voices.py
#  Copyright (C) 2025  Christopher T. Williams  â€” All Rights Reserved
#  "Fireside Reader" is a trademark (TM) of Christopher T. Williams.
#
#  Distributed under GPL-3.0-or-later (eSpeak-NG linkage requirement).
#  See LICENSE for full terms and third-party dependency notices.
#
#  Designed and authored by: Christopher T. Williams
# =============================================================================
#!/usr/bin/env python3
"""
install_mbrola_voices.py
========================
Downloads the MBROLA en1 and en2 English diphone voices and installs them
so eSpeak-NG can use them as the "Orator" and "Rector" voice profiles.

MBROLA voices are free for non-commercial use.
Source: https://github.com/numediart/MBROLA-voices

Run from the project root:
    python3 scripts/install_mbrola_voices.py
"""

import os
import sys
import pathlib
import tarfile
import zipfile
import subprocess
import platform

try:
    import requests
except ImportError:
    sys.exit("Install deps:  pip install requests")

# â”€â”€ MBROLA voice files (GitHub release, free for non-commercial use) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
VOICES = [
    {
        "name"    : "en1",
        "url"     : "https://github.com/numediart/MBROLA-voices/raw/master/data/en1/en1",
        "desc"    : "British English male 1 â€“ deep, authoritative",
    },
    {
        "name"    : "en2",
        "url"     : "https://github.com/numediart/MBROLA-voices/raw/master/data/en2/en2",
        "desc"    : "British English male 2 â€“ lighter, scholarly",
    },
]

HEADERS = {"User-Agent": "FiresideReader/1.0 (MBROLA voice installer)"}

def mbrola_dir() -> pathlib.Path:
    """Return the directory where eSpeak-NG expects MBROLA voices."""
    sys_name = platform.system()

    # Check common Linux paths first
    candidates = [
        pathlib.Path("/usr/share/mbrola"),
        pathlib.Path("/usr/local/share/mbrola"),
    ]

    # On Windows, try to find the eSpeak-NG installation
    if sys_name == "Windows":
        program_files = os.environ.get("ProgramFiles", r"C:\Program Files")
        candidates = [
            pathlib.Path(program_files) / "eSpeak NG" / "espeak-ng-data" / "mbrola",
            pathlib.Path(program_files) / "eSpeak"   / "espeak-data"    / "mbrola",
        ] + candidates

    for path in candidates:
        if path.exists():
            return path

    # Fall back: create under /usr/share/mbrola (Linux) or beside the exe
    fallback = pathlib.Path("/usr/share/mbrola")
    if sys_name != "Windows":
        return fallback

    # Windows: put it next to the built executable
    return pathlib.Path("build") / "mbrola"


def install_voice(voice: dict, dest_dir: pathlib.Path) -> None:
    name = voice["name"]
    url  = voice["url"]
    dest = dest_dir / name

    if dest.exists():
        print(f"  âœ“  {name} already installed at {dest}")
        return

    dest_dir.mkdir(parents=True, exist_ok=True)

    print(f"  â†“  Downloading {name}  ({voice['desc']}) â€¦")
    r = requests.get(url, headers=HEADERS, timeout=30, stream=True)
    if r.status_code != 200:
        print(f"  âœ—  HTTP {r.status_code} for {url}")
        return

    total = int(r.headers.get("content-length", 0))
    downloaded = 0
    with open(dest, "wb") as f:
        for chunk in r.iter_content(chunk_size=65536):
            f.write(chunk)
            downloaded += len(chunk)
            if total:
                pct = downloaded * 100 // total
                print(f"\r     {pct:3d}%", end="", flush=True)
    print(f"\r  âœ“  {name} saved to {dest}  ({downloaded:,} bytes)")


def check_mbrola_binary() -> bool:
    """Check if the mbrola binary is on PATH (needed for audio output)."""
    try:
        subprocess.run(["mbrola", "--help"],
                       capture_output=True, timeout=5)
        return True
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return False


def main():
    print("Fireside Reader â€” MBROLA voice installer")
    print("=" * 44)

    dest = mbrola_dir()
    print(f"MBROLA voice directory: {dest}\n")

    # Install voice data files
    for v in VOICES:
        install_voice(v, dest)

    print()

    # Check mbrola binary
    if not check_mbrola_binary():
        sys_name = platform.system()
        print("âš   The 'mbrola' binary was not found on your PATH.")
        print("   eSpeak-NG uses MBROLA as a subprocess for diphone synthesis.")
        print()
        if sys_name == "Linux":
            print("   Install it with:")
            print("     sudo apt install mbrola")
            print("   or:")
            print("     sudo dnf install mbrola")
        elif sys_name == "Windows":
            print("   Download MbrolaTools from:")
            print("   https://github.com/numediart/MBROLA/releases")
            print("   and add its directory to your PATH.")
        print()
        print("   Without mbrola, Fireside Reader will automatically fall back")
        print("   to the built-in eSpeak-NG formant voices (Elder / Statesman).")
    else:
        print("âœ“  mbrola binary found â€“ MBROLA voices fully operational.")

    print()
    print("Done.  Restart Fireside Reader to use the new voices.")
    print("Tip:   Use the 'Voice' button in the fire screen to cycle profiles.")


if __name__ == "__main__":
    main()
