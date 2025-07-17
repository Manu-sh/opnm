#pragma once

#include <string>
#include <istream>
#include <optional>
#include <cctype>
#include <cstdint>
#include <utility>

//#include <pnm/Header.hpp>

std::string read_comment(std::istream &is) {

    std::string comment;
    if (is.peek() != '#') return comment;

    for (int ch = is.get(); is; ch = is.get()) {
        if (ch == '\r' || ch == '\n') return is.putback(ch), comment;
        comment.push_back(char(ch));
    }

    return comment;
}

void chomp_comments(std::istream &is) {
    while (!read_comment(is).empty());
}

void chomp_whitespaces(std::istream &is) {
    while (is && std::isspace(is.peek()))
        is.get();
}

void chomp_comments_and_spaces(std::istream &is) {
    for (int ch = is.peek(); ch == '#' || std::isspace(ch); ch = is.peek())
        chomp_whitespaces(is), chomp_comments(is);
}


std::optional<pnm::Format> magic_to_enum(char magic[2]) {

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

pnm::Format parse_magic_section(std::istream &is) {

    char format[2]{};
    if (!is.read(format, sizeof format))
        throw std::runtime_error{"Missing magic number"};

    std::optional<pnm::Format> magic = magic_to_enum(format);
    if (!magic) throw std::runtime_error{"Invalid magic"};
    return magic.value();
}


uint16_t parse_width_section(std::istream &is) {

    uint16_t width;
    if (!(is >> width))
        throw std::runtime_error{"missing or invalid width"};

    return width;
}


uint16_t parse_height_section(std::istream &is) {

    uint16_t height;
    if (!(is >> height))
        throw std::runtime_error{"missing or invalid height"};

    return height;
}

// PGM and PPM only
uint16_t parse_maxvalue_section(std::istream &is) {

    uint16_t maxvalue;
    if (!(is >> maxvalue) || !maxvalue || maxvalue == UINT16_MAX) // "The maximum color value (Maxval). Must be less than 65536 and more than zero."
        throw std::runtime_error{"missing or invalid maxvalue"};

    if (maxvalue != 255)
        throw std::runtime_error{"a maxvalue different than 255 is not currently supported by opnm"};

    return maxvalue;
}


// const auto &[pixmap, garbage] = parse_pbm4_pixmap(fpnm, width, height);
std::pair<std::string, std::string> parse_pbm4_pixmap(std::istream &is, uint16_t width, uint16_t height) {

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();

    // PIXMAP & GARBAGE
    std::ostringstream oss;
    oss << is.rdbuf();

    // "You can put any junk you want after the raster, if it starts with a white space character."
    std::string pixmap_and_garbage{oss.str()};
    if (pixmap_and_garbage.empty())
        throw std::runtime_error{"missing pixmap data"};


    // "A raster of Height rows, in order from top to bottom. Each row is Width bits, packed 8 to a byte"
    const uint16_t row_bytes = ceil_div(width, 8);

    uint32_t expected_bsize = row_bytes * height;
    if (pixmap_and_garbage.size() < expected_bsize)
        throw std::runtime_error{"Invalid pixmap+garbage size, expected to be at least "s + std::to_string(expected_bsize) + " bytes found " + std::to_string(pixmap_and_garbage.size()) };

    // TODO: tecnicamente in fondo dopo uno spazio bianco ci si può mettere qualunque cosa, ciò vuol dire che bisogna usare substring se > expected_bsize
    std::string garbage{pixmap_and_garbage.cbegin() + expected_bsize, pixmap_and_garbage.cend()};
    pixmap_and_garbage.resize(expected_bsize); // remove any garbage data from pixman

    return {pixmap_and_garbage, garbage};
}
