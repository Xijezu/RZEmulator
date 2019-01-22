#include "MessageBuffer.h"
#include "memory.h"

MessageBuffer::MessageBuffer() : _wpos(0), _rpos(0), _storage()
{
    _storage.resize(4096);
}

MessageBuffer::MessageBuffer(std::size_t initialSize) : _wpos(0), _rpos(0), _storage()
{
    _storage.resize(initialSize);
}

MessageBuffer::MessageBuffer(MessageBuffer const &right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right._storage)
{
}

MessageBuffer::MessageBuffer(MessageBuffer &&right) : _wpos(right._wpos), _rpos(right._rpos), _storage(right.Move())
{
}

void MessageBuffer::Reset()
{
    _wpos = 0;
    _rpos = 0;
}

void MessageBuffer::Resize(size_type bytes)
{
    _storage.resize(bytes);
}

void MessageBuffer::Normalize()
{
    if (_rpos)
    {
        if (_rpos != _wpos)
            memmove(GetBasePointer(), GetReadPointer(), GetActiveSize());
        _wpos -= _rpos;
        _rpos = 0;
    }
}

void MessageBuffer::EnsureFreeSpace()
{
    // resize buffer if it's already full
    if (GetRemainingSpace() == 0)
        _storage.resize(_storage.size() * 3 / 2);
}

void MessageBuffer::Write(void const *data, std::size_t size)
{
    if (size)
    {
        memcpy(GetWritePointer(), data, size);
        WriteCompleted(size);
    }
}

std::vector<uint8_t> &&MessageBuffer::Move()
{
    _wpos = 0;
    _rpos = 0;
    return std::move(_storage);
}