#include "fmt/lilim/scr_file_decoder.h"
#include "io/bit_reader.h"

using namespace au;
using namespace au::fmt::lilim;

static int init_huffman(io::BitReader &bit_reader, u16 nodes[2][512], int &size)
{
    if (!bit_reader.get(1))
        return bit_reader.get(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman(bit_reader, nodes, size);
    nodes[1][pos] = init_huffman(bit_reader, nodes, size);
    return pos;
}

static bstr decode_huffman(const bstr &input, const size_t target_size)
{
    bstr output;
    output.reserve(input.size() * 2);
    io::BitReader bit_reader(input);
    u16 nodes[2][512];
    auto size = 256;
    auto root = init_huffman(bit_reader, nodes, size);
    while (output.size() < target_size && !bit_reader.eof())
    {
        auto byte = root;
        while (byte >= 256 && byte <= 511)
            byte = nodes[bit_reader.get(1)][byte];
        output += static_cast<const u8>(byte);
    }
    return output;
}

bool ScrFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("scr");
}

std::unique_ptr<io::File> ScrFileDecoder::decode_impl(
    io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto size_orig = input_file.stream.read_u32_le();
    auto data = input_file.stream.read_to_eof();
    data = decode_huffman(data, size_orig);
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("txt");
    return output_file;
}

static auto dummy = fmt::register_fmt<ScrFileDecoder>("lilim/scr");
