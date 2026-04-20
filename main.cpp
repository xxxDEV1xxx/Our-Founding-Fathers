/*
 * ============================================================================
 *  Fireside Reader
 *  Copyright (C) 2025  Christopher T. Williams  â€” All Rights Reserved
 *  "Fireside Reader" is a trademark (TM) of Christopher T. Williams.
 *
 *  This file is part of Fireside Reader.
 *
 *  Fireside Reader is distributed subject to the GNU General Public Licence
 *  v3.0 or later solely because it is linked against eSpeak-NG (GPL-3.0+).
 *  See the LICENSE file in the project root for full terms, third-party
 *  dependency notices, and the MBROLA non-commercial voice licence notice.
 *
 *  Third-party components used by this file:
 *    â€¢ eSpeak-NG  â€” GPL-3.0-or-later   github.com/espeak-ng/espeak-ng
 *    â€¢ MBROLA voices (en1/en2) â€” Non-commercial only  github.com/numediart/MBROLA-voices
 *    â€¢ SDL2 / SDL2_ttf / SDL2_mixer â€” zlib licence    libsdl.org
 *    â€¢ Tesseract OCR  â€” Apache-2.0    github.com/tesseract-ocr/tesseract
 *    â€¢ Leptonica      â€” BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#include "app.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int /*argc*/, char* /*argv*/[])
{
    // Locate the data/authors directory relative to the executable.
    // CMake copies it next to the binary; fall back to CWD.
    std::string dataRoot = "data/authors";
    if (!fs::exists(dataRoot)) {
        std::cerr << "[main] data/authors not found at '" << dataRoot
                  << "' â€“ running with empty repository.\n";
    }

    App app;
    if (!app.init(dataRoot)) {
        std::cerr << "[main] App::init() failed â€“ exiting.\n";
        return 1;
    }

    app.run();
    return 0;
}
