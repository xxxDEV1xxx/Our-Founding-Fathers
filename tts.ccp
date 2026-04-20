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
#include "tts.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>

// â”€â”€ eSpeak-NG header detection â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#if __has_include(<espeak-ng/speak_lib.h>)
#  include <espeak-ng/speak_lib.h>
#  define ESPEAK_HEADER_OK 1
#elif __has_include(<espeak/speak_lib.h>)
#  include <espeak/speak_lib.h>
#  define ESPEAK_HEADER_OK 1
#else
#  warning "eSpeak-NG headers not found â€“ TTS will be a no-op stub"
#  define ESPEAK_HEADER_OK 0
#endif

// â”€â”€ Global engine pointer for the C callback â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
static TtsEngine* g_ttsEngine = nullptr;

// â”€â”€ eSpeak callback â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#if ESPEAK_HEADER_OK
static int espeakEventCallback(short* /*wav*/, int /*numSamples*/,
                                espeak_EVENT* events)
{
    for (; events && events->type != espeakEVENT_LIST_TERMINATED; ++events) {
        if (events->type == espeakEVENT_WORD && g_ttsEngine) {
            // text_position is 1-based character offset into the synthesised text
            g_ttsEngine->onWordEvent(
                static_cast<std::size_t>(events->text_position - 1));
        }
    }
    return 0;   // 0 = continue synthesis
}
#endif

// â”€â”€ Helpers â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Attempt to set a voice; return false if eSpeak rejects it.
/*static*/ bool TtsEngine::tryVoice(const char* voiceName)
{
#if ESPEAK_HEADER_OK
    // espeak_SetVoiceByName returns EE_OK (0) on success
    return espeak_SetVoiceByName(voiceName) == EE_OK;
#else
    (void)voiceName;
    return false;
#endif
}

// Push all tuning parameters from 'p' to the live eSpeak instance.
/*static*/ void TtsEngine::applyProfile(const VoiceProfile& p, int volumePct)
{
#if ESPEAK_HEADER_OK
    int esVol = std::clamp(volumePct * 2, 0, 200);   // app 0-100 â†’ eSpeak 0-200

    espeak_SetParameter(espeakRATE,      p.rate,      0);
    espeak_SetParameter(espeakPITCH,     p.pitch,     0);
    espeak_SetParameter(espeakVOLUME,    esVol,       0);
    espeak_SetParameter(espeakWORDGAP,   p.wordGap,   0);

    // Flutter and roughness live in the voice file itself in eSpeak-NG, but
    // we can approximate them through the "voice variant" string appended
    // to the voice name (already encoded in the profile name, e.g. +croak).
    // As a belt-and-suspenders measure we nudge PITCH_RANGE to give a more
    // declamatory, classical cadence.
    //   Low range â†’ flat, authoritative preacher delivery
    //   High range â†’ expressive, conversational
    int pitchRange = std::max(0, 40 - p.flutter / 4);   // 0-40
    espeak_SetParameter(espeakRANGE, pitchRange, 0);
#else
    (void)p; (void)volumePct;
#endif
}

// â”€â”€ Constructor / Destructor â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
TtsEngine::TtsEngine()  { g_ttsEngine = this; }
TtsEngine::~TtsEngine() {
    stop();
    if (ttsThread_.joinable()) ttsThread_.join();
#if ESPEAK_HEADER_OK
    espeak_Terminate();
#endif
    if (g_ttsEngine == this) g_ttsEngine = nullptr;
}

// â”€â”€ init â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool TtsEngine::init(const TtsConfig& cfg)
{
    cfg_ = cfg;

#if ESPEAK_HEADER_OK
    // AUDIO_OUTPUT_PLAYBACK â†’ eSpeak uses the system audio device directly.
    // buflength=0 â†’ use eSpeak default (500 ms).
    int sr = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, nullptr, 0);
    if (sr < 0) {
        std::cerr << "[TTS] espeak_Initialize failed (returned " << sr << ")\n";
        return false;
    }
    espeak_SetSynthCallback(espeakEventCallback);

    // Walk VOICE_PROFILES from best to fallback until one loads.
    int target = std::clamp(cfg.profileIndex, 0, NUM_VOICE_PROFILES - 1);

    // Try the requested profile first (primary then its fallback).
    auto tryProfile = [&](int idx) -> bool {
        const VoiceProfile& p = VOICE_PROFILES[idx];
        if (tryVoice(p.primary)) {
            std::cout << "[TTS] Voice loaded: " << p.primary
                      << "  (" << p.name << ")\n";
            activeProfile_ = idx;
            applyProfile(p, cfg.volume);
            return true;
        }
        std::cout << "[TTS] Primary '" << p.primary
                  << "' unavailable â€“ trying fallback '" << p.fallback << "'\n";
        if (tryVoice(p.fallback)) {
            std::cout << "[TTS] Fallback loaded: " << p.fallback << "\n";
            activeProfile_ = idx;
            applyProfile(p, cfg.volume);
            return true;
        }
        return false;
    };

    // First try the user-requested profile.
    if (!tryProfile(target)) {
        // Then walk remaining profiles in order.
        bool loaded = false;
        for (int i = 0; i < NUM_VOICE_PROFILES && !loaded; ++i) {
            if (i != target) loaded = tryProfile(i);
        }
        if (!loaded) {
            // Last-resort: bare "en"
            espeak_SetVoiceByName("en");
            std::cerr << "[TTS] All profiles failed; using bare 'en' voice\n";
            activeProfile_ = 2;   // closest: Elder
        }
    }

    state_ = TtsState::Idle;
    return true;
