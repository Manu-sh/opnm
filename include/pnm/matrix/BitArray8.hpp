#pragma once
#include <cstdint>
#include <cassert>

struct __attribute__((__packed__)) BitArray8 {

    private:
        inline BitArray8 & set_bit(uint8_t i) {
            data |= (1 << (7-i));
            return *this;
        }

        inline BitArray8 & clear_bit(uint8_t i) {
            data &= ~(1 << (7-i));
            return *this;
        }

    public:
        inline bool operator[](uint8_t i) const {
            assert(i < 8);
            return (data >> (7 - i)) & 1;
        }

        inline void operator()(uint8_t i, bool value) {
            return (void)(value ? this->set_bit(i) : this->clear_bit(i));
        }

    private:
        uint8_t data;
};

static_assert(sizeof(BitArray8) == 1, "BitArray8 must be exactly 1 byte");
