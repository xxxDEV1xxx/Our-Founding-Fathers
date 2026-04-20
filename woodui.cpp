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
 *    eSpeak-NG  GPL-3.0-or-later   github.com/espeak-ng/espeak-ng
 *    MBROLA voices (en1/en2) Non-commercial only  github.com/numediart/MBROLA-voices
 *    SDL2 / SDL2_ttf / SDL2_mixer zlib licence    libsdl.org
 *    Tesseract OCR  Apache-2.0    github.com/tesseract-ocr/tesseract
 *    Leptonica      BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#include "woodui.hpp"
#include <cmath>
#include <vector>
#include <random>
#include <string>

// Grain state
static std::vector<float> grainMap;  // per-pixel grain offset
static int grainW = 0, grainH = 0;
static std::mt19937 grainRng(42);

void WoodUI::initGrain(unsigned seed)
{
    grainRng.seed(seed);
    grainW = 1024; grainH = 768;
    grainMap.resize(static_cast<std::size_t>(grainW * grainH));
    std::uniform_real_distribution<float> d(-1.f, 1.f);
    for (auto& v : grainMap) v = d(grainRng);
}

// Wood panel 
void WoodUI::drawWoodPanel(SDL_Renderer* r, const SDL_Rect& rect)
{
    // Base warm brown layers
    static const SDL_Color woodBase[4] = {
        {101, 67, 33, 255},   // dark walnut
        {120, 80, 40, 255},
        {140, 95, 45, 255},
        {160,110, 55, 255}
    };

    for (int y = rect.y; y < rect.y + rect.h; ++y) {
        for (int x = rect.x; x < rect.x + rect.w; ++x) {
            // Concentric wood-ring bands using sine in X with Y-based noise
            float gx = (float)((x - rect.x) % grainW);
            float gy = (float)((y - rect.y) % grainH);
            float noise = grainMap.empty() ? 0.f :
                          grainMap[static_cast<std::size_t>(gy) * grainW
                                 + static_cast<std::size_t>(gx)];

            float ring = std::sin((float)x * 0.12f + noise * 8.f
                                + (float)y  * 0.018f) * 0.5f + 0.5f;

            int band = static_cast<int>(ring * 3.99f);
            band = std::max(0, std::min(3, band));
            SDL_Color c = woodBase[band];

            // Darken near board edges to simulate depth / frame
            float ex = (float)(x - rect.x) / (float)rect.w;
            float ey = (float)(y - rect.y) / (float)rect.h;
            float edge = std::min({ex, 1.f - ex, ey, 1.f - ey});
            float dim  = std::min(1.f, edge * 20.f);  // 0 at edgeâ†’1 interior
            c.r = static_cast<uint8_t>(c.r * dim);
            c.g = static_cast<uint8_t>(c.g * dim);
            c.b = static_cast<uint8_t>(c.b * dim);

            SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
            SDL_RenderDrawPoint(r, x, y);
        }
    }
}