#else
    std::cerr << "[TTS] Compiled without eSpeak headers â€“ no voice output\n";
    return false;
#endif
}

// â”€â”€ setProfile â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void TtsEngine::setProfile(int idx)
{
    idx = std::clamp(idx, 0, NUM_VOICE_PROFILES - 1);
    cfg_.profileIndex = idx;
#if ESPEAK_HEADER_OK
    const VoiceProfile& p = VOICE_PROFILES[idx];
    if (!tryVoice(p.primary)) tryVoice(p.fallback);
    activeProfile_ = idx;
    applyProfile(p, cfg_.volume);
#endif
}

// â”€â”€ setVolume â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void TtsEngine::setVolume(int pct)
{
    cfg_.volume = std::clamp(pct, 0, 100);
#if ESPEAK_HEADER_OK
    espeak_SetParameter(espeakVOLUME, cfg_.volume * 2, 0);
#endif
}

// â”€â”€ profileName â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
const char* TtsEngine::profileName() const
{
    return VOICE_PROFILES[activeProfile_].name;
}

// â”€â”€ playText â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void TtsEngine::playText(const std::string& text, std::size_t startIndex)
{
    stop();
    if (ttsThread_.joinable()) ttsThread_.join();

    currentText_   = text;
    currentIndex_  = startIndex;
    state_         = TtsState::Playing;

    ttsThread_ = std::thread([this, startIndex]{ threadFunc(startIndex); });
}

void TtsEngine::threadFunc(std::size_t startIndex)
{
#if ESPEAK_HEADER_OK
    if (currentText_.empty()) { state_ = TtsState::Idle; return; }

    std::size_t idx = std::min(startIndex, currentText_.size() - 1);
    const char* ptr = currentText_.c_str() + idx;
    auto sz = static_cast<unsigned int>(currentText_.size() - idx + 1);

    // espeakCHARS_UTF8: treat input as UTF-8.
    // POS_CHARACTER   : the offset we pass is a character (byte) position.
    espeak_Synth(ptr, sz, 0, POS_CHARACTER, 0,
                 espeakCHARS_UTF8, nullptr, nullptr);
    espeak_Synchronize();   // blocks until audio playback finishes

    if (state_.load() != TtsState::Stopped)
        state_ = TtsState::Idle;
#else
    (void)startIndex;
    state_ = TtsState::Idle;
#endif
}

// â”€â”€ stop / pause / resume â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void TtsEngine::stop()
{
#if ESPEAK_HEADER_OK
    espeak_Cancel();
#endif
    state_ = TtsState::Stopped;
}

void TtsEngine::pause()
{
    if (state_.load() != TtsState::Playing) return;
#if ESPEAK_HEADER_OK
    espeak_Cancel();
#endif
    state_ = TtsState::Paused;
    // currentIndex_ was last updated by onWordEvent â€“ resume picks it up.
}

void TtsEngine::resume()
{
    if (state_.load() != TtsState::Paused) return;
    std::size_t idx = currentIndex_.load();
    if (ttsThread_.joinable()) ttsThread_.join();
    state_ = TtsState::Playing;
    ttsThread_ = std::thread([this, idx]{ threadFunc(idx); });
}

// â”€â”€ onWordEvent â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void TtsEngine::onWordEvent(std::size_t charPos)
{
    currentIndex_ = charPos;

    // Extract the word starting at charPos and pass it to the scene display
    if (wordCB_ && !currentText_.empty()) {
        std::size_t start = charPos;
        std::size_t end   = start;
        while (end < currentText_.size() &&
               !std::isspace(static_cast<unsigned char>(currentText_[end])))
            ++end;
        if (end > start)
            wordCB_(currentText_.substr(start, end - start));
    }
}

// â”€â”€ seekToWordBoundary â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
/*static*/ std::size_t TtsEngine::seekToWordBoundary(const std::string& text,
                                                      std::size_t index)
{
    if (text.empty()) return 0;
    if (index >= text.size()) index = text.size() - 1;

    // Step back to the start of this word (stop at whitespace).
    while (index > 0 &&
           !std::isspace(static_cast<unsigned char>(text[index - 1])))
        --index;

    // Skip any leading whitespace so we land on a real character.
    while (index < text.size() &&
           std::isspace(static_cast<unsigned char>(text[index])))
        ++index;

    return index;
}
