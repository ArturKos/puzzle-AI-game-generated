cmake_minimum_required(VERSION 3.16)
project(puzzle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

# ── Collect resource files ────────────────────────────────────────────
file(GLOB RES_IMG_KIDS   CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/RES/img_kids/*")
file(GLOB RES_IMG_ADULTS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/RES/img_adults/*")
file(GLOB RES_SOUNDS     CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/RES/sounds/*")
set(ALL_RESOURCES ${RES_IMG_KIDS} ${RES_IMG_ADULTS} ${RES_SOUNDS})

# ── Generate embedded resource C++ files ──────────────────────────────
set(EMBED_DIR "${CMAKE_BINARY_DIR}/generated")
set(EMBED_SOURCES
    ${EMBED_DIR}/embedded_img_kids.cpp
    ${EMBED_DIR}/embedded_img_adults.cpp
    ${EMBED_DIR}/embedded_sounds.cpp
    ${EMBED_DIR}/embedded_registry.cpp
)
set(EMBED_HEADER ${EMBED_DIR}/embedded_resources.h)

add_custom_command(
    OUTPUT ${EMBED_SOURCES} ${EMBED_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EMBED_DIR}
    COMMAND python3 ${CMAKE_SOURCE_DIR}/cmake/embed_resources.py
        ${EMBED_DIR} ${CMAKE_SOURCE_DIR}/RES ${ALL_RESOURCES}
    DEPENDS ${ALL_RESOURCES} ${CMAKE_SOURCE_DIR}/cmake/embed_resources.py
    COMMENT "Embedding resources into C++ arrays"
)

# ── Build ─────────────────────────────────────────────────────────────
add_executable(puzzle
    src/main.cpp
    src/image_scanner.cpp
    src/puzzle_renderer.cpp
    src/helpers.cpp
    src/scores.cpp
    src/flying_pieces.cpp
    src/ui.cpp
    src/puzzle_generator.cpp
    ${EMBED_SOURCES}
)

target_include_directories(puzzle PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${EMBED_DIR}
    ${CAIRO_INCLUDE_DIRS}
)

target_link_libraries(puzzle
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    cairo
)
