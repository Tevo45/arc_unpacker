#ifndef AU_UTIL_LOG_H
#define AU_UTIL_LOG_H
#include <memory>
#include <string>

namespace au {
namespace util {

    class Logger
    {
    public:
        enum class Color : unsigned char
        {
            Black,
            Navy,
            Green,
            Teal,
            Maroon,
            Purple,
            Brown,
            Grey,
            DarkGrey,
            Blue,
            Lime,
            Cyan,
            Red,
            Magenta,
            Yellow,
            White,

            Original
        };

        Logger();
        ~Logger();
        void set_color(Color c);
        void info(const std::string &str);
        void success(const std::string &str);
        void warn(const std::string &str);
        void err(const std::string &str);
        void flush();
        void mute();
        void unmute();
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }

#endif