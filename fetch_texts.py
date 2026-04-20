# =============================================================================
#  Fireside Reader fetch_texts.py
#  Copyright (C) 2025  Christopher T. Williams  All Rights Reserved
#
#  Distributed under GPL-3.0-or-later (eSpeak-NG linkage requirement).
#  See LICENSE for full terms and third-party dependency notices.
#
#  Designed and authored by: Christopher T. Williams
# =============================================================================
#!/usr/bin/env python3
"""
fetch_texts.py
==============
Scrapes public-domain speeches and documents by the U.S. Founding Fathers
from reliable .edu / archive sources and saves them as plain .txt files in

    data/authors/<AuthorName>/<DocumentTitle>.txt

Run from the project root:
    python3 scripts/fetch_texts.py

Dependencies:
    pip install requests beautifulsoup4 lxml
"""

import re
import sys
import time
import pathlib
import unicodedata
import textwrap

try:
    import requests
    from bs4 import BeautifulSoup
except ImportError:
    sys.exit("Install deps:  pip install requests beautifulsoup4 lxml")

# Output root
OUT_ROOT = pathlib.Path("data/authors")
DELAY    = 1.2   # seconds between requests (be a good citizen)

# Source catalogue
# Format: (author_name, document_title, url, content_selector_css)
# Sources: Avalon Project (Yale Law), Founders Online (UVA/NARA), Library of Congress
SOURCES = [

    # George Washington
    ("George Washington",
     "Farewell Address 1796",
     "https://avalon.law.yale.edu/18th_century/washing.asp",
     "div.document"),

    ("George Washington",
     "First Inaugural Address 1789",
     "https://avalon.law.yale.edu/18th_century/wash1.asp",
     "div.document"),

    ("George Washington",
     "Second Inaugural Address 1793",
     "https://avalon.law.yale.edu/18th_century/wash2.asp",
     "div.document"),

    # Thomas Jefferson
    ("Thomas Jefferson",
     "Declaration of Independence 1776",
     "https://avalon.law.yale.edu/18th_century/declare.asp",
     "div.document"),

    ("Thomas Jefferson",
     "First Inaugural Address 1801",
     "https://avalon.law.yale.edu/19th_century/jefinau1.asp",
     "div.document"),

    ("Thomas Jefferson",
     "Second Inaugural Address 1805",
     "https://avalon.law.yale.edu/19th_century/jefinau2.asp",
     "div.document"),

    ("Thomas Jefferson",
     "Notes on the State of Virginia - Query XIV",
     "https://avalon.law.yale.edu/18th_century/jeffvir.asp",
     "div.document"),

    # James Madison
    ("James Madison",
     "Federalist No 10",
     "https://avalon.law.yale.edu/18th_century/fed10.asp",
     "div.document"),

    ("James Madison",
     "Federalist No 51",
     "https://avalon.law.yale.edu/18th_century/fed51.asp",
     "div.document"),

    ("James Madison",
     "Memorial and Remonstrance Against Religious Assessments 1785",
     "https://avalon.law.yale.edu/18th_century/madison.asp",
     "div.document"),

    # Alexander Hamilton
    ("Alexander Hamilton",
     "Federalist No 1",
     "https://avalon.law.yale.edu/18th_century/fed01.asp",
     "div.document"),

    ("Alexander Hamilton",
     "Federalist No 9",
     "https://avalon.law.yale.edu/18th_century/fed09.asp",
     "div.document"),

    ("Alexander Hamilton",
     "Federalist No 70",
     "https://avalon.law.yale.edu/18th_century/fed70.asp",
     "div.document"),

    ("Alexander Hamilton",
     "Federalist No 78",
     "https://avalon.law.yale.edu/18th_century/fed78.asp",
     "div.document"),

    # Benjamin Franklin
    ("Benjamin Franklin",
     "Autobiography Part I",
     "https://www.gutenberg.org/cache/epub/20203/pg20203.txt",
     None),   # plain-text URL â€“ no HTML parsing needed

    ("Benjamin Franklin",
     "Speech at the Constitutional Convention 1787",
     "https://avalon.law.yale.edu/18th_century/franklin.asp",
     "div.document"),

    ("Benjamin Franklin",
     "Poor Richards Almanack Selections",
     "https://www.gutenberg.org/cache/epub/3928/pg3928.txt",
     None),

    # John Adams
    ("John Adams",
     "Thoughts on Government 1776",
     "https://avalon.law.yale.edu/18th_century/thoughts.asp",
     "div.document"),

    ("John Adams",
     "A Defence of the Constitutions Vol I Preface",
     "https://oll.libertyfund.org/titles/adams-a-defence-of-the-constitutions-of-government-vol-1",
     "div.field-items"),

    ("John Adams",
     "First Inaugural Address 1797",
     "https://avalon.law.yale.edu/18th_century/adam1.asp",
     "div.document"),

    # Patrick Henry
    ("Patrick Henry",
     "Give Me Liberty or Give Me Death 1775",
     "https://avalon.law.yale.edu/18th_century/patrick.asp",
     "div.document"),

    # Thomas Paine
    ("Thomas Paine",
     "Common Sense 1776",
     "https://www.gutenberg.org/cache/epub/147/pg147.txt",
     None),

    ("Thomas Paine",
     "The American Crisis No 1 1776",
     "https://avalon.law.yale.edu/18th_century/crisis1776.asp",
     "div.document"),

    # James Monroe
    ("James Monroe",
     "Monroe Doctrine 1823",
     "https://avalon.law.yale.edu/19th_century/monroe.asp",
     "div.document"),

    # Roger Sherman
    ("Roger Sherman",
     "A Caveat Against Injustice 1752",
     "https://avalon.law.yale.edu/18th_century/sherman.asp",
     "div.document"),
]

