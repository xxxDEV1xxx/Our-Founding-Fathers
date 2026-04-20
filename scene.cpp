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
#include "scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Helpers
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::fillRect(SDL_Renderer* r,
                              int x, int y, int w, int h,
                              uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_Rect rc{x,y,w,h};
    SDL_RenderFillRect(r, &rc);
}

void SceneRenderer::hline(SDL_Renderer* r, int x, int y, int w,
                           uint8_t R, uint8_t G, uint8_t B)
{
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderDrawLine(r, x, y, x+w-1, y);
}

void SceneRenderer::vline(SDL_Renderer* r, int x, int y, int h,
                           uint8_t R, uint8_t G, uint8_t B)
{
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderDrawLine(r, x, y, x, y+h-1);
}

void SceneRenderer::circle(SDL_Renderer* r, int cx, int cy, int radius,
                            uint8_t R, uint8_t G, uint8_t B, bool filled)
{
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            float d = std::sqrt(float(dx*dx + dy*dy));
            if (filled ? d <= radius : (d >= radius-1 && d <= radius+0.5f))
                SDL_RenderDrawPoint(r, cx+dx, cy+dy);
        }
    }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Constructor / Destructor
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
SceneRenderer::SceneRenderer()
    : firePx_(FW * FH, 0), rng_(std::random_device{}())
{
    buildFirePalette();
    std::memset(glyphCache_, 0, sizeof(glyphCache_));
}

