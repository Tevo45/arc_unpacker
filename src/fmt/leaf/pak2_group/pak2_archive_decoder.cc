#include "fmt/leaf/pak2_group/pak2_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Pak2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("pak"))
        return false;
    const auto file_count = input_file.stream.seek(0x1C).read_u16_le();
    if (file_count < 2)
        return false;
    const auto value1 = input_file.stream.seek(0x34).read_u32_le();
    const auto value2 = input_file.stream.seek(0x3C).read_u32_le();
    for (const auto i : util::range(file_count))
    {
        if (input_file.stream.seek(0x34 + i * 0x20).read_u32_le() != value1)
            return false;
        if (input_file.stream.seek(0x3C + i * 0x20).read_u32_le() != value2)
            return false;
    }
    return true;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak2ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.seek(0x1C).read_u16_le();
    const auto data_offset = 0x20 + 0x20 * file_count;
    input_file.stream.seek(0x20);
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        input_file.stream.skip(2);
        auto name = input_file.stream.read(12);
        for (auto &c : name)
            c = (c << 4) | (c >> 4);
        entry->name = name.str(true);
        if (entry->name.empty())
            continue;
        input_file.stream.skip(2);
        entry->offset = input_file.stream.read_u32_le() + data_offset;
        input_file.stream.skip(4);
        entry->size = input_file.stream.read_u32_le();
        input_file.stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak2ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(e.name, data);
}

std::vector<std::string> Pak2ArchiveDecoder::get_linked_formats() const
{
    return
    {
        "leaf/pak2-compressed-file",
        "leaf/pak2-audio",
        "leaf/pak2-image",
        "leaf/pak2-texture",
    };
}

static auto dummy = fmt::register_fmt<Pak2ArchiveDecoder>("leaf/pak2");
