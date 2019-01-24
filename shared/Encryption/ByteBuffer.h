#pragma once
/*
 * Copyright (C) 2011-2017 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2017 MaNGOS <https://www.getmangos.eu/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MdERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MX_TIMEZONE
#if _MSC_VER >= 1900
#define MX_TIMEZONE _timezone
#else
#define MX_TIMEZONE timezone
#endif
#endif

#include "Common.h"
#include "ByteConverter.h"

#include "Errors.h"
#include <exception>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <time.h>
#include "MessageBuffer.h"

// Root of ByteBuffer exception hierarchy
class ByteBufferException : public std::exception
{
  public:
    ~ByteBufferException() throw() {}

    char const *what() const throw() { return msg_.c_str(); }

  protected:
    std::string &message() throw() { return msg_; }

  private:
    std::string msg_;
};

class ByteBufferPositionException : public ByteBufferException
{
  public:
    ByteBufferPositionException(bool add, size_t pos, size_t size, size_t valueSize);

    ~ByteBufferPositionException() throw() {}
};

class ByteBufferSourceException : public ByteBufferException
{
  public:
    ByteBufferSourceException(size_t pos, size_t size, size_t valueSize);

    ~ByteBufferSourceException() throw() {}
};

class ByteBuffer
{
  public:
    constexpr static size_t DEFAULT_SIZE = 0x1000;

    // constructor
    ByteBuffer() : _rpos(0), _wpos(0), _bitpos(8), _curbitval(0)
    {
        _storage.reserve(DEFAULT_SIZE);
    }

    explicit ByteBuffer(size_t reserve) : _rpos(0), _wpos(0), _bitpos(8), _curbitval(0)
    {
        _storage.reserve(reserve);
    }

    // copy constructor
    explicit ByteBuffer(const ByteBuffer &buf) : _rpos(buf._rpos), _wpos(buf._wpos),
                                                 _bitpos(buf._bitpos), _curbitval(buf._curbitval), _storage(buf._storage)
    {
    }

    explicit ByteBuffer(MessageBuffer &&buffer);

    void clear()
    {
        _storage.clear();
        _rpos = _wpos = 0;
    }

    template <typename T>
    void append(T value)
    {
        FlushBits();
        EndianConvert(value);
        append((uint8_t *)&value, sizeof(value));
    }

    void FlushBits()
    {
        if (_bitpos == 8)
            return;

        append((uint8_t *)&_curbitval, sizeof(uint8_t));
        _curbitval = 0;
        _bitpos = 8;
    }

    template <typename T>
    void put(size_t pos, T value)
    {
        EndianConvert(value);
        put(pos, (uint8_t *)&value, sizeof(value));
    }

    /**
        * @name   PutBits
        * @brief  Places specified amount of bits of value at specified position in packet.
        *         To ensure all bits are correctly written, only call this method after
        *         bit flush has been performed

        * @param  pos Position to place the value at, in bits. The entire value must fit in the packet
        *             It is advised to obtain the position using bitwpos() function.

        * @param  value Data to write.
        * @param  bitCount Number of bits to store the value on.
       */
    template <typename T>
    void PutBits(size_t pos, T value, uint32_t bitCount)
    {
        if (!bitCount)
            throw ByteBufferSourceException((pos + bitCount) / 8, size(), 0);

        if (pos + bitCount > size() * 8)
            throw ByteBufferPositionException(false, (pos + bitCount) / 8, size(), (bitCount - 1) / 8 + 1);

        for (uint32_t i = 0; i < bitCount; ++i)
        {
            size_t wp = (pos + i) / 8;
            size_t bit = (pos + i) % 8;
            if ((value >> (bitCount - i - 1)) & 1)
                _storage[wp] |= 1 << (7 - bit);
            else
                _storage[wp] &= ~(1 << (7 - bit));
        }
    }

    // Writing start
    template <typename T>
    ByteBuffer &operator<<(T value)
    {
        append<T>(value);
        return *this;
    }

    ByteBuffer &operator<<(const std::string &value)
    {
        if (size_t len = value.length())
            append((uint8_t const *)value.c_str(), len);
        append((uint8_t)0);
        return *this;
    }

    ByteBuffer &operator<<(const char *str)
    {
        if (size_t len = (str ? strlen(str) : 0))
            append((uint8_t const *)str, len);
        append((uint8_t)0);
        return *this;
    }

    // Reading start
    template <typename T>
    ByteBuffer &operator>>(T &value)
    {
        value = read<T>();
        return *this;
    }

    ByteBuffer &operator>>(std::string &value)
    {
        value.clear();
        while (rpos() < size()) // prevent crash at wrong string format in packet
        {
            auto c = read<char>();
            if (c == 0)
                break;
            value += c;
        }
        return *this;
    }

    uint8_t &operator[](size_t const pos)
    {
        if (pos >= size())
            throw ByteBufferPositionException(false, pos, 1, size());
        return _storage[pos];
    }

    uint8_t const &operator[](size_t const pos) const
    {
        if (pos >= size())
            throw ByteBufferPositionException(false, pos, 1, size());
        return _storage[pos];
    }

    size_t rpos() const { return _rpos; }

    size_t rpos(size_t rpos_)
    {
        _rpos = rpos_;
        return _rpos;
    }

    void rfinish()
    {
        _rpos = wpos();
    }

    size_t wpos() const { return _wpos; }

    size_t wpos(size_t wpos_)
    {
        _wpos = wpos_;
        return _wpos;
    }

    template <typename T>
    void read_skip() { read_skip(sizeof(T)); }

    void read_skip(size_t skip)
    {
        if (_rpos + skip > size())
            throw ByteBufferPositionException(false, _rpos, skip, size());
        _rpos += skip;
    }

    template <typename T>
    T read()
    {
        auto r = read<T>(_rpos);
        _rpos += sizeof(T);
        return r;
    }

    template <typename T>
    T read(size_t pos) const
    {
        if (pos + sizeof(T) > size())
            throw ByteBufferPositionException(false, pos, sizeof(T), size());
        T val = *((T const *)&_storage[pos]);
        EndianConvert(val);
        return val;
    }

    void read(uint8_t *dest, size_t len)
    {
        if (_rpos + len > size())
            throw ByteBufferPositionException(false, _rpos, len, size());
        std::memcpy(dest, &_storage[_rpos], len);
        _rpos += len;
    }

    std::string ReadString(uint32_t length)
    {
        if (!length)
            return std::string();
        char *buffer = new char[length + 1]();
        read((uint8_t *)buffer, length);
        std::string retval = buffer;
        delete[] buffer;
        return retval;
    }

    //! Method for writing strings that have their length sent separately in packet
    //! without null-terminating the string
    void WriteString(std::string const &str)
    {
        if (size_t len = str.length())
            append(str.c_str(), len);
    }

    void fill(const std::string &src, size_t cnt)
    {
        append(src.c_str(), src.length());
        if (src.length() < cnt)
        {
            while (src.length() < cnt)
            {
                append<uint8_t>(0);
                --cnt;
            }
        }
    }

    uint8_t *contents() { return &_storage[0]; }

    const uint8_t *contents() const { return &_storage[0]; }

    size_t size() const { return _storage.size(); }

    bool empty() const { return _storage.empty(); }

    void resize(size_t newsize)
    {
        _storage.resize(newsize, 0);
        _rpos = 0;
        _wpos = size();
    }

    void reserve(size_t ressize)
    {
        if (ressize > size())
            _storage.reserve(ressize);
    }

    void append(const char *src, size_t cnt)
    {
        return append((const uint8_t *)src, cnt);
    }

    template <class T>
    void append(const T *src, size_t cnt)
    {
        return append((const uint8_t *)src, cnt * sizeof(T));
    }

    void append(const uint8_t *src, size_t cnt)
    {
        if (!cnt)
            return;
        //throw ByteBufferSourceException(_wpos, size(), cnt);

        if (!src)
            return;
        //throw ByteBufferSourceException(_wpos, size(), cnt);

        ASSERT(size() < 10000000, "Size too big");

        if (_storage.size() < _wpos + cnt)
            _storage.resize(_wpos + cnt);
        std::memcpy(&_storage[_wpos], src, cnt);
        _wpos += cnt;
    }

    void append(const ByteBuffer &buffer)
    {
        if (buffer.wpos())
            append(buffer.contents(), buffer.wpos());
    }

    void put(size_t pos, const uint8_t *src, size_t cnt)
    {
        if (pos + cnt > size())
            throw ByteBufferPositionException(true, pos, cnt, size());

        if (!src)
            throw ByteBufferSourceException(_wpos, size(), cnt);

        std::memcpy(&_storage[pos], src, cnt);
    }

    void print_storage() const;
    void textlike() const;
    void hexlike() const;

  protected:
    size_t _rpos, _wpos, _bitpos;
    uint8_t _curbitval;
    std::vector<uint8_t> _storage;
};

template <typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
    b << (uint32_t)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
    uint32_t vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
    b << (uint32_t)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
    {
        b << *i;
    }
    return b;
}

template <typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
    uint32_t vsize;
    b >> vsize;
    v.clear();
    while (vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V>
inline ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32_t)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V>
inline ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
    uint32_t msize;
    b >> msize;
    m.clear();
    while (msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}

/// @todo Make a ByteBuffer.cpp and move all this inlining to it.
template <>
inline std::string ByteBuffer::read<std::string>()
{
    std::string tmp;
    *this >> tmp;
    return tmp;
}

template <>
inline void ByteBuffer::read_skip<char *>()
{
    std::string temp;
    *this >> temp;
}

template <>
inline void ByteBuffer::read_skip<char const *>()
{
    read_skip<char *>();
}

template <>
inline void ByteBuffer::read_skip<std::string>()
{
    read_skip<char *>();
}