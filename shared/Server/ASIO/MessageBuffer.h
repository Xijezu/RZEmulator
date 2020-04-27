#pragma once
/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <vector>

class MessageBuffer
{
    typedef std::vector<uint8_t>::size_type size_type;

public:
    MessageBuffer();
    ~MessageBuffer() = default;

    explicit MessageBuffer(std::size_t initialSize);
    MessageBuffer(MessageBuffer const &right);
    MessageBuffer(MessageBuffer &&right);

    void Reset();
    void Resize(size_type bytes);

    uint8_t *GetBasePointer() { return _storage.data(); }
    uint8_t *GetReadPointer() { return GetBasePointer() + _rpos; }
    uint8_t *GetWritePointer() { return GetBasePointer() + _wpos; }

    void ReadCompleted(size_type bytes) { _rpos += bytes; }
    void WriteCompleted(size_type bytes) { _wpos += bytes; }

    size_type GetActiveSize() const { return _wpos - _rpos; }
    size_type GetRemainingSpace() const { return _storage.size() - _wpos; }
    size_type GetBufferSize() const { return _storage.size(); }

    // Discards inactive data
    void Normalize();
    // Ensures there's "some" free space, make sure to call Normalize() before this
    void EnsureFreeSpace();

    void Write(void const *data, std::size_t size);
    std::vector<uint8_t> &&Move();

    MessageBuffer &operator=(MessageBuffer const &right)
    {
        if (this != &right)
        {
            _wpos = right._wpos;
            _rpos = right._rpos;
            _storage = right._storage;
        }

        return *this;
    }

    MessageBuffer &operator=(MessageBuffer &&right)
    {
        if (this != &right)
        {
            _wpos = right._wpos;
            _rpos = right._rpos;
            _storage = right.Move();
        }

        return *this;
    }

private:
    size_type _wpos;
    size_type _rpos;
    std::vector<uint8_t> _storage;
};