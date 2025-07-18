#include <pnm/pnm_common.hpp>
#include <pnm/pgm/PGM.hpp>
#include <pnm/Header.hpp>
#include <pnm/pnm.hpp>
#include <pnm/parse_utils.hpp>


#include <cstddef>
#include <cassert>
#include <cstring>
#include <memory>
#include <new>



PNM<pnm::grayscale<pnm::BIT_8>> PNM<pnm::grayscale<pnm::BIT_8>>::parse(std::istream &is) {

    //chomp_comments_and_spaces(is);
    // avoid touching the stream if no magic number is found
    if (is.peek() != 'P')
        throw std::runtime_error{"not a PNM file"};

    auto tmp = is.get();
    if (is.peek() != '2' && is.peek() != '5') {
        is.putback(tmp); // put 'P' back into the stream
        throw std::runtime_error{"not a PGM file"};
    }

    is.putback(tmp); // put 'P' back into the stream
    const auto &format = parse_magic_section(is);

    chomp_comments_and_spaces(is);
    uint16_t width = parse_width_section(is);

    chomp_comments_and_spaces(is);
    uint16_t height = parse_height_section(is);

    chomp_comments_and_spaces(is);
    (void)parse_maxvalue_section(is);

    if (!std::isspace(is.peek()))
        throw std::runtime_error{"Invalid format expected a single whitespace before the raster block begin"};

    is.get();

    switch (format) {
        case pnm::Format::PGM5: return PNM<pnm::grayscale<pnm::BIT_8>>(width, height, is, false);
        case pnm::Format::PGM2: return PNM<pnm::grayscale<pnm::BIT_8>>(width, height, is, true);
        default:; // shut-up warnings
    }

    throw std::runtime_error{"not a PGM file"};
}




const PNM<pnm::grayscale<pnm::BIT_8>> & PNM<pnm::grayscale<pnm::BIT_8>>::write_ascii(const char *const file_name) const {

    if (!m_length) return *this;

    const auto bsize = 4 * m_length + 1; // 4 -> strlen("255 ") + 1 for the null terminator we actually dont use
    std::unique_ptr<uint8_t[]> mem{new uint8_t[bsize]};
    uint8_t *p = mem.get();

    for (uint32_t i = 0; i < m_length; ++i) {
        const pnm::grayscale<pnm::BIT_8> px = this->m_vct[i];
        memcpy(p, map_ascii[px.data], map_length[px.data]), p += map_length[px.data];
    }

    const uint8_t *const beg = mem.get();
    const uint8_t *const end = p;
    assert(end <= &beg[bsize-1]);
    assert((size_t)std::distance(beg, end) <= bsize-1);
    *p = '\0'; // p is writable here

    auto header = pnm::Header<pnm::Format::PGM2, pnm::BIT_8>{m_width, m_height};
    return ::write_file_content(file_name, header, beg, end), *this;
}

const PNM<pnm::grayscale<pnm::BIT_8>> & PNM<pnm::grayscale<pnm::BIT_8>>::write_binary(const char *const file_name) const {

    if (!m_length) return *this;

    const auto bsize = sizeof(*m_vct) * m_length;
    const uint8_t *const beg = ((uint8_t*)(void *)this->m_vct);
    const uint8_t *const end = ((uint8_t*)(void *)(this->m_vct + m_length));

    assert(end <= (beg + bsize));
    assert((size_t)std::distance(beg, end) <= bsize);

    auto header = pnm::Header<pnm::Format::PGM5, pnm::BIT_8>{m_width, m_height};
    return ::write_file_content(file_name, header, beg, end), *this;
}

