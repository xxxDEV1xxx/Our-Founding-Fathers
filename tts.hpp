/*
 * ============================================================================
 *  Fireside Reader
 *  Copyright (C) 2025  Christopher T. Williams All Rights Reserved
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
#pragma once
#include <functional>
#include <string>
#include <atomic>
#include <thread>

// Voice profiles
// Each entry is a complete eSpeak-NG parameter set tuned for a classical /
// old-English character.  The engine tries the "primary" voice first; if it
// fails to load (e.g. MBROLA not installed) it transparently falls back to the
// pure-formant voice listed in "fallback".
struct VoiceProfile {
    const char* name;           // display name shown in UI
    const char* primary;        // eSpeak-NG voice string (MBROLA-backed preferred)
    const char* fallback;       // pure formant voice used when primary unavailable
    int rate;                   // words per minute  (80-400, default 175)
    int pitch;                  // 0-99  (lower = deeper / more authoritative)
    int wordGap;                // extra inter-word pause  (units of 10 ms)
    int flutter;                // 0-100  pitch wavering â†’ "older" quality
    int roughness;              // 0-7    creaky timbre
};

// Ordered best â†’ adequate.  The engine selects the first profile whose
// primary voice loads, so on a system without MBROLA it silently uses the
// formant voices.
inline constexpr VoiceProfile VOICE_PROFILES[] = {
    //  name                      primary        fallback    rate  pit  gap  flu  rgh
    { "Orator  (MBROLA en1)",  "mb-en1",      "en-rp",     115,  38,   1,  12,   1 },
    { "Rector  (MBROLA en2)",  "mb-en2",      "en-gb",     110,  35,   2,  10,   2 },
    { "Elder   (Brit RP)",     "en-rp",       "en-gb",     120,  40,   1,  14,   2 },
    { "Statesman (Brit GB)",   "en-gb",       "en",        125,  42,   1,   8,   1 },
    { "Preacher (RP +m3)",     "en-rp+m3",    "en+m3",     108,  36,   2,  16,   3 },
    { "Scholar  (GB +m2)",     "en-gb+m2",    "en+m2",     118,  44,   1,  10,   1 },

};
inline constexpr int NUM_VOICE_PROFILES =
    static_cast<int>(sizeof(VOICE_PROFILES) / sizeof(VOICE_PROFILES[0]));

// TtsConfig
struct TtsConfig {
    int profileIndex = 0;   // index into VOICE_PROFILES[]
    int volume       = 17;  // app-level 0-100; scaled to eSpeak 0-200 internally
};

// TtsState
enum class TtsState { Idle, Playing, Paused, Stopped };

// TtsEngine
class TtsEngine {
public:
    TtsEngine();
    ~TtsEngine();

    // Call once at startup.  Returns false if the eSpeak library itself failed.
    bool init(const TtsConfig& cfg);

    // Switch voice profile at runtime (takes effect on next playText call).
    void setProfile(int profileIndex);

    // Volume 0-100 (app scale).
    void setVolume(int pct);

    // Begin speaking from byte offset startIndex.  Non-blocking.
    void playText(const std::string& text, std::size_t startIndex = 0);

    void stop();
    void pause();
    void resume();

    // Walk 'index' back to the first byte of the enclosing word.
    static std::size_t seekToWordBoundary(const std::string& text,
                                          std::size_t index);

    TtsState    state()        const { return state_.load(); }
    std::size_t currentIndex() const { return currentIndex_.load(); }

    // Active profile display name, e.g. "Elder (Brit RP)".
    const char* profileName() const;

    // Called by the eSpeak word-boundary callback â€“ do not invoke directly.
    void onWordEvent(std::size_t charPos);

    // Optional: set a callback invoked on every word event with the word text.
    // Used to feed words into the scene text display in sync with TTS.
    using WordCB = std::function<void(const std::string& word)>;
    void setWordCallback(WordCB cb) { wordCB_ = std::move(cb); }

private:
    TtsConfig cfg_;
    int       activeProfile_ = 0;   // index of the profile actually loaded

    std::atomic<TtsState>    state_        { TtsState::Idle };
    std::atomic<std::size_t> currentIndex_ { 0 };

    std::string currentText_;
    std::thread ttsThread_;

    // Returns true if eSpeak accepted the voice name without error.
    static bool tryVoice(const char* voiceName);

    // Push all parameters from 'p' into the eSpeak library.
    static void applyProfile(const VoiceProfile& p, int volumePct);

    void threadFunc(std::size_t startIndex);
    WordCB wordCB_;
};
