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
#include "fire.hpp"
#include <algorithm>
#include <cstring>

// â”€â”€ Doom-style fire palette (black â†’ red â†’ orange â†’ yellow â†’ white) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void FireSim::buildPalette()
{
    // 37 entries (index 0 = cold/black, 36 = hottest/white)
    palette_.resize(37);
    for (int i = 0; i < 37; ++i) {
        float t = static_cast<float>(i) / 36.f;
        SDL_Color c{};
        if (t < 0.33f) {
            float s = t / 0.33f;
            c.r = static_cast<uint8_t>(s * 180);
            c.g = 0;
            c.b = 0;
        } else if (t < 0.66f) {
            float s = (t - 0.33f) / 0.33f;
            c.r = static_cast<uint8_t>(180 + s * 75);
            c.g = static_cast<uint8_t>(s * 120);
            c.b = 0;
        } else {
            float s = (t - 0.66f) / 0.34f;
            c.r = 255;
            c.g = static_cast<uint8_t>(120 + s * 135);
            c.b = static_cast<uint8_t>(s * 200);
        }
        c.a = 255;
        palette_[i] = c;
    }
    palette_[0] = {0,0,0,255};   // Ensure cold = pure black
}

// â”€â”€ Constructor â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
FireSim::FireSim()
    : pixels_(FIRE_W * FIRE_H, 0),
      rng_(std::random_device{}())
{
    buildPalette();
}

FireSim::~FireSim()
{
    if (texture_) SDL_DestroyTexture(texture_);
}

// â”€â”€ init â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool FireSim::init(SDL_Renderer* renderer)
{
    texture_ = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 FIRE_W, FIRE_H);
    if (!texture_) return false;
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    seed();
    return true;
}

// â”€â”€ seed: ignite the bottom row â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void FireSim::seed()
{
    int row = FIRE_H - 1;
    for (int x = 0; x < FIRE_W; ++x)
        pixels_[row * FIRE_W + x] = 36;    // maximum heat
}

// â”€â”€ spreadFire (single pixel) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void FireSim::spreadFire(int src)
{
    std::uniform_int_distribution<int> jitter(0, 3);
    int rand  = jitter(rng_);
    int dst   = src - FIRE_W - rand + 1;    // propagate upward
    if (dst < 0) return;
    int decay = rand & 1;
    pixels_[dst] = static_cast<uint8_t>(
        std::max(0, static_cast<int>(pixels_[src]) - decay));
}

// â”€â”€ update â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void FireSim::update()
{
    // Re-seed the bottom row with random intensity for a realistic flicker
    std::uniform_int_distribution<int> noise(28, 36);
    int row = FIRE_H - 1;
    for (int x = 0; x < FIRE_W; ++x)
        pixels_[row * FIRE_W + x] = static_cast<uint8_t>(noise(rng_));

    for (int y = 1; y < FIRE_H; ++y)
        for (int x = 0; x < FIRE_W; ++x)
            spreadFire(y * FIRE_W + x);
}

// â”€â”€ render â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void FireSim::render(SDL_Renderer* renderer, const SDL_Rect& destRect)
{
    void* pixels;
    int pitch;
    SDL_LockTexture(texture_, nullptr, &pixels, &pitch);
    auto* px = static_cast<uint32_t*>(pixels);

    for (int y = 0; y < FIRE_H; ++y) {
        for (int x = 0; x < FIRE_W; ++x) {
            uint8_t idx = pixels_[y * FIRE_W + x];
            SDL_Color c  = palette_[idx];
            // SDL RGBA8888 = R<<24 | G<<16 | B<<8 | A
            px[y * (pitch/4) + x] =
                (static_cast<uint32_t>(c.r) << 24) |
                (static_cast<uint32_t>(c.g) << 16) |
                (static_cast<uint32_t>(c.b) <<  8) |
                (static_cast<uint32_t>(c.a));
        }
    }
    SDL_UnlockTexture(texture_);
    SDL_RenderCopy(renderer, texture_, nullptr, &destRect);
}

// â”€â”€ hotspot â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
SDL_Point FireSim::hotspot(const SDL_Rect& destRect)
{
    return { destRect.x + destRect.w / 2,
             destRect.y + destRect.h * 3 / 4 };
}
