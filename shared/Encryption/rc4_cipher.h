#pragma once

#include <string.h>

class RC4Cipher
{
public:
    bool init(const char *pKey, int keyLen = 0) { return prepareKey(m_state, pKey, keyLen); }

    void code(const unsigned char *pSrc, unsigned char *pDst, int len) { codeBlock(m_state, pSrc, pDst, len); }

    void encode(const unsigned char *pSrc, unsigned char *pDst, int len) { codeBlock(m_state, pSrc, pDst, len); }

    void decode(const unsigned char *pSrc, unsigned char *pDst, int len) { codeBlock(m_state, pSrc, pDst, len); }

    struct State
    {
        int x, y;
        unsigned char s[256];
    };

    void saveStateTo(State &outState) const { outState = m_state; }

    void loadStateFrom(const State &aState) { m_state = aState; }

private:
    static bool prepareKey(State &m_state, const char *pKey, int keyLen)
    {
        if (pKey == NULL || *pKey == 0)
            return false;

        if (keyLen == 0)
            keyLen = (int)strlen(pKey);

        int i, j;
        for (i = 0; i < 256; i++)
            m_state.s[i] = i;

        unsigned char key[256];

        j = 0;
        for (i = 0; i < 256; i++)
        {
            key[i] = pKey[j++];
            if (j >= keyLen)
                j = 0;
        }

        j = 0;
        for (i = 0; i < 256; i++)
        {
            (j += m_state.s[j] + key[j]) &= 0xff;
            swapByte(m_state.s[i], m_state.s[j]);
        }

        m_state.x = m_state.y = 0;
        skipFor(m_state, 1013);

        return true;
    }

    static void skipFor(State &m_state, int len)
    {
        int x = m_state.x, y = m_state.y;

        while (len--)
        {
            (++x) &= 0xff;
            int sx = m_state.s[x];
            (y += sx) &= 0xff;
            m_state.s[x] = m_state.s[y];
            m_state.s[y] = sx;
        }

        m_state.x = x, m_state.y = y;
    }

    static void codeBlock(State &m_state, const unsigned char *pSrc, unsigned char *pDst, int len)
    {
        int x = m_state.x, y = m_state.y;

        while (len--)
        {
            (++x) &= 0xff;
            int sx = m_state.s[x];
            (y += sx) &= 0xff;
            int sy = m_state.s[y];
            m_state.s[x] = sy;
            m_state.s[y] = sx;
            *(pDst++) = *(pSrc++) ^ m_state.s[(sx + sy) & 0xff];
        }

        m_state.x = x, m_state.y = y;
    }

    static void swapByte(unsigned char &a, unsigned char &b)
    {
        int t = a;
        a = b;
        b = t;
    }

    State m_state;
};
