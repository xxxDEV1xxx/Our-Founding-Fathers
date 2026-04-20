
/*
 * ============================================================================
 *  Fireside Reader
 *  Copyright (C) 2025  Christopher T. Williams  All Rights Reserved
 *
 *  This file is part of Fireside Reader.
 *
 *  Fireside Reader is distributed subject to the GNU General Public Licence
 *  v3.0 or later solely because it is linked against eSpeak-NG (GPL-3.0+).
 *  See the LICENSE file in the project root for full terms, third-party
 *  dependency notices, and the MBROLA non-commercial voice licence notice.
 *
 *  Third-party components used by this file:
 *   eSpeak-NG  GPL-3.0-or-later   github.com/espeak-ng/espeak-ng
 *   MBROLA voices (en1/en2) Non-commercial only  github.com/numediart/MBROLA-voices
 *   SDL2 / SDL2_ttf / SDL2_mixer zlib licence    libsdl.org
 *   Tesseract OCR   Apache-2.0    github.com/tesseract-ocr/tesseract
 *   Leptonica     BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#pragma once
#include <string>

// Wraps Tesseract OCR for image-based text extraction.
// Text files bypass this entirely the Repository class decides.
class OcrEngine {
public:
    OcrEngine();
    ~OcrEngine();

    // dataPath: directory containing tessdata/ (empty = use env TESSDATA_PREFIX)
    // lang    : Tesseract language code, default "eng"
    bool init(const std::string& dataPath = "", const std::string& lang = "eng");

    // Returns extracted UTF-8 text, or empty string on failure.
    std::string extractText(const std::string& imagePath);

    bool isInitialized() const { return initialized_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    bool  initialized_ = false;
};
