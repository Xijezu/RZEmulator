#pragma once

struct XRC4Cipher
{
public:
    XRC4Cipher();
    virtual ~XRC4Cipher();

    void SetKey(const char *pKey);

    // pSource �� pTarget �� ���Ƶ� ������.
    virtual void Encode(const void *pSource, void *pTarget, unsigned len, bool bIsPeek = false);
    virtual void Decode(const void *pSource, void *pTarget, unsigned len, bool bIsPeek = false);
    virtual void Clear();

private:
    inline void tryCipher(const void *pSource, void *pTarget, unsigned len);
    inline void doCipher(const void *pSource, void *pTarget, unsigned len);

    struct TImpl;
    TImpl *m_pImpl;
};