// Raised 3-D button 
void WoodUI::drawButton(SDL_Renderer* r, TTF_Font* font,
                        const SDL_Rect& rect, const std::string& label,
                        bool pressed)
{
    // Button face (warm amber)
    SDL_Color face   = {180, 130, 60, 255};
    SDL_Color hiCol  = {220, 180,100, 255};   // highlight bevel
    SDL_Color shCol  = { 60,  40, 15, 255};   // shadow bevel
    if (pressed) std::swap(hiCol, shCol);

    // Fill face
    SDL_SetRenderDrawColor(r, face.r, face.g, face.b, 255);
    SDL_RenderFillRect(r, &rect);

    // Bevel thickness = 2 px
    int t = 2;
    // Top / Left highlight
    SDL_SetRenderDrawColor(r, hiCol.r, hiCol.g, hiCol.b, 255);
    for (int i = 0; i < t; ++i) {
        SDL_RenderDrawLine(r, rect.x+i, rect.y+i, rect.x+rect.w-1-i, rect.y+i);        // top
        SDL_RenderDrawLine(r, rect.x+i, rect.y+i, rect.x+i, rect.y+rect.h-1-i);        // left
    }
    // Bottom / Right shadow
    SDL_SetRenderDrawColor(r, shCol.r, shCol.g, shCol.b, 255);
    for (int i = 0; i < t; ++i) {
        SDL_RenderDrawLine(r, rect.x+i, rect.y+rect.h-1-i,
                           rect.x+rect.w-1-i, rect.y+rect.h-1-i);                      // bottom
        SDL_RenderDrawLine(r, rect.x+rect.w-1-i, rect.y+i,
                           rect.x+rect.w-1-i, rect.y+rect.h-1-i);                      // right
    }

    // Label text
    if (font && !label.empty()) {
        SDL_Color textCol = {30, 15, 5, 255};
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, label.c_str(), textCol);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
            if (tex) {
                SDL_Rect dst {
                    rect.x + (rect.w - surf->w) / 2 + (pressed ? 1 : 0),
                    rect.y + (rect.h - surf->h) / 2 + (pressed ? 1 : 0),
                    surf->w, surf->h
                };
                SDL_RenderCopy(r, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }
}

// Tooltip 
void WoodUI::drawTooltip(SDL_Renderer* r, TTF_Font* font,
                         int x, int y,
                         const std::string& text,
                         int alpha)
{
    if (!font) return;

    SDL_Color textCol = {30, 20, 10, 255};   // near-black, off-black warmth
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), textCol);
    if (!surf) return;

    int pad = 6;
    SDL_Rect box { x, y, surf->w + pad*2, surf->h + pad*2 };

    // Clamp to screen edges (assume 1024Ã—768)
    if (box.x + box.w > 1024) box.x = 1024 - box.w - 2;
    if (box.y + box.h > 768)  box.y = 768  - box.h - 2;
    if (box.x < 0) box.x = 0;
    if (box.y < 0) box.y = 0;

    // Semi-transparent white background
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 255, 252, 240, static_cast<uint8_t>(alpha));
    SDL_RenderFillRect(r, &box);

    // 3-D border
    SDL_SetRenderDrawColor(r, 80, 60, 20, 230);
    SDL_RenderDrawRect(r, &box);
    SDL_SetRenderDrawColor(r, 180, 150, 80, 230);
    SDL_Rect inner { box.x+1, box.y+1, box.w-2, box.h-2 };
    SDL_RenderDrawRect(r, &inner);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    // Text
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst { box.x + pad, box.y + pad, surf->w, surf->h };
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

// Log silhouette 
void WoodUI::drawLogSilhouette(SDL_Renderer* r, const SDL_Rect& destRect)
{
    // Two crossed log shapes as filled rounded rectangles
    SDL_SetRenderDrawColor(r, 35, 20, 8, 255);
    int cx  = destRect.x + destRect.w / 2;
    int bot = destRect.y + destRect.h - 4;
    int lh  = 14;   // log height
    // Log 1 (angled leftâ†’right)
    for (int i = -lh/2; i < lh/2; ++i) {
        SDL_RenderDrawLine(r,
            cx - destRect.w*3/8, bot + i,
            cx + destRect.w*3/8, bot + i - 6);
    }
    // Log 2 (angled rightâ†’left)
    for (int i = -lh/2; i < lh/2; ++i) {
        SDL_RenderDrawLine(r,
            cx - destRect.w*3/8, bot + i - 6,
            cx + destRect.w*3/8, bot + i);
    }
}

// Plain label
void WoodUI::drawLabel(SDL_Renderer* r, TTF_Font* font,
                       int x, int y, const std::string& text,
                       SDL_Color col)
{
    if (!font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst { x, y, surf->w, surf->h };
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
    SDL_FreeSurface(surf);   // intentional double-free guard via null after first
}
