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
â”œâ”€â”€ CMakeLists.txt
â”œâ”€â”€ README.md
â”œâ”€â”€ assets/
â”‚   â”œâ”€â”€ OldEnglish.ttf        â† add manually (free OFL font)
â”‚   â””â”€â”€ fire_loop.ogg/.wav    â† add manually (CC0 audio)
â”œâ”€â”€ data/
â”‚   â””â”€â”€ authors/
â”‚       â”œâ”€â”€ George Washington/
â”‚       â”‚   â”œâ”€â”€ Farewell Address 1796.txt
â”‚       â”‚   â””â”€â”€ First Inaugural Address 1789.txt
â”‚       â”œâ”€â”€ Thomas Jefferson/
â”‚       â”‚   â””â”€â”€ Declaration of Independence 1776.txt
â”‚       â””â”€â”€ ...               â† populated by scripts/fetch_texts.py
â”œâ”€â”€ scripts/
â”‚   â””â”€â”€ fetch_texts.py
â””â”€â”€ src/
    â”œâ”€â”€ main.cpp
    â”œâ”€â”€ app.hpp / app.cpp       â† main application + event loop
    â”œâ”€â”€ tts.hpp / tts.cpp       â† eSpeak-NG text-to-speech engine
    â”œâ”€â”€ ocr.hpp / ocr.cpp       â† Tesseract OCR for image files
    â”œâ”€â”€ repository.hpp / .cpp   â† author/book/page file system
    â”œâ”€â”€ fire.hpp / fire.cpp     â† 8-bit Doom-style fire simulation
    â””â”€â”€ woodui.hpp / .cpp       â† wood-panel 8-bit UI drawing helpers
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
| `REV` button    | Press repeatedly for x1â†’x3â†’x5â†’x10â†’x20 rewind |
| `FWD` button    | Same, forward                                 |
| Click fire area | Stop seek; resume reading from current word   |
| Any key         | Same                                          |
| `PrvPg/NxtPg`  | Jump to previous/next 76-line page            |
| `PrvBk/NxtBk`  | Jump to previous/next book by same author     |
| `PrvAuth/NxtAuth` | Jump to previous/next author folder         |
| `PLAY`          | Play / Pause toggle                           |
| `T-` / `T+`     | TTS volume âˆ’5 / +5 %                          |
| `A-` / `A+`     | Ambient fire volume âˆ’5 / +5 %                 |
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
