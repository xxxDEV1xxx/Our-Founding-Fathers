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
#include "ocr.hpp"
#include <iostream>

#if __has_include(<tesseract/baseapi.h>)
#  include <tesseract/baseapi.h>
#  include <leptonica/allheaders.h>
#  define HAVE_TESSERACT 1
#else
#  define HAVE_TESSERACT 0
#endif

// â”€â”€ Private implementation â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
struct OcrEngine::Impl {
#if HAVE_TESSERACT
    tesseract::TessBaseAPI api;
#endif
};

OcrEngine::OcrEngine()  : impl_(new Impl) {}
OcrEngine::~OcrEngine() {
#if HAVE_TESSERACT
    impl_->api.End();
#endif
    delete impl_;
}

bool OcrEngine::init(const std::string& dataPath, const std::string& lang)
{
#if HAVE_TESSERACT
    const char* dp = dataPath.empty() ? nullptr : dataPath.c_str();
    if (impl_->api.Init(dp, lang.c_str())) {
        std::cerr << "[OCR] Tesseract Init failed\n";
        return false;
    }
    initialized_ = true;
    return true;
#else
    std::cerr << "[OCR] Tesseract not compiled in â€“ OCR unavailable\n";
    return false;
#endif
}

std::string OcrEngine::extractText(const std::string& imagePath)
{
#if HAVE_TESSERACT
    if (!initialized_) return {};

    Pix* image = pixRead(imagePath.c_str());
    if (!image) {
        std::cerr << "[OCR] Cannot read image: " << imagePath << "\n";
        return {};
    }

    impl_->api.SetImage(image);
    char* raw = impl_->api.GetUTF8Text();
    std::string result = raw ? raw : "";
    delete[] raw;
    pixDestroy(&image);
    return result;
#else
    (void)imagePath;
    return {};
#endif
}