SceneRenderer::~SceneRenderer()
{
    freeGlyphCache();
    if (fireTex_) SDL_DestroyTexture(fireTex_);
    if (bgTex_)   SDL_DestroyTexture(bgTex_);
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  init
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool SceneRenderer::init(SDL_Renderer* renderer, TTF_Font* pixelFont)
{
    font_ = pixelFont;
    if (font_) fontH_ = TTF_FontHeight(font_);

    // Fire texture
    fireTex_ = SDL_CreateTexture(renderer,
                                  SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  FW, FH);
    if (!fireTex_) return false;
    SDL_SetTextureBlendMode(fireTex_, SDL_BLENDMODE_BLEND);
    seedFire();

    // Static background
    buildBackground(renderer);

    // Glyph cache
    if (font_) buildGlyphCache(renderer);

    return true;
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Fire palette  (blackâ†’deep redâ†’orangeâ†’yellowâ†’white core)
//  Colours hand-matched to Bard's Tale III campfire palette
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::buildFirePalette()
{
    // 37 colours indexed 0 (cold) â†’ 36 (hottest)
    const SDL_Color stops[] = {
        {  0,  0,  0, 255},   //  0  black
        { 20,  0,  0, 255},   //  2  near-black red
        { 60,  0,  0, 255},   //  4  dark crimson
        {120,  0,  0, 255},   //  8  crimson
        {180, 20,  0, 255},   // 14  deep red
        {220, 60,  0, 255},   // 20  red-orange
        {240,120,  0, 255},   // 26  orange
        {255,180,  0, 255},   // 30  amber
        {255,220, 60, 255},   // 33  yellow
        {255,245,160, 255},   // 35  pale yellow
        {255,255,220, 255},   // 36  near-white
    };
    const int numStops = sizeof(stops)/sizeof(stops[0]);

    for (int i = 0; i < 37; ++i) {
        float t   = float(i) / 36.f;
        float si  = t * (numStops - 1);
        int   s0  = int(si);
        int   s1  = std::min(s0+1, numStops-1);
        float f   = si - s0;
        SDL_Color a = stops[s0], b = stops[s1];
        firePal_[i] = {
            uint8_t(a.r + f*(b.r-a.r)),
            uint8_t(a.g + f*(b.g-a.g)),
            uint8_t(a.b + f*(b.b-a.b)),
            255
        };
    }
    firePal_[0].a = 0;   // fully transparent black = shows background
}

void SceneRenderer::seedFire()
{
    for (int x = 0; x < FW; ++x)
        firePx_[(FH-1)*FW + x] = 36;
}

void SceneRenderer::spreadFire(int src)
{
    std::uniform_int_distribution<int> d(0,3);
    int rnd = d(rng_);
    int dst = src - FW - rnd + 1;
    if (dst < 0) return;
    int decay  = rnd & 1;
    firePx_[dst] = uint8_t(std::max(0, int(firePx_[src]) - decay));
}

void SceneRenderer::updateFire()
{
    // Flicker the seed row
    std::uniform_int_distribution<int> n(26, 36);
    for (int x = 0; x < FW; ++x)
        firePx_[(FH-1)*FW + x] = uint8_t(n(rng_));

    for (int y = 1; y < FH; ++y)
        for (int x = 0; x < FW; ++x)
            spreadFire(y*FW + x);
}

void SceneRenderer::renderFire(SDL_Renderer* r)
{
    void* px; int pitch;
    SDL_LockTexture(fireTex_, nullptr, &px, &pitch);
    auto* p = static_cast<uint32_t*>(px);
    int stride = pitch / 4;

    for (int y = 0; y < FH; ++y) {
        for (int x = 0; x < FW; ++x) {
            SDL_Color c = firePal_[firePx_[y*FW+x]];
            p[y*stride+x] =
                (uint32_t(c.r)<<24)|(uint32_t(c.g)<<16)|
                (uint32_t(c.b)<< 8)| uint32_t(c.a);
        }
    }
    SDL_UnlockTexture(fireTex_);

    // Scale fire up to the campfire pit rect
    SDL_RenderCopy(r, fireTex_, nullptr, &fireRect_);
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Static background (rendered once into bgTex_)
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::buildBackground(SDL_Renderer* r)
{
    bgTex_ = SDL_CreateTexture(r,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET,
                                W, H);
    if (!bgTex_) { std::cerr << "[Scene] bgTex_ create failed\n"; return; }

    SDL_SetRenderTarget(r, bgTex_);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    SDL_Rect full{0,0,W,H};
    drawSky   (r, full);
    drawMoon  (r);
    drawStars (r);
    drawTrees (r);
    drawGround(r);
    drawStoneRing(r);

    SDL_SetRenderTarget(r, nullptr);
}

// â”€â”€ Sky â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Deep blue-black gradient matching BT3's night palette
void SceneRenderer::drawSky(SDL_Renderer* r, const SDL_Rect& /*rc*/)
{
    // Sky occupies top ~60% of screen
    int skyH = int(H * 0.62f);
    for (int y = 0; y < skyH; ++y) {
        float t = float(y) / float(skyH);
        // Deep indigo at top â†’ near-black at horizon
        uint8_t R = uint8_t(  2 + t *  6);
        uint8_t G = uint8_t(  4 + t *  8);
        uint8_t B = uint8_t( 28 + t * 12);
        SDL_SetRenderDrawColor(r, R, G, B, 255);
        SDL_RenderDrawLine(r, 0, y, W-1, y);
    }
}

// â”€â”€ Moon â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Crescent moon upper-right, styled after BT3
void SceneRenderer::drawMoon(SDL_Renderer* r)
{
    int cx = W - 140, cy = 60, rad = 28;
    // Outer disc â€“ pale silvery yellow
    circle(r, cx, cy, rad, 210, 210, 140, true);
    // Inner "bite" to create crescent â€“ dark blue (sky colour)
    circle(r, cx+14, cy-8, rad-4, 5, 10, 36, true);

    // Subtle rim highlight
    for (int a = 200; a < 310; ++a) {
        float th = a * float(M_PI) / 180.f;
        int px = cx + int(float(rad)*std::cos(th));
        int py = cy + int(float(rad)*std::sin(th));
        SDL_SetRenderDrawColor(r, 240, 240, 200, 255);
        SDL_RenderDrawPoint(r, px, py);
    }
}

// â”€â”€ Stars â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::drawStars(SDL_Renderer* r)
{
    // Deterministic star field using a fixed seed
    std::mt19937 srng(0xBEEF1776);
    std::uniform_int_distribution<int> sx(0, W);
    std::uniform_int_distribution<int> sy(0, int(H*0.55f));
    std::uniform_int_distribution<int> bright(140, 255);
    std::uniform_int_distribution<int> size(0, 7);   // 0-6 = 1px, 7 = 2px

    for (int i = 0; i < 180; ++i) {
        int x  = sx(srng), y = sy(srng);
        uint8_t b = uint8_t(bright(srng));
        // Slight colour tint â€“ cool white or warm yellow
        uint8_t r2 = b, g2 = b;
        uint8_t bl = (i%5==0) ? uint8_t(std::min(255,int(b)+30)) : b;
        SDL_SetRenderDrawColor(r, r2, g2, bl, 255);
        SDL_RenderDrawPoint(r, x, y);
        if (size(srng) == 7) {   // ~1/8 are 2Ã—2
            SDL_RenderDrawPoint(r, x+1, y);
            SDL_RenderDrawPoint(r, x, y+1);
        }
    }
}

// â”€â”€ Pine trees â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Silhouette treeline, pixel-art style matching BT3 (layered triangles)
void SceneRenderer::drawTrees(SDL_Renderer* r)
{
    // Ground horizon Y
    int horizon = int(H * 0.62f);

    // Colour palette: very dark blue-greens for silhouette
    const SDL_Color treeCol[3] = {
        {  8, 20, 18, 255},   // dark
        { 12, 30, 22, 255},   // mid
        { 18, 40, 28, 255},   // lighter (front row)
    };

    // Helper: draw one triangular pine layer
    auto pine = [&](int cx, int baseY, int halfW, int height,
                    SDL_Color col)
    {
        for (int y = 0; y < height; ++y) {
            float t = float(y)/float(height);
            int hw = std::max(1, int(halfW * (1.f - t)));
            SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
            SDL_RenderDrawLine(r, cx-hw, baseY-y, cx+hw, baseY-y);
        }
    };

    // Back row (tallest, darkest)
    int bY = horizon + 10;
    for (int x = -30; x < W+30; x += 55) {
        int vary = (x * 37 + 17) % 25;
        pine(x,      bY, 28, 90+vary, treeCol[0]);
        pine(x+28,   bY, 22, 70+vary, treeCol[0]);
    }
    // Mid row
    for (int x = 10; x < W; x += 48) {
        int vary = (x * 53 + 7) % 20;
        pine(x,    bY+5, 24, 80+vary, treeCol[1]);
        pine(x+24, bY+5, 18, 60+vary, treeCol[1]);
    }
    // Front row (tallest silhouette trees flanking the fire area)
    pine(60,    bY+15, 32, 110, treeCol[2]);
    pine(140,   bY+15, 26,  90, treeCol[2]);
    pine(W-80,  bY+15, 32, 110, treeCol[2]);
    pine(W-160, bY+15, 26,  90, treeCol[2]);
    // Fill solid ground under each tree
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    fillRect(r, 0, bY+15, W, H - bY - 15, 0, 0, 0);
}

// â”€â”€ Ground â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::drawGround(SDL_Renderer* r)
{
    int groundY = int(H * 0.64f);
    int groundH = H - groundY;

    // Dark earth base
    for (int y = 0; y < groundH; ++y) {
        float t = float(y)/float(groundH);
        uint8_t R = uint8_t(14 + t*8);
        uint8_t G = uint8_t( 9 + t*5);
        uint8_t B = uint8_t( 4 + t*2);
        SDL_SetRenderDrawColor(r, R, G, B, 255);
        SDL_RenderDrawLine(r, 0, groundY+y, W-1, groundY+y);
    }

    // Scattered pixel "pebble" texture
    std::mt19937 grng(0xC0FFEE);
    std::uniform_int_distribution<int> gx(0, W);
    std::uniform_int_distribution<int> gy(groundY+5, H-10);
    std::uniform_int_distribution<int> gb(30, 55);
    for (int i = 0; i < 400; ++i) {
        uint8_t b = uint8_t(gb(grng));
        SDL_SetRenderDrawColor(r, b, uint8_t(b*0.7f), uint8_t(b*0.4f), 255);
        SDL_RenderDrawPoint(r, gx(grng), gy(grng));
    }

    // Firelight glow on ground (orange ellipse under fire area)
    int gc = W/2, gy2 = int(H * 0.71f);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int ey = -30; ey <= 30; ++ey) {
        for (int ex = -120; ex <= 120; ++ex) {
            float d = std::sqrt(float(ex*ex)/float(120*120) +
                                float(ey*ey)/float(30*30));
            if (d > 1.f) continue;
            uint8_t alpha = uint8_t((1.f-d)*(1.f-d) * 80);
            SDL_SetRenderDrawColor(r, 220, 100, 20, alpha);
            SDL_RenderDrawPoint(r, gc+ex, gy2+ey);
        }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

// â”€â”€ Stone ring â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// BT3's campfire has visible grey/blue stones around the fire base
void SceneRenderer::drawStoneRing(SDL_Renderer* r)
{
    // Ring centre
    int cx = W / 2;
    int cy = int(H * 0.725f);
    int rx = 110, ry = 28;   // ellipse radii

    // Stone positions spread around the ellipse
    const int NUM_STONES = 11;
    for (int i = 0; i < NUM_STONES; ++i) {
        float angle = float(i) * float(M_PI*2) / float(NUM_STONES);
        int sx = cx + int(float(rx) * std::cos(angle));
        int sy = cy + int(float(ry) * std::sin(angle));

        int sw = 18 + (i%3)*4;
        int sh = 11 + (i%2)*3;

        // Stone face â€“ grey-blue, BT3 palette
        uint8_t gv = uint8_t(85 + (i%5)*12);
        fillRect(r, sx-sw/2, sy-sh/2, sw, sh,
                 uint8_t(gv*0.8f), uint8_t(gv*0.85f), gv);

        // Top highlight
        hline(r, sx-sw/2+1, sy-sh/2,   sw-2, 140, 150, 170);
        // Shadow bottom
        hline(r, sx-sw/2+1, sy+sh/2-1, sw-2,  40,  45,  55);
        // Inner shadow edge
        hline(r, sx-sw/2+2, sy-sh/2+1, sw-4,  60,  65,  80);

        // Occasional moss pixel accent (dark green)
        if (i%3 == 0) {
            SDL_SetRenderDrawColor(r, 30, 60, 30, 255);
            SDL_RenderDrawPoint(r, sx-2, sy-sh/2+2);
            SDL_RenderDrawPoint(r, sx+3, sy-sh/2+3);
        }
    }

    // Ash / ember bed inside ring
    fillRect(r, cx-70, cy-8, 140, 18, 28, 24, 20);
    // Purple-pink ember glow (matches BT3 magenta fire base)
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int ey = -7; ey <= 7; ++ey) {
        for (int ex = -60; ex <= 60; ++ex) {
            float d = std::sqrt(float(ex*ex)/3600.f + float(ey*ey)/49.f);
            if (d > 1.f) continue;
            uint8_t a = uint8_t((1.f-d)*120);
            SDL_SetRenderDrawColor(r, 160, 40, 120, a);
            SDL_RenderDrawPoint(r, cx+ex, cy+ey);
        }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    // Two crossing log silhouettes
    SDL_SetRenderDrawColor(r, 30, 18, 8, 255);
    for (int t = -6; t <= 6; ++t) {
        SDL_RenderDrawLine(r, cx-80, cy+t+4, cx+80, cy+t-4);
        SDL_RenderDrawLine(r, cx-80, cy+t-4, cx+80, cy+t+4);
    }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Glyph cache
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::buildGlyphCache(SDL_Renderer* r)
{
    if (!font_) return;
    SDL_Color white{255,255,255,255};
    for (int c = 32; c < 127; ++c) {
        char buf[2] = {char(c), 0};
        SDL_Surface* s = TTF_RenderText_Blended(font_, buf, white);
        if (!s) continue;
        glyphCache_[c].tex = SDL_CreateTextureFromSurface(r, s);
        glyphCache_[c].w   = s->w;
        glyphCache_[c].h   = s->h;
        SDL_FreeSurface(s);
    }
}

void SceneRenderer::freeGlyphCache()
{
    for (auto& g : glyphCache_)
        if (g.tex) { SDL_DestroyTexture(g.tex); g.tex = nullptr; }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  Text system
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void SceneRenderer::pushText(const std::string& word)
{
    // Append word + space to the queue
    for (char c : word) wordQueue_ += c;
    wordQueue_ += ' ';
}

void SceneRenderer::clearText()
{
    lines_.clear();
    wordQueue_.clear();
    revealAccum_ = 0.f;
}

void SceneRenderer::startNewLine()
{
    TextLine line;
    lines_.push_back(std::move(line));
    curLine_ = &lines_.back();
    charQueue_ =
