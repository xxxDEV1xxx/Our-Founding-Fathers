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
 *    Tesseract OCR  â€” Apache-2.0    github.com/tesseract-ocr/tesseract
 *    Leptonica      â€” BSD-2-Clause  leptonica.org
 *
 *  Designed and authored by: Christopher T. Williams
 * ============================================================================
 */
#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <deque>
#include <cstdint>
#include <random>

// TextGlyph
// One character cell in the rolling text display.
struct TextGlyph {
    char    ch        = ' ';
    uint8_t alpha     = 0;      // current rendered alpha  0-255
    float   alphaf    = 0.f;    // float accumulator for smooth fade
    bool    fadeIn    = true;   // true = fading in, false = fading out
    bool    done      = false;  // fully faded out and can be recycled
};

// TextLine
// One line of glyphs as displayed on screen.
struct TextLine {
    std::vector<TextGlyph> glyphs;
    bool  fullyRevealed = false;   // all chars have reached alpha 255
    bool  fadingOut     = false;
    float lineAlpha     = 1.f;     // whole-line alpha for gradient fade-out
};

// SceneRenderer 
// Renders the complete Bard's Tale III INSPIRED night campfire scene:
//   Pixel-art night sky with stars and crescent moon
//   Silhouette pine tree treeline
//   Stone ring campfire with Doom-style fire simulation
//   Ground with grass / dirt texture
//   Rolling letter-by-letter text panel (top-left, like BT3)
//   Gradient fade-in on arrival, fade-out once 10 lines accumulate
class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    bool init(SDL_Renderer* renderer, TTF_Font* pixelFont);

    // Called every frame â€“ advances fire sim, letter roll, fades.
    void update(float dt);

    // Draw everything. Call between SDL_RenderClear and SDL_RenderPresent.
    void render(SDL_Renderer* renderer);

    // Feed the next character to display (called in sync with TTS word events).
    // The scene queues characters and reveals them letter by letter.
    void pushText(const std::string& word);   // push one word at a time
    void clearText();

    // Tooltip overlay: call each frame with mouse position.
    // hotspotWorld is the world-space centre of the fire hotspot.
    void renderTooltip(SDL_Renderer* renderer, int mx, int my,
                       SDL_Point hotspot, int radiusPx,
                       const std::string& text);

    // Expose fire hotspot so app can pass mouse coords in.
    SDL_Point fireHotspot() const;

    static constexpr int W = 1024;
    static constexpr int H = 768;

private:
    // Fire simulation (Doom algorithm)
    static constexpr int FW = 160;   // fire sim width  (pixels)
    static constexpr int FH = 100;   // fire sim height (pixels)

    std::vector<uint8_t>   firePx_;       // intensity 0-36
    SDL_Texture*           fireTex_  = nullptr;
    SDL_Color              firePal_[37];
    std::mt19937           rng_;

    void buildFirePalette();
    void seedFire();
    void spreadFire(int src);
    void updateFire();
    void renderFire(SDL_Renderer* r);

    // Fire blit rect on screen (the campfire pit area)
    SDL_Rect fireRect_ { 360, 390, 300, 200 };

    // Static scene elements (drawn once onto a cached texture)
    SDL_Texture* bgTex_ = nullptr;   // sky + trees + ground (static)
    void buildBackground(SDL_Renderer* r);

    void drawSky      (SDL_Renderer* r, const SDL_Rect& rc);
    void drawMoon     (SDL_Renderer* r);
    void drawStars    (SDL_Renderer* r);
    void drawTrees    (SDL_Renderer* r);
    void drawGround   (SDL_Renderer* r);
    void drawStoneRing(SDL_Renderer* r);

    // Rolling text
    TTF_Font* font_       = nullptr;
    int       fontH_      = 18;       // pixel height of one text line
    int       lineSpacing_= 22;       // pixels between lines

    static constexpr int MAX_LINES    = 10;   // before fade-out begins
    static constexpr int CHARS_WIDE   = 28;   // chars per line (fits left panel)
    static constexpr float REVEAL_RATE= 40.f; // chars revealed per second
    static constexpr float FADE_RATE  = 80.f; // alpha units per second (fade in)
    static constexpr float FADEOUT_RATE=60.f; // alpha units per second (fade out)

    // Text panel top-left anchor (like BT3 â€“ upper left of scene)
    SDL_Rect textPanel_ { 28, 24, 400, 240 };

    std::deque<TextLine>  lines_;       // active display lines
    std::string           wordQueue_;   // characters not yet assigned to a line
    std::string           pendingWord_; // word being assembled
    int                   charQueue_   = 0;   // chars queued for this line
    TextLine*             curLine_     = nullptr;

    float revealAccum_  = 0.f;   // fractional chars to reveal this frame
    float fadeAccum_    = 0.f;

    void tickReveal(float dt);
    void tickFades (float dt);
    void pushCharToLines(char c);
    void startNewLine();
    void renderText(SDL_Renderer* r);

    // Render one glyph with given alpha at pixel position.
    void renderGlyph(SDL_Renderer* r, char ch, int x, int y, uint8_t alpha);

    // Tooltip state
    float tooltipAlpha_ = 0.f;   // smooth appear / disappear

    // Misc
    // Glyph cache to avoid re-rendering identical chars every frame.
    struct GlyphCache {
        SDL_Texture* tex = nullptr;
        int w = 0, h = 0;
    };
    GlyphCache glyphCache_[128];   // ASCII 0-127

    void buildGlyphCache(SDL_Renderer* r);
    void freeGlyphCache();

    // Pixel-art helpers
    void fillRect(SDL_Renderer* r, int x, int y, int w, int h,
                  uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255);
    void hline   (SDL_Renderer* r, int x, int y, int w,
                  uint8_t R, uint8_t G, uint8_t B);
    void vline   (SDL_Renderer* r, int x, int y, int h,
                  uint8_t R, uint8_t G, uint8_t B);
    void circle  (SDL_Renderer* r, int cx, int cy, int radius,
                  uint8_t R, uint8_t G, uint8_t B, bool filled = false);
};
