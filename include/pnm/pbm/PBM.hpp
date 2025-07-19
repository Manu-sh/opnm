#pragma once
#include <cstdint>
#include <type_traits>

#include <pnm/pnm.hpp>
#include <pnm/pixels/monochrome.hpp>

#include <pnm/common.hpp>
#include <pnm/matrix/BitMatrix1D.hpp>

#include <string>

#include <istream>
#include <ostream>
#include <fstream>
#include <ios>

#include <cstring>
#include <stdexcept>

using std::literals::string_literals::operator""s;

template <>
struct PNM<pnm::monochrome_t>: private BitMatrix1D {

    static PNM<pnm::monochrome_t> parse(std::istream &is);

    using BitMatrix1D::BitMatrix1D;
    using BitMatrix1D::operator();

    uint16_t height() const noexcept { return BitMatrix1D::height(); }
    uint16_t  width() const noexcept { return BitMatrix1D::width();  }
    uint32_t length() const noexcept { return BitMatrix1D::length(); }
    uint64_t  bsize() const noexcept { return BitMatrix1D::bsize();  }

    const BitArray8 * unwrap() const noexcept { return BitMatrix1D::unwrap(); }
          BitArray8 * unwrap()       noexcept { return BitMatrix1D::unwrap(); }

    bool operator()(uint16_t r, uint16_t c) const {
        return BitMatrix1D::operator()(r, c);
    }

    void operator()(uint16_t r, uint16_t c, pnm::monochrome_t px) {
        BitMatrix1D::operator()(r, c, px.data);
    }

    const PNM & write_content(std::ostream &os, bool use_ascii_fmt = 0) const {
        return use_ascii_fmt ? write_ascii(os) : write_binary(os);
    }

    const PNM & write_file_content(const char *file_name, bool use_ascii_fmt = 0) const {
        using std::ios_base;
        std::ofstream fPPM;
        fPPM.exceptions(ios_base::failbit|ios_base::badbit);
        fPPM.open(file_name, ios_base::out|ios_base::binary|ios_base::trunc);
        return write_content(fPPM, use_ascii_fmt);
    }

private:
    // this is used by parser() helper function, the reason is that gcc seems doesn't handle well copy-elision unless i write return PNM<..>{} directly
    PNM(uint16_t width, uint16_t height, std::istream &is, bool is_ascii): BitMatrix1D{width, height} {

        const auto &binary_load = [this] (std::istream &is) { // Load P4 pixmap directly into memory

            is.read((char *)this->unwrap(), this->bsize()); // "A raster of Height rows, in order from top to bottom. Each row is Width bits, packed 8 to a byte"

            if (uint32_t(is.gcount()) != this->bsize())
                throw std::runtime_error{"Invalid pixmap size, expected to be at least "s + std::to_string(this->bsize()) + " bytes found " + std::to_string(is.gcount()) };
        };

        const auto &ascii_load = [this] (std::istream &is) { // Load P1 data from a stream
            for (int r = 0, ch; r < this->height(); r++) {
                for (int c = 0; c < this->width(); c++) {
                    if (!(is >> ch)) throw std::runtime_error{"I/O error, missing or incomplete pixmap storing pixel pixmap[r="s + std::to_string(r) + "][c="  + std::to_string(c) + "]"s};
                    if (ch != 0 && ch != 1) throw std::runtime_error{"Invalid pixmap value ("s + std::to_string(ch) + ") pixmap[r="s + std::to_string(r) + "][c="  + std::to_string(c) + "]"s};
                    this->operator()(r, c, ch);
                }
            }
        };

        is_ascii ? ascii_load(is) : binary_load(is);
    }


protected:
    const PNM & write_ascii(std::ostream &os) const;
    const PNM & write_binary(std::ostream &os) const;
};
