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
#include "woodui.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

// Speed-stage jump sizes in characters (approx words * avg-word-length)
static constexpr std::size_t STAGE_CHARS[] = {40, 120, 200, 400, 800};

// â”€â”€ Ctor / Dtor â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
App::App()  {}
App::~App() {
    stopAmbient();
    tts_.stop();
    if (fireChunk_)  Mix_FreeChunk(fireChunk_);
    if (font_)       TTF_CloseFont(font_);
    if (smallFont_)  TTF_CloseFont(smallFont_);
    if (renderer_)   SDL_DestroyRenderer(renderer_);
    if (window_)     SDL_DestroyWindow(window_);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    delete repo_;
}

// â”€â”€ init â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool App::init(const std::string& dataRoot)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "[App] SDL_Init: " << SDL_GetError() << "\n";
        return false;
    }
    if (TTF_Init() != 0) {
        std::cerr << "[App] TTF_Init: " << TTF_GetError() << "\n";
        return false;
    }
    // Try 44100 Hz first; fall back to 22050 for older hardware
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
            std::cerr << "[App] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
            return false;
        }
    }

    window_ = SDL_CreateWindow("Fireside Reader â€” Christopher Williams",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!window_) { std::cerr << SDL_GetError(); return false; }

    renderer_ = SDL_CreateRenderer(window_, -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) { std::cerr << SDL_GetError(); return false; }

    // â”€â”€ Fonts â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Try a bundled font first; fall back to any TTF in the assets folder.
    font_ = TTF_OpenFont("assets/OldEnglish.ttf", 15);
    if (!font_) font_ = TTF_OpenFont("assets/font.ttf", 15);
    if (!font_) {
        std::cerr << "[App] No font found in assets/  â€“ label text will be absent\n";
    }
    smallFont_ = TTF_OpenFont("assets/OldEnglish.ttf", 12);
    if (!smallFont_) smallFont_ = font_;

    // â”€â”€ Fire ambient sound â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Try OGG/MP3 first (SDL_mixer with OGG support), fall back to .wav
    fireChunk_ = Mix_LoadWAV("assets/fire_loop.ogg");
    if (!fireChunk_) fireChunk_ = Mix_LoadWAV("assets/fire_loop.wav");
    if (!fireChunk_) std::cerr << "[App] No fire_loop audio found\n";

    // â”€â”€ TTS â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    TtsConfig cfg;
    cfg.profileIndex = 0;       // "Orator (MBROLA en1)" â€“ best quality
    cfg.volume       = ttsVol_;
    if (!tts_.init(cfg))
        std::cerr << "[App] TTS init failed â€“ voice will be unavailable\n";

    // â”€â”€ Repository â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    repo_ = new Repository(dataRoot);
    repo_->scan();

    // â”€â”€ UI â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    WoodUI::initGrain(0xDEAD);
    scene_.init(renderer_, font_);

    // Wire TTS word events â†’ scene text display
    tts_.setWordCallback([this](const std::string& word){
        scene_.pushText(word);
    });
    buildMainButtons();
    buildControlButtons();

    return true;
}

// â”€â”€ buildMainButtons â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::buildMainButtons()
{
    mainBtns_.clear();
    const int margin  = 8;
    const int btnH    = 36;
    const int btnW    = WIN_W - 40;
    const int startX  = 20;
    const int startY  = 20;

    const auto& authors = repo_->authors();

    for (int i = 0; i < 16; ++i) {
        Button b;
        b.rect  = { startX, startY + i * (btnH + margin), btnW, btnH };
        b.label = (i < (int)authors.size()) ? authors[i].name : "[Empty]";

        b.onClick = [this, i]() {
            if (i < (int)repo_->authors().size())
                enterFireScreen(i);
        };
        mainBtns_.push_back(b);
    }
}

