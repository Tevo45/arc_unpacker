#ifndef FORMATS_TOUHOU_TFPK_ARCHIVE_H
#define FORMATS_TOUHOU_TFPK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class TfpkArchive final : public Archive
        {
        public:
            TfpkArchive();
            ~TfpkArchive();
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
