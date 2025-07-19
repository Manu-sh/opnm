#pragma once
#include <cstdint>
#include <type_traits>
#include <istream>
#include <string>
#include <stdexcept>

#include <pnm/pnm.hpp>
#include <pnm/pixels/grayscale.hpp>

#include <pnm/common.hpp>
#include <pnm/matrix/StandardMatrix1D.hpp>


using std::literals::string_literals::operator""s;


template <>
struct PNM<pnm::grayscale<pnm::BIT_8>>: private StandardMatrix1D<pnm::grayscale<pnm::BIT_8>> {

    using GrayScale = pnm::grayscale<pnm::BIT_8>;
    using StandardMatrix1D<GrayScale>::StandardMatrix1D;

    static_assert(sizeof(GrayScale{}.data) == 1, "only 8 bit depth is supported, the size of each channel MUST be exactly 1 byte");

    static PNM<GrayScale> parse(std::istream &is);

    uint16_t height() const noexcept { return StandardMatrix1D::height(); }
    uint16_t  width() const noexcept { return StandardMatrix1D::width();  }
    uint32_t length() const noexcept { return StandardMatrix1D::length(); }
    uint64_t  bsize() const noexcept { return StandardMatrix1D::bsize();  }

    const GrayScale * unwrap() const noexcept { return StandardMatrix1D::unwrap(); }
          GrayScale * unwrap()       noexcept { return StandardMatrix1D::unwrap(); }

    const GrayScale & operator()(uint16_t r, uint16_t c) const noexcept { return StandardMatrix1D::operator()(r, c); }
          GrayScale & operator()(uint16_t r, uint16_t c)       noexcept { return StandardMatrix1D::operator()(r, c); }

    const PNM & write_file_content(const char *const file_name, bool use_ascii_fmt = 0) const {
        return use_ascii_fmt ? write_ascii(file_name) : write_binary(file_name);
    }

    private:
        // this is used by parser() helper function, the reason is that gcc seems doesn't handle well copy-elision unless i write return PNM<..>{} directly
        PNM(uint16_t width, uint16_t height, std::istream &is, bool is_ascii): StandardMatrix1D{width, height} {

            const auto &binary_load = [this] (std::istream &is) { // Load P5 pixmap directly into memory
                is.read((char *)this->unwrap(), this->bsize());
                if (uint32_t(is.gcount()) != this->bsize())
                    throw std::runtime_error{"Invalid pixmap size, expected to be at least "s + std::to_string(this->bsize()) + " bytes found " + std::to_string(is.gcount()) };
            };

            const auto &ascii_load = [this] (std::istream &is) { // Load P2 data from a stream
                for (int r = 0; r < this->height(); r++) {
                    for (int c = 0; c < this->width(); c++)
                        if (!(is >> this[0](r, c).data))
                            throw std::runtime_error{"I/O error, missing or incomplete pixmap storing pixel pixmap[r="s + std::to_string(r) + "][c="  + std::to_string(c) + "]"s};
                }
            };

            is_ascii ? ascii_load(is) : binary_load(is);
        }

    protected:
        const PNM & write_ascii(const char *const file_name) const;
        const PNM & write_binary(const char *const file_name) const;
};