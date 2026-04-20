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
 *    eSpeak-NG  GPL-3.0-or-later   github.com/espeak-ng/espeak-ng
 *    MBROLA voices (en1/en2) Non-commercial only  github.com/numediart/MBROLA-voices
 *    SDL2 / SDL2_ttf / SDL2_mixer zlib licence    libsdl.org
 *    Tesseract OCR Apache-2.0    github.com/tesseract-ocr/tesseract
 *    Leptonica      BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

//  WoodUI 
// Stateless drawing helpers that produce the 8-bit wood-panel aesthetic.
// All calls assume the correct renderer is already set as target.
namespace WoodUI
{
    // Seed the procedural wood grain (call once).
    void initGrain(unsigned seed = 42);

    // Draw a wood-grained background filling 'rect'.
    void drawWoodPanel(SDL_Renderer* r, const SDL_Rect& rect);

    // Draw a single raised 3-D button (shadowed bevel).
    // 'pressed' inverts the bevel to give a "pushed in" look.
    void drawButton(SDL_Renderer* r, TTF_Font* font,
                    const SDL_Rect& rect, const std::string& label,
                    bool pressed = false);

    // Draw a semi-transparent tooltip box with old-English text.
    // alpha: 0-255 transparency of background (e.g. 190 for ~75 %).
    void drawTooltip(SDL_Renderer* r, TTF_Font* font,
                     int x, int y,
                     const std::string& text,
                     int alpha = 190);

    // Draw the ambient-fire "log" silhouette at the bottom of 'destRect'.
    void drawLogSilhouette(SDL_Renderer* r, const SDL_Rect& destRect);

    // Draw a volume bar label (e.g. "TTS: 45%")
    void drawLabel(SDL_Renderer* r, TTF_Font* font,
                   int x, int y, const std::string& text,
                   SDL_Color col = {220,200,160,255});
}
