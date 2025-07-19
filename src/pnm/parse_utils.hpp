#pragma once

#include <string>
#include <vector>
#include <istream>
#include <optional>
#include <cctype>
#include <cstdint>
#include <utility>

//#include <pnm/Header.hpp>

static std::string read_comment(std::istream &is) {

    std::string comment;
    if (is.peek() != '#') return comment;

    for (int ch = is.get(); is; ch = is.get()) {
        if (ch == '\r' || ch == '\n') return is.putback(ch), comment;
        comment.push_back(char(ch));
    }

    return comment;
}

static void chomp_comments(std::istream &is) {
    while (!read_comment(is).empty());
}

static void chomp_whitespaces(std::istream &is) {
    while (is && std::isspace(is.peek()))
        is.get();
}

static void chomp_comments_and_spaces(std::istream &is) {
    for (int ch = is.peek(); ch == '#' || std::isspace(ch); ch = is.peek())
        chomp_whitespaces(is), chomp_comments(is);
}


static std::optional<pnm::Format> magic_to_enum(char magic[2]) {

    if (magic[0] != 'P') goto fail;
    switch (magic[1]) {
        case '1': return pnm::Format::PBM1;
        case '2': return pnm::Format::PGM2;
        case '3': return pnm::Format::PPM3;
        case '4': return pnm::Format::PBM4;
        case '5': return pnm::Format::PGM5;
        case '6': return pnm::Format::PPM6;
    }

    fail:
    return std::nullopt;
}

static pnm::Format parse_magic_section(std::istream &is) {

    char format[2]{};
    if (!is.read(format, sizeof format))
        throw std::runtime_error{"Missing magic number"};

    std::optional<pnm::Format> magic = magic_to_enum(format);
    if (!magic) throw std::runtime_error{"Invalid magic"};
    return magic.value();
}


static uint16_t parse_width_section(std::istream &is) {

    uint16_t width;
    if (!(is >> width))
        throw std::runtime_error{"missing or invalid width"};

    return width;
}


static uint16_t parse_height_section(std::istream &is) {

    uint16_t height;
    if (!(is >> height))
        throw std::runtime_error{"missing or invalid height"};

    return height;
}

// PGM and PPM only
static uint16_t parse_maxvalue_section(std::istream &is) {

    uint16_t maxvalue;
    if (!(is >> maxvalue) || !maxvalue) // "The maximum color value (Maxval). Must be less than 65536 and more than zero."
        throw std::runtime_error{"missing or invalid maxvalue"};

    if (maxvalue != 255)
        throw std::runtime_error{"a maxvalue different than 255 is not currently supported by opnm"};

    return maxvalue;
}


static std::vector<uint8_t> read_pixmap_blk(std::istream &is, uint32_t expected_bsize) {

    std::vector<uint8_t> pixmap;
    pixmap.resize(expected_bsize);
    is.read((char*)pixmap.data(), expected_bsize);
    pixmap.resize(is.gcount()); // truncate

    if (pixmap.size() < expected_bsize)
        throw std::runtime_error{"Invalid pixmap size, expected to be at least "s + std::to_string(expected_bsize) + " bytes, found " + std::to_string(pixmap.size()) };

    return pixmap;
}


static std::vector<uint8_t> parse_pbm4_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();

    // "A raster of Height rows, in order from top to bottom. Each row is Width bits, packed 8 to a byte"
    const uint16_t row_bytes = ceil_div(width, 8);
    uint32_t expected_bsize = row_bytes * height;

    return read_pixmap_blk(is, expected_bsize);
}

static std::vector<uint8_t> parse_pgm5_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();

    // "ach row consists of Width gray values, in order from left to right. Each gray value is a number from 0 through Maxval, with 0 being black and Maxval being white"
    // "If the Maxval is less than 256, it is 1 byte."

    uint32_t expected_bsize = 1 * width * height; // 1 byte per pixel
    return read_pixmap_blk(is, expected_bsize);
}


static std::vector<uint8_t> parse_ppm6_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();

    // "Each row consists of Width pixels, in order from left to right. Each pixel is a triplet of red, green, and blue samples, in that order."
    // "If the Maxval is less than 256, it is 1 byte."

    uint32_t expected_bsize = 3 * width * height; // 3 byte per pixel
    return read_pixmap_blk(is, expected_bsize);
}

static std::vector<uint8_t> read_ascii_pixmap_values_as_bytes(std::istream &is, uint32_t expected_bsize) {

    std::vector<uint8_t> pixmap;
    pixmap.reserve(expected_bsize);

    for (uint64_t i = 0, ch; i < expected_bsize; i++) {
        if (!(is >> ch)) throw std::runtime_error{"I/O error, missing or incomplete pixmap storing value pixmap[i="s + std::to_string(i) + "]"s};
        pixmap.push_back((uint8_t)ch);
    }

    return pixmap;
}



static std::vector<uint8_t> parse_pbm1_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();
    return read_ascii_pixmap_values_as_bytes(is, width * height);
}


static std::vector<uint8_t> parse_pgm2_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();
    return read_ascii_pixmap_values_as_bytes(is, width * height);
}


static std::vector<uint8_t> parse_ppm3_raster_section(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();
    return read_ascii_pixmap_values_as_bytes(is, width * height * 3); // 3 values per pixel
}