// â”€â”€ buildControlButtons â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::buildControlButtons()
{
    ctrlBtns_.clear();

    // Layout: fire screen control bar sits at y = WIN_H - 160
    const int barY  = WIN_H - 150;
    const int btnH  = 32;
    const int capW  = 70;   // end-cap (rev/fwd) width
    const int midW  = 70;   // middle buttons
    const int playW = 80;
    const int cx    = WIN_W / 2;

    // Helper
    auto make = [&](int x, int w, const std::string& lbl,
                    std::function<void()> fn)
    {
        Button b;
        b.rect    = { x - w/2, barY, w, btnH };
        b.label   = lbl;
        b.onClick = fn;
        ctrlBtns_.push_back(b);
    };

    // â”€â”€ Left end-cap: REVERSE â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    make(cx - 4*midW - capW/2, capW, "REV", [this]() {
        pb_.revStage = (pb_.revStage + 1) % PlaybackContext::NUM_STAGES;
        int s = PlaybackContext::SPEED_STAGES[pb_.revStage];
        seekBy(-static_cast<int>(STAGE_CHARS[pb_.revStage]), s);
    });

    // â”€â”€ Left inner buttons â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    make(cx - 3*midW, midW, "PrvPg",   [this](){ changePage(-1);   });
    make(cx - 2*midW, midW, "PrvBk",   [this](){ changeBook(-1);   });
    make(cx - 1*midW, midW, "PrvAuth", [this](){ changeAuthor(-1); });

    // â”€â”€ Centre: PLAY/PAUSE â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    make(cx, playW, "PLAY", [this]() {
        auto st = tts_.state();
        if (st == TtsState::Playing) {
            tts_.pause();
        } else if (st == TtsState::Paused) {
            tts_.resume();
        } else {
            startReading();
        }
    });

    // â”€â”€ Right inner buttons â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    make(cx + 1*midW, midW, "NxtAuth", [this](){ changeAuthor(1);  });
    make(cx + 2*midW, midW, "NxtBk",   [this](){ changeBook(1);    });
    make(cx + 3*midW, midW, "NxtPg",   [this](){ changePage(1);    });

    // â”€â”€ Right end-cap: FORWARD â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    make(cx + 4*midW + capW/2, capW, "FWD", [this]() {
        pb_.fwdStage = (pb_.fwdStage + 1) % PlaybackContext::NUM_STAGES;
        seekBy(static_cast<int>(STAGE_CHARS[pb_.fwdStage]), pb_.fwdStage);
    });

    // â”€â”€ Volume row (below control bar) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    const int volY  = barY + btnH + 12;
    const int volW  = 44;
    const int volH  = 28;

    // TTS volume
    Button vt_m;
    vt_m.rect  = { cx - 3*volW, volY, volW, volH };
    vt_m.label = "T-";
    vt_m.onClick = [this](){ adjustTtsVol(-5); };
    ctrlBtns_.push_back(vt_m);

    Button vt_p;
    vt_p.rect  = { cx - 2*volW, volY, volW, volH };
    vt_p.label = "T+";
    vt_p.onClick = [this](){ adjustTtsVol(5); };
    ctrlBtns_.push_back(vt_p);

    // Ambient volume
    Button va_m;
    va_m.rect  = { cx + volW, volY, volW, volH };
    va_m.label = "A-";
    va_m.onClick = [this](){ adjustAmbVol(-5); };
    ctrlBtns_.push_back(va_m);

    Button va_p;
    va_p.rect  = { cx + 2*volW, volY, volW, volH };
    va_p.label = "A+";
    va_p.onClick = [this](){ adjustAmbVol(5); };
    ctrlBtns_.push_back(va_p);

    // Voice profile cycle (centred below the volume row)
    Button vprof;
    vprof.rect    = { cx - volW, volY + volH + 6, volW * 3, volH };
    vprof.label   = "Voice";
    vprof.onClick = [this]() {
        tts_profile_ = (tts_profile_ + 1) % NUM_VOICE_PROFILES;
        tts_.setProfile(tts_profile_);
    };
    ctrlBtns_.push_back(vprof);
}

// â”€â”€ run / mainLoop â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::run() { mainLoop(); }

void App::mainLoop()
{
    while (running_) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) handleEvent(e);
        update();
        render();
        SDL_Delay(16);   // ~60 fps
    }
}

// â”€â”€ handleEvent â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::handleEvent(const SDL_Event& e)
{
    if (e.type == SDL_QUIT) { running_ = false; return; }

    if (screen_ == Screen::MainMenu) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            for (auto& b : mainBtns_) {
                if (inRect(e.button.x, e.button.y, b.rect)) {
                    b.pressed = true;
                    if (b.onClick) b.onClick();
                    b.pressed = false;
                    return;
                }
            }
        }
        return;
    }

    // â”€â”€ Fire screen events â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
        // Stop reverse/forward speed-seek on any input if playing
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            // Check control buttons first
            for (auto& b : ctrlBtns_) {
                if (inRect(mx, my, b.rect)) {
                    b.pressed = true;
                    if (b.onClick) b.onClick();
                    b.pressed = false;
                    return;
                }
            }
            // Click anywhere on fire area = stop rev/fwd, resume from word
            SDL_Point fhs = scene_.fireHotspot();
            SDL_Rect sceneFireArea { fhs.x - 160, fhs.y - 180, 320, 210 };
            if (inRect(mx, my, sceneFireArea)) {
                pb_.revStage = 0; pb_.fwdStage = 0;
                // If TTS was paused by seek, resume
                if (tts_.state() == TtsState::Paused) tts_.resume();
                return;
            }
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            stopReading();
            stopAmbient();
            screen_ = Screen::MainMenu;
        }
    }
}

// â”€â”€ update â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::update()
{
    if (screen_ == Screen::Fire) {
        scene_.update(1.f/60.f);

        // 4-second intro delay
        if (!readingStarted_) {
            Uint32 elapsed = SDL_GetTicks() - fireEnterTick_;
            if (elapsed >= 4000) {
                readingStarted_ = true;
                startReading();
            }
        }
    }
}

// â”€â”€ render â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::render()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    if (screen_ == Screen::MainMenu)  renderMainMenu();
    else                               renderFireScreen();

    SDL_RenderPresent(renderer_);
}

