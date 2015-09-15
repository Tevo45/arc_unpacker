#include "err.h"
#include "fmt/entis/sound/lossless.h"
#include "fmt/entis/common/huffman_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis;
using namespace au::fmt::entis::sound;

static bstr decode_chunk_pcm8(
    const MioHeader &header, const MioChunk &chunk, common::Decoder &decoder)
{
    throw err::NotSupportedError("PCM8 is not supported");
}

static bstr decode_chunk_pcm16(
    const MioHeader &header, const MioChunk &chunk, common::Decoder &decoder)
{
    auto sample_count = chunk.sample_count;
    auto channel_count = header.channel_count;
    auto total_sample_count = sample_count * channel_count;

    if (chunk.initial)
        decoder.reset();

    bstr decoded(total_sample_count * sizeof(s16));
    decoder.set_input(chunk.data);
    decoder.decode(decoded.get<u8>(), decoded.size());

    bstr mixed(total_sample_count * sizeof(s16));
    for (auto i : util::range(channel_count))
    {
        auto offset = i * sample_count * sizeof(s16);
        u8 *source1_ptr = decoded.get<u8>() + offset;
        u8 *source2_ptr = source1_ptr + sample_count;
        u8 *target_ptr = mixed.get<u8>() + offset;
        for (auto j : util::range(sample_count))
        {
            s8 bytLow = source2_ptr[j];
            s8 bytHigh = source1_ptr[j];
            target_ptr[j * sizeof(s16) + 0] = bytLow;
            target_ptr[j * sizeof(s16) + 1] = bytHigh ^ (bytLow >> 7);
        }
    }

    bstr output(total_sample_count * sizeof(s16));
    auto source_ptr = mixed.get<s16>();
    auto step = channel_count;
    for (auto i : util::range(channel_count))
    {
        auto target_ptr = output.get<s16>() + i;
        s16 value = 0;
        s16 delta = 0;
        for (auto j : util::range(sample_count))
        {
            delta += *source_ptr++;
            value += delta;
            *target_ptr = value;
            target_ptr += step;
        }
    }

    return output;
}

struct LosslessSoundDecoder::Priv final
{
    const MioHeader &header;
    common::HuffmanDecoder decoder;

    Priv(const MioHeader &header);
};

LosslessSoundDecoder::Priv::Priv(const MioHeader &header) : header(header)
{
}

LosslessSoundDecoder::LosslessSoundDecoder(const MioHeader &header)
    : p(new Priv(header))
{
    if (header.architecture != common::Architecture::RunLengthHuffman)
        throw err::CorruptDataError("Expected Huffman decoder");
}

LosslessSoundDecoder::~LosslessSoundDecoder()
{
}

bstr LosslessSoundDecoder::process_chunk(const MioChunk &chunk)
{
    if (p->header.bits_per_sample == 16)
        return decode_chunk_pcm16(p->header, chunk, p->decoder);

    if (p->header.bits_per_sample == 8)
        return decode_chunk_pcm8(p->header, chunk, p->decoder);

    throw err::UnsupportedBitDepthError(p->header.bits_per_sample);
}
