# =============================================================================
#  Fireside Reader CMakeLists.txt
#  Copyright (C) 2025  Christopher T. Williams All Rights Reserved
#
#  Distributed under GPL-3.0-or-later (eSpeak-NG linkage requirement).
#  See LICENSE for full terms and third-party dependency notices.
#
#  Designed and authored by: Christopher T. Williams
# =============================================================================
cmake_minimum_required(VERSION 3.15)
project(FiresideReader CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2
find_package(SDL2 REQUIRED)

# SDL2_ttf  (may not ship a cmake config on all distros)
find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
    HINTS /usr/include/SDL2 /usr/local/include/SDL2)
find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)

# SDL2_mixer
find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
    HINTS /usr/include/SDL2 /usr/local/include/SDL2)
find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer)

# eSpeak-NG
find_path(ESPEAK_INCLUDE_DIR NAMES espeak-ng/speak_lib.h espeak/speak_lib.h
    HINTS /usr/include /usr/local/include)
find_library(ESPEAK_LIBRARY NAMES espeak-ng espeak)

# Tesseract
find_package(Tesseract QUIET)
if(NOT Tesseract_FOUND)
    find_path(TESSERACT_INCLUDE_DIR NAMES tesseract/baseapi.h
        HINTS /usr/include /usr/local/include)
    find_library(TESSERACT_LIBRARY NAMES tesseract)
    find_library(LEPTONICA_LIBRARY NAMES lept leptonica)
    set(Tesseract_INCLUDE_DIRS ${TESSERACT_INCLUDE_DIR})
    set(Tesseract_LIBRARIES    ${TESSERACT_LIBRARY} ${LEPTONICA_LIBRARY})
endif()

# Sources
add_executable(FiresideReader
    src/main.cpp
    src/app.cpp
    src/tts.cpp
    src/ocr.cpp
    src/repository.cpp
    src/fire.cpp
    src/woodui.cpp
)

target_include_directories(FiresideReader PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
    ${ESPEAK_INCLUDE_DIR}
    ${Tesseract_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(FiresideReader
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARY}
    ${SDL2_MIXER_LIBRARY}
    ${ESPEAK_LIBRARY}
    ${Tesseract_LIBRARIES}
)

# Copy assets next to the executable
add_custom_command(TARGET FiresideReader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:FiresideReader>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data   $<TARGET_FILE_DIR:FiresideReader>/data
)
