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
#include "repository.hpp"
#include "ocr.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>

namespace fs = std::filesystem;

// â”€â”€ Helpers â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
std::string Repository::toLower(const std::string& s)
{
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return r;
}

bool Repository::isTextFile(const std::string& ext)
{
    std::string e = toLower(ext);
    return e == ".txt" || e == ".md";
}

bool Repository::isImageFile(const std::string& ext)
{
    std::string e = toLower(ext);
    return e==".png"||e==".jpg"||e==".jpeg"||
           e==".bmp"||e==".tif"||e==".tiff"||e==".webp";
}

bool Repository::isHtmlFile(const std::string& ext)
{
    std::string e = toLower(ext);
    return e==".html"||e==".htm"||e==".webx";
}

// â”€â”€ Constructor â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
Repository::Repository(const std::string& rootDir) : rootDir_(rootDir) {}

// â”€â”€ scan â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool Repository::scan()
{
    authors_.clear();
    if (!fs::exists(rootDir_)) {
        std::cerr << "[Repo] Root not found: " << rootDir_ << "\n";
        return false;
    }

    for (auto& de : fs::directory_iterator(rootDir_)) {
        if (!de.is_directory()) continue;
        Author a;
        a.name = de.path().filename().string();

        for (auto& fe : fs::directory_iterator(de.path())) {
            if (!fe.is_regular_file()) continue;
            std::string ext = fe.path().extension().string();
            // Accept text, image, html â€“ skip PDFs (need external conversion)
            if (!isTextFile(ext) && !isImageFile(ext) && !isHtmlFile(ext))
                continue;
            Book b;
            b.path  = fe.path().string();
            b.title = fe.path().stem().string();
            a.books.push_back(b);
        }

        if (!a.books.empty()) {
            std::sort(a.books.begin(), a.books.end(),
                      [](const Book& x, const Book& y){ return x.title < y.title; });
            authors_.push_back(std::move(a));
        }
    }

    std::sort(authors_.begin(), authors_.end(),
              [](const Author& x, const Author& y){ return x.name < y.name; });

    std::cout << "[Repo] Found " << authors_.size() << " authors\n";
    return true;
}

// â”€â”€ loadBookText â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
std::string Repository::loadBookText(const std::string& path)
{
    std::string ext = fs::path(path).extension().string();

    // â”€â”€ Plain text (preferred path â€“ no OCR) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (isTextFile(ext)) {
        std::ifstream in(path, std::ios::binary);
        if (!in) { std::cerr << "[Repo] Cannot open: " << path << "\n"; return {}; }
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    // â”€â”€ HTML â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (isHtmlFile(ext)) {
        std::ifstream in(path, std::ios::binary);
        if (!in) return {};
        std::ostringstream ss;
        ss << in.rdbuf();
        return stripHtmlTags(ss.str());
    }

    // â”€â”€ Images â†’ Tesseract OCR â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (isImageFile(ext)) {
        static OcrEngine ocr;
        static bool ocrReady = false;
        if (!ocrReady) ocrReady = ocr.init("", "eng");
        if (!ocrReady) {
            std::cerr << "[Repo] OCR unavailable for: " << path << "\n";
            return {};
        }
        return ocr.extractText(path);
    }

    std::cerr << "[Repo] Unsupported file type: " << path << "\n";
    return {};
}

// â”€â”€ stripHtmlTags â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
std::string Repository::stripHtmlTags(const std::string& html)
{
    std::string out;
    out.reserve(html.size());
    bool inTag = false;
    for (std::size_t i = 0; i < html.size(); ++i) {
        char c = html[i];
        if (c == '<')       { inTag = true;  continue; }
        if (c == '>')       { inTag = false; out += ' '; continue; }
        if (!inTag)         out += c;
    }
    // Collapse multiple spaces / newlines
    std::string clean;
    clean.reserve(out.size());
    bool lastSpace = false;
    for (char c : out) {
        if (std::isspace((unsigned char)c)) {
            if (!lastSpace) { clean += '\n'; lastSpace = true; }
        } else {
            clean += c;
            lastSpace = false;
        }
    }
    return clean;
}

// â”€â”€ paginate â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
std::vector<PageLocation> Repository::paginate(const std::string& text,
                                                std::size_t linesPerPage)
{
    std::vector<PageLocation> pages;
    if (text.empty()) return pages;

    // Look for explicit "Page N" or "--- N ---" markers first
    static const std::regex pageMarker(
        R"((?:Page|PAGE)\s+(\d+)|---\s*(\d+)\s*---)",
        std::regex::icase);

    auto begin = std::sregex_iterator(text.begin(), text.end(), pageMarker);
    auto end   = std::sregex_iterator();

    if (begin != end) {
        // Explicit markers found â€“ use them
        std::size_t prev = 0;
        int pageNum = 1;
        for (auto it = begin; it != end; ++it) {
            std::size_t pos = static_cast<std::size_t>(it->position());
            if (pos > prev) {
                PageLocation p;
                p.startIndex = prev;
                p.length     = pos - prev;
                p.pageNumber = pageNum++;
                pages.push_back(p);
            }
            prev = pos;
        }
        if (prev < text.size()) {
            PageLocation p;
            p.startIndex = prev;
            p.length     = text.size() - prev;
            p.pageNumber = pageNum;
            pages.push_back(p);
        }
        return pages;
    }

    // Auto-paginate at linesPerPage line boundaries
    std::size_t lineCount = 0;
    std::size_t pageStart = 0;
    int pageNum = 1;

    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            ++lineCount;
            if (lineCount >= linesPerPage) {
                PageLocation p;
                p.startIndex = pageStart;
                p.length     = (i + 1) - pageStart;
                p.pageNumber = pageNum++;
                pages.push_back(p);
                pageStart  = i + 1;
                lineCount  = 0;
            }
        }
    }

    if (pageStart < text.size()) {
        PageLocation p;
        p.startIndex = pageStart;
        p.length     = text.size() - pageStart;
        p.pageNumber = pageNum;
        pages.push_back(p);
    }

    return pages;
}