// â”€â”€ renderMainMenu â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::renderMainMenu()
{
    SDL_Rect fullScreen { 0, 0, WIN_W, WIN_H };
    WoodUI::drawWoodPanel(renderer_, fullScreen);

    for (auto& b : mainBtns_)
        WoodUI::drawButton(renderer_, font_, b.rect, b.label, b.pressed);

    // Title
    if (font_) {
        WoodUI::drawLabel(renderer_, font_,
                          WIN_W/2 - 120, WIN_H - 30,
                          "Fireside Reader â€” Select an Author",
                          {220,200,140,255});
    }
}

// â”€â”€ renderFireScreen â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::renderFireScreen()
{
    // â”€â”€ Bard's Tale III scene (sky + trees + campfire + rolling text) â”€â”€â”€â”€â”€â”€â”€â”€â”€
    scene_.render(renderer_);

    // â”€â”€ Control bar overlay (dark semi-transparent strip at bottom) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    SDL_Rect barBg { 0, WIN_H - 155, WIN_W, 155 };
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 8, 5, 2, 210);
    SDL_RenderFillRect(renderer_, &barBg);
    // Thin amber top border on the bar
    SDL_SetRenderDrawColor(renderer_, 140, 90, 20, 200);
    SDL_RenderDrawLine(renderer_, 0, WIN_H-155, WIN_W-1, WIN_H-155);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);

    // Control buttons
    for (auto& b : ctrlBtns_)
        WoodUI::drawButton(renderer_, font_, b.rect, b.label, b.pressed);

    // Labels
    if (smallFont_) {
        // Volume
        std::string tvol = "TTS:" + std::to_string(ttsVol_) + "%";
        std::string avol = "AMB:" + std::to_string(ambVol_) + "%";
        WoodUI::drawLabel(renderer_, smallFont_, WIN_W/2-130, WIN_H-48, tvol);
        WoodUI::drawLabel(renderer_, smallFont_, WIN_W/2+ 50, WIN_H-48, avol);

        // Speed stages
        std::string rvStr = "REV x" +
            std::to_string(PlaybackContext::SPEED_STAGES[pb_.revStage]);
        std::string fwStr = "FWD x" +
            std::to_string(PlaybackContext::SPEED_STAGES[pb_.fwdStage]);
        WoodUI::drawLabel(renderer_, smallFont_,  6, WIN_H-155, rvStr);
        WoodUI::drawLabel(renderer_, smallFont_, WIN_W-80, WIN_H-155, fwStr);

        // Author / book / page â€” shown in lower-left of scene above bar
        auto& authors = repo_->authors();
        if (!authors.empty() && pb_.authorIdx < (int)authors.size()) {
            auto& a = authors[pb_.authorIdx];
            std::string info = a.name;
            if (pb_.bookIdx < (int)a.books.size())
                info += "  Ã¢Â€Â”  " + a.books[pb_.bookIdx].title;
            if (!pb_.pages.empty())
                info += "  [" + std::to_string(pb_.pageIdx+1)
                      + "/" + std::to_string(pb_.pages.size()) + "]";
            WoodUI::drawLabel(renderer_, smallFont_, 8, WIN_H-170, info,
                              {180,140,60,255});
        }

        // Voice profile
        std::string vname = std::string("Voice: ") + tts_.profileName();
        WoodUI::drawLabel(renderer_, smallFont_, WIN_W/2-70, WIN_H-24, vname,
                          {140,110,50,255});
    }

    // â”€â”€ Tooltip (fire hotspot hover) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    scene_.renderTooltip(renderer_, mx, my,
                         scene_.fireHotspot(), 96,
                         "Designed by: Christopher Williams ~");
}

// â”€â”€ enterFireScreen â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::enterFireScreen(int authorIdx)
{
    pb_.authorIdx = authorIdx;
    pb_.bookIdx   = 0;
    pb_.pageIdx   = 0;
    pb_.currentCharIdx = 0;
    pb_.revStage  = 0;
    pb_.fwdStage  = 0;
    readingStarted_  = false;
    screen_          = Screen::Fire;
    fireEnterTick_   = SDL_GetTicks();

    loadCurrentBook();
    startAmbient();
}

// â”€â”€ loadCurrentBook â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::loadCurrentBook()
{
    auto& authors = repo_->authors();
    if (authors.empty() || pb_.authorIdx >= (int)authors.size()) return;
    auto& a = authors[pb_.authorIdx];
    if (a.books.empty() || pb_.bookIdx >= (int)a.books.size()) return;

    std::string path = a.books[pb_.bookIdx].path;
    pb_.currentText  = repo_->loadBookText(path);
    pb_.pages        = repo_->paginate(pb_.currentText);
    pb_.currentCharIdx = pb_.pages.empty() ? 0 : pb_.pages[pb_.pageIdx].startIndex;

    std::cout << "[App] Loaded: " << path
              << " (" << pb_.currentText.size() << " chars, "
              << pb_.pages.size() << " pages)\n";
}

// â”€â”€ startReading / stopReading â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void App::startReading()
{
    tts_.setVolume(ttsVol_);
    tts_.playText(pb_.currentText, pb_.currentCharIdx);
}

void App::stopReading()
{
    tts_.stop();
    scene_.clearText();
}

// â”€â”€ changePage â”€â”€â”€â”€â”
