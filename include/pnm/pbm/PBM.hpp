#pragma once
#include <cstdint>
#include <type_traits>

#include <pnm/pnm.hpp>
#include <pnm/pixels/monochrome.hpp>

#include <pnm/common.hpp>
#include <pnm/matrix/BitMatrix1D.hpp>

#include <string>
#include <istream>
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

    const PNM & write_file_content(const char *const file_name, bool use_ascii_fmt = 0) const {
        return use_ascii_fmt ? write_ascii(file_name) : write_binary(file_name);
    }

#if 0
    // TODO: nell'aprire un file ascii sarà utile data la completamente diversa rappresentazione in memoria creare un nuovo file PNM quindi settare i pixel uno ad uno
    //  mentre si parsa la pixmap in formato ascii, tecnicamente i commenti se non si cambia formato andrebbero mantenuti, quindi l'header da scrivere dovrebbe essere lo stesso
    //  ricevuto in input tanto è già stato validato una volta che la pixmap arriva

    // Load P4 pixmap directly into memory
    PNM(uint16_t width, uint16_t height, const std::vector<uint8_t> &pixmap): BitMatrix1D{width, height} {

        // "A raster of Height rows, in order from top to bottom. Each row is Width bits, packed 8 to a byte"
        const uint16_t row_bytes = ceil_div(width, 8);
        uint32_t expected_bsize = row_bytes * height;
        if (pixmap.size() < expected_bsize)
            throw std::runtime_error{"Invalid pixmap size, expected to be at least "s + std::to_string(expected_bsize) + " bytes found " + std::to_string(pixmap.size()) };

        // copy the pixmap into memory
        if (this->bsize() != expected_bsize)
            throw std::runtime_error{"Fatal error"};

        memcpy(this->unwrap(), (const void *)pixmap.data(), expected_bsize);
    }
#endif

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
                    this->operator()(r, c, {(uint8_t)ch, (uint8_t)ch, (uint8_t)ch});
                }
            }
        };

        is_ascii ? ascii_load(is) : binary_load(is);
    }


protected:
        const PNM & write_ascii(const char *const file_name) const;
        const PNM & write_binary(const char *const file_name) const;
};