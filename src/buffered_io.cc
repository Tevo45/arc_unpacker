#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "buffered_io.h"
#include "logger.h"

struct BufferedIO::Internals
{
    char *buffer;
    size_t buffer_size;
    size_t buffer_pos;

    Internals(const char *buffer, size_t buffer_size)
    {
        this->buffer = (char*)malloc(buffer_size);
        if (this->buffer == nullptr)
            throw std::bad_alloc();
        memcpy(this->buffer, buffer, buffer_size);
        this->buffer_pos = 0;
        this->buffer_size = buffer_size;
    }
};

void BufferedIO::seek(size_t offset)
{
    if (offset > internals->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    internals->buffer_pos = offset;
}

void BufferedIO::skip(int offset)
{
    if (internals->buffer_pos + offset > internals->buffer_size)
        throw std::runtime_error("Seeking beyond EOF");
    internals->buffer_pos += offset;
}

void BufferedIO::read(void *destination, size_t length)
{
    assert(destination != nullptr);
    if (internals->buffer_pos + length > internals->buffer_size)
        throw std::runtime_error("Reading beyond EOF");
    memcpy(destination, internals->buffer + internals->buffer_pos, length);
    internals->buffer_pos += length;
}

void BufferedIO::write(const void *source, size_t length)
{
    assert(source != nullptr);
    size_t new_pos = internals->buffer_pos + length;
    if (new_pos > internals->buffer_size)
    {
        char *new_buffer = (char*)realloc(internals->buffer, new_pos);
        if (new_buffer == nullptr)
            throw std::bad_alloc();
        internals->buffer = new_buffer;
        internals->buffer_size = new_pos;
    }
    memcpy(internals->buffer + internals->buffer_pos, source, length);
    internals->buffer_pos += length;
}

void BufferedIO::write_from_io(IO &source, size_t length)
{
    size_t new_pos = internals->buffer_pos + length;
    if (new_pos > internals->buffer_size)
    {
        char *new_buffer = (char*)realloc(internals->buffer, new_pos);
        if (new_buffer == nullptr)
            throw std::bad_alloc();
        internals->buffer = new_buffer;
        internals->buffer_size = new_pos;
    }
    source.read(internals->buffer + internals->buffer_pos, length);
    internals->buffer_pos = new_pos;
}

size_t BufferedIO::tell() const
{
    return internals->buffer_pos;
}

size_t BufferedIO::size() const
{
    return internals->buffer_size;
}

void BufferedIO::truncate(size_t new_size)
{
    if (new_size == 0)
    {
        free(internals->buffer);
        internals->buffer_size = 0;
        internals->buffer_pos = 0;
        internals->buffer = nullptr;
    }
    char *new_buffer = (char*)realloc(internals->buffer, new_size);
    if (new_buffer == nullptr)
        throw std::bad_alloc();
    internals->buffer = new_buffer;
    internals->buffer_size = new_size;
    if (internals->buffer_pos > new_size)
        internals->buffer_pos = new_size;
}

BufferedIO::BufferedIO() : internals(new BufferedIO::Internals("", 0))
{
}

BufferedIO::BufferedIO(const std::string &buffer)
    : internals(new BufferedIO::Internals(buffer.data(), buffer.size()))
{
}

BufferedIO::BufferedIO(const char *buffer, size_t buffer_size)
    : internals(new BufferedIO::Internals(buffer, buffer_size))
{
}

BufferedIO::~BufferedIO()
{
    if (internals != nullptr)
        free(internals->buffer);
}
