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
 *    Leptonica  BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#pragma once
#include <SDL.h>
#include <vector>
#include <cstdint>
#include <random>

// 8-bit "Doom fire" algorithm
// Produces a convincing flickering log-fire pixel simulation.
// The fire texture is FIRE_W x FIRE_H pixels; caller blits it into any rect.
class FireSim {
public:
    static constexpr int FIRE_W = 120;
    static constexpr int FIRE_H = 80;

    FireSim();

    // Must be called once after SDL_Init.
    bool init(SDL_Renderer* renderer);

    // Advance one animation frame.
    void update();

    // Blit the fire into 'destRect' on the given renderer.
    void render(SDL_Renderer* renderer, const SDL_Rect& destRect);

    // Hot-spot is at the vertical centre of the bottom third of destRect.
    static SDL_Point hotspot(const SDL_Rect& destRect);

    ~FireSim();

private:
    std::vector<uint8_t>  pixels_;          // fire intensity 0-36
    std::vector<SDL_Color> palette_;        // 37-entry colour map
    SDL_Texture*           texture_ = nullptr;
    std::mt19937           rng_;

    void buildPalette();
    void seed();
    void spreadFire(int src);
};