# Utilities
HEADERS = {
    "User-Agent": (
        "FiresideReader/1.0 (educational; public-domain text retrieval; "
        "contact: fireside-reader@example.com)"
    )
}

def safe_filename(name: str) -> str:
    """Strip illegal filesystem characters from a name."""
    name = unicodedata.normalize("NFKD", name)
    name = re.sub(r'[\\/:*?"<>|]', "_", name)
    return name.strip()[:120]


def fetch_html_text(url: str, selector: str) -> str:
    """Fetch a page and extract inner text from the given CSS selector."""
    r = requests.get(url, headers=HEADERS, timeout=20)
    r.raise_for_status()
    soup = BeautifulSoup(r.text, "lxml")
    el = soup.select_one(selector)
    if el is None:
        # Fall back: take the largest <div> or whole <body>
        el = soup.find("body") or soup
    raw = el.get_text(separator="\n")
    return clean_text(raw)


def fetch_plain_text(url: str) -> str:
    """Download a .txt file directly."""
    r = requests.get(url, headers=HEADERS, timeout=30)
    r.raise_for_status()
    return clean_text(r.text)


def clean_text(raw: str) -> str:
    """Normalise whitespace and remove excessive blank lines."""
    lines = raw.splitlines()
    out   = []
    blank = 0
    for line in lines:
        line = line.rstrip()
        if line:
            out.append(line)
            blank = 0
        else:
            blank += 1
            if blank <= 2:
                out.append("")
    return "\n".join(out) + "\n"


def save(author: str, title: str, text: str) -> pathlib.Path:
    dest = OUT_ROOT / safe_filename(author) / (safe_filename(title) + ".txt")
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(text, encoding="utf-8")
    return dest


# Main
def main() -> None:
    total = len(SOURCES)
    for i, (author, title, url, selector) in enumerate(SOURCES, 1):
        print(f"[{i:02d}/{total}] {author}  {title}")
        try:
            if selector is None:
                text = fetch_plain_text(url)
            else:
                text = fetch_html_text(url, selector)

            if len(text.strip()) < 200:
                print(f"       Very short response ({len(text)} chars) skipping")
                continue

            path = save(author, title, text)
            print(f"  {len(text):,} chars {path}")

        except Exception as exc:
            print(f" —  ERROR: {exc}")

        time.sleep(DELAY)

    print("\nDone.  Texts saved to", OUT_ROOT.resolve())


if __name__ == "__main__":
    main()
