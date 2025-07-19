#pragma once
#include <cstdint>
#include <type_traits>
#include <istream>
#include <string>
#include <stdexcept>

#include <pnm/pnm.hpp>
#include <pnm/pixels/rgb.hpp>

#include <pnm/common.hpp>
#include <pnm/matrix/StandardMatrix1D.hpp>

using std::literals::string_literals::operator""s;

// https://en.wikipedia.org/wiki/Netpbm

template <>
struct PNM<pnm::rgb<pnm::BIT_8>>: private StandardMatrix1D<pnm::rgb<pnm::BIT_8>> {

    using RGB = pnm::rgb<pnm::BIT_8>;
    using StandardMatrix1D<RGB>::StandardMatrix1D;

    static_assert(sizeof(RGB{}.r) + sizeof(RGB{}.g) + sizeof(RGB{}.b) == 3, "only 8 bit depth is supported, the size of each channel MUST be exactly 1 byte");


    static PNM<RGB> parse(std::istream &is);

    uint16_t height() const noexcept { return StandardMatrix1D::height(); }
    uint16_t  width() const noexcept { return StandardMatrix1D::width();  }
    uint32_t length() const noexcept { return StandardMatrix1D::length(); }
    uint64_t  bsize() const noexcept { return StandardMatrix1D::bsize();  }

    const RGB * unwrap() const noexcept { return StandardMatrix1D::unwrap(); }
          RGB * unwrap()       noexcept { return StandardMatrix1D::unwrap(); }

    const RGB & operator()(uint16_t r, uint16_t c) const noexcept { return StandardMatrix1D::operator()(r, c); }
          RGB & operator()(uint16_t r, uint16_t c)       noexcept { return StandardMatrix1D::operator()(r, c); }

    const PNM & write_file_content(const char *const file_name, bool use_ascii_fmt = 0) const {
        return use_ascii_fmt ? write_ascii(file_name) : write_binary(file_name);
    }

    private:
        // this is used by parser() helper function, the reason is that gcc seems doesn't handle well copy-elision unless i write return PNM<..>{} directly
        PNM(uint16_t width, uint16_t height, std::istream &is, bool is_ascii): StandardMatrix1D{width, height} {

            const auto &binary_load = [this] (std::istream &is) { // Load P6 pixmap directly into memory
                is.read((char *)this->unwrap(), this->bsize());
                if (uint32_t(is.gcount()) != this->bsize())
                    throw std::runtime_error{"Invalid pixmap size, expected to be at least "s + std::to_string(this->bsize()) + " bytes found "s + std::to_string(is.gcount()) };
            };

            const auto &ascii_load = [this] (std::istream &is) { // Load P3 data from a stream
                for (int r = 0; r < this->height(); r++) {
                    for (int c = 0; c < this->width(); c++) {
                        uint8_t *rgb = (uint8_t *)&this[0](r, c); // pointer to rgb triple (3 byte)
                        for (uint8_t i = 0; i < sizeof(RGB); ++i) {
                            if (!(is >> rgb[i])) throw std::runtime_error{"I/O error, missing or incomplete pixmap storing pixel pixmap[r="s + std::to_string(r) + "][c=" + std::to_string(c) + "]"s};
                        }
                    }
                }
            };

            is_ascii ? ascii_load(is) : binary_load(is);
        }


    protected:
        const PNM & write_ascii(const char *const file_name) const;
        const PNM & write_binary(const char *const file_name) const;
};