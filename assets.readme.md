# assets/

Place the following files here before building:

## OldEnglish.ttf   (required)
Any Old-English / blackletter TrueType font with an OFL or public-domain licence.

Recommended free options:
- **UnifrakturMaguntia** â€” https://fonts.google.com/specimen/UnifrakturMaguntia
- **MedievalSharp**      â€” https://openfontlibrary.org/en/font/medievalsharp
- **GoudyTextMT**        â€” available on many font archive sites

Rename the downloaded .ttf to `OldEnglish.ttf`.

## fire_loop.ogg  *or*  fire_loop.wav   (required)
A looping fireplace / crackling fire ambience sound.

Recommended free CC0 sources:
- https://freesound.org/search/?q=fireplace+loop&filter=license%3ACC0
  (search "fireplace loop" filtered to CC0)
- https://freesound.org/people/inchadney/sounds/135419/   (CC0 campfire)

Download the file and place it as `assets/fire_loop.ogg` (preferred for
smaller file size) or `assets/fire_loop.wav`.

The application tries OGG first and silently falls back to WAV if OGG
support is not compiled into SDL2_mixer.
