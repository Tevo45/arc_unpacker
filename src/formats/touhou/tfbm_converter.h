#ifndef FORMATS_TOUHOU_TFBM_CONVERTER_H
#define FORMATS_TOUHOU_TFBM_CONVERTER_H
#include "formats/converter.h"
#include "formats/touhou/palette.h"

namespace Formats
{
    namespace Touhou
    {
        class TfbmConverter : public Converter
        {
        public:
            TfbmConverter();
            ~TfbmConverter();
            void set_palette_map(const PaletteMap &palette_map);
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
