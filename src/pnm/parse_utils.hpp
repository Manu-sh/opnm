#pragma once

#include <string>
#include <istream>
#include <optional>
#include <cctype>

//#include <pnm/Header.hpp>

// return an empty string if there is no comment on the current line
// "Before the whitespace character that delimits the raster, any characters from a "#" to but not including the next carriage return or newline character, or end of file, is a comment and is ignored.": https://netpbm.sourceforge.net/doc/pbm.html
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


std::optional<pnm::Format> parse_magic(char magic[2]) {

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
