# Fireside Reader
**Designed by: Christopher Williams ~**

An 8-bit styled text-to-speech reader with a flickering log fire, old-English
voices, and a central repository of Founding Fathers speeches and documents.

---

## Dependencies

### Linux (Debian/Ubuntu)
```bash
sudo apt install \
    libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
    espeak-ng libespeak-ng-dev \
    tesseract-ocr libtesseract-dev libleptonica-dev \
    python3-pip cmake build-essential
```

### Windows (vcpkg)
```powershell
vcpkg install sdl2 sdl2-ttf sdl2-mixer espeak-ng tesseract
```
Add the vcpkg toolchain file to your CMake call:
```
-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## Build

```bash
# 1. Clone / unzip the project
cd FiresideReader

# 2. Fetch the Founding Fathers text repository (one-time)
pip3 install requests beautifulsoup4 lxml
python3 scripts/fetch_texts.py

# 3. Add a font
#    Copy any Old-English style .ttf into  assets/OldEnglish.ttf
#    Suggested free font: "MedievalSharp" or "UnifrakturMaguntia"
#    from Google Fonts (OFL licence).

# 4. Add ambient fire audio
#    Place a looping fire ambience as  assets/fire_loop.ogg  or .wav
#    Free sources:  freesound.org  (search "fireplace loop" â€“ CC0)

# 5. CMake configure & build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 6. Run
./build/FiresideReader
```

---

## Project Layout

```
FiresideReader/
 CMakeLists.txt
README.md
assets/
OldEnglish.ttf   add manually (free OFL font)
fire_loop.ogg/.wav  add manually (CC0 audio)
data/
authors/
George Washington/
Farewell Address 1796.txt
First Inaugural Address 1789.txt
Thomas Jefferson/
Declaration of Independence 1776.txt
populated by scripts/fetch_texts.py
scripts/
fetch_texts.py
src/
main.cpp
app.hpp / app.cpp  main application + event loop
    tts.hpp / tts.cpp eSpeak-NG text-to-speech engine
ocr.hpp / ocr.cpp  Tesseract OCR for image files
    repository.hpp / .cpp author/book/page file system
fire.hpp / fire.cpp 8-bit Doom-style fire simulation
    woodui.hpp / .cpp wood-panel 8-bit UI drawing helpers
```

---

## Voices

eSpeak-NG ships several English variants. The code defaults to **en-rp**
(Received Pronunciation â€“ closest to classical/old-English delivery).
Other classical options:

| eSpeak voice name | Character          |
|-------------------|--------------------|
| `en-rp`           | Received Pronunc.  |
| `en-gb`           | British English    |
| `en-wm`           | West Midlands      |
| `en-sc`           | Scottish           |

To install extra eSpeak voice data:
```bash
sudo apt install espeak-ng-data
```

---

## Controls (Fire Screen)

| Control         | Action                                        |
|-----------------|-----------------------------------------------|
| `REV` button    | Press repeatedly for x1’x3’x5’x10’x20 rewind |
| `FWD` button    | Same, forward                                 |
| Click fire area | Stop seek; resume reading from current word   |
| Any key         | Same                                          |
| `PrvPg/NxtPg`  | Jump to previous/next 76-line page            |
| `PrvBk/NxtBk`  | Jump to previous/next book by same author     |
| `PrvAuth/NxtAuth` | Jump to previous/next author folder         |
| `PLAY`          | Play / Pause toggle                           |
| `T-` / `T+`     | TTS volume ’5 / +5 %                          |
| `A-` / `A+`     | Ambient fire volume ’5 / +5 %                 |
| `ESC`           | Return to author selection menu               |

---

## Tooltip Easter Egg

Hover your mouse over the **hottest point** of the fire (centre-bottom of
the flame, Â¾ down the firebox).  Within a 1-inch radius a translucent
tooltip appears:

> *Designed by: Christopher Williams ~*

in old-English 8-bit lettering, with the fire flickering through the
75 % translucent white background.

---

## Licence

All fetched texts are public domain (pre-1927 U.S. government documents).
eSpeak-NG is LGPL-3.0.  SDL2 / SDL2_ttf / SDL2_mixer are zlib.
Tesseract is Apache-2.0.  This application code is MIT.
