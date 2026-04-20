/*
 * ============================================================================
 *  Fireside Reader
 *  Copyright (C) 2025  Christopher T. Williams All Rights Reserved
 *
 *  This file is part of Fireside Reader.
 *
 *  Fireside Reader is distributed subject to the GNU General Public Licence
 *  v3.0 or later solely because it is linked against eSpeak-NG (GPL-3.0+).
 *  See the LICENSE file in the project root for full terms, third-party
 *  dependency notices, and the MBROLA non-commercial voice licence notice.
 *
 *  Third-party components used by this file:
 *    eSpeak-NG  GPL-3.0-or-later   github.com/espeak-ng/espeak-ng
 *    MBROLA voices (en1/en2) Non-commercial only  github.com/numediart/MBROLA-voices
 *    SDL2 / SDL2_ttf / SDL2_mixer zlib licence    libsdl.org
 *    Tesseract OCR  Apache-2.0    github.com/tesseract-ocr/tesseract
 *     Leptonica  BSD-2-Clause  leptonica.org
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
