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
#pragma once
#include <string>
#include <vector>

// â”€â”€ Data structures â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
struct Book {
    std::string title;      // stem of filename
    std::string path;       // full filesystem path
};

struct Author {
    std::string        name;   // directory name
    std::vector<Book>  books;
};

// Half-open byte range inside the loaded text buffer.
struct PageLocation {
    std::size_t startIndex = 0;
    std::size_t length     = 0;
    int         pageNumber = -1;   // -1 = auto-paginated
};

// â”€â”€ Repository â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
class Repository {
public:
    explicit Repository(const std::string& rootDir);

    // Scan rootDir for author sub-directories. Returns false if rootDir missing.
    bool scan();

    const std::vector<Author>& authors() const { return authors_; }

    // Load and decode a single book file.
    // Text files â†’ direct read.
    // Images     â†’ Tesseract OCR.
    // HTML/HTM   â†’ naive tag-strip.
    // PDF/others â†’ skipped (returns empty; see scripts/fetch_texts.py for conversion).
    std::string loadBookText(const std::string& path);

    // Build page table (76 lines per page unless the text embeds
    // explicit "Page N" markers, which we respect).
    std::vector<PageLocation> paginate(const std::string& text,
                                       std::size_t linesPerPage = 76);

private:
    std::string        rootDir_;
    std::vector<Author> authors_;

    static std::string toLower(const std::string& s);
    static bool        isTextFile (const std::string& ext);
    static bool        isImageFile(const std::string& ext);
    static bool        isHtmlFile (const std::string& ext);
    std::string        stripHtmlTags(const std::string& html);
};
