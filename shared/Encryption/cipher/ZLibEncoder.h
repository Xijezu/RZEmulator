
#include <memory.h>

#include <zlib.h>
void *s_memcpy(void *dest, size_t dest_size, const void *src, size_t copy_size)
{
    //assert( src != NULL );

    if (src != NULL && copy_size > 0)
    {
        if (dest != NULL && dest_size >= copy_size)
        {
#ifdef SUPPORT_SAFE_FUNCTION
#ifdef _DEBUG
            errno_t error =
#endif

                ::memcpy_s(dest, dest_size, src, copy_size);

#ifdef _DEBUG
            assert(error == 0);
#endif
#else
            ::memcpy(dest, src, copy_size);
#endif
        }
    }

    return dest;
}

struct XZlibEncoder
{

    static int Encode(const void *pSource, const size_t nSourceLen, void *pTarget, const size_t nTargetLen, size_t *pEncodedLen)
    {
        if (!nSourceLen)
            return Z_DATA_ERROR;

        void *pFinalTarget = 0;
        if (!(pTarget >= (static_cast<const char *>(pSource) + nSourceLen) || pSource >= (static_cast<char *>(pTarget) + nTargetLen)))
        {
            pFinalTarget = pTarget;
            pTarget = new char[nTargetLen];
        }

        unsigned long nEncodedLength = static_cast<const unsigned long>(nTargetLen);

        int nErrorCode = compress(reinterpret_cast<unsigned char *>(pTarget), &nEncodedLength,
                                  reinterpret_cast<const unsigned char *>(pSource), static_cast<unsigned long>(nSourceLen));

        if (nErrorCode == Z_OK)
        {
            if (pEncodedLen)
            {
                *pEncodedLen = static_cast<size_t>(nEncodedLength);
            }

            if (pFinalTarget)
            {
                s_memcpy(pFinalTarget, nTargetLen, pTarget, nEncodedLength);
                delete[] pTarget;
            }
        }

        return nErrorCode;
    }

    static int Decode(const void *pSource, const size_t nSourceLen, void *pTarget, const size_t nTargetLen, size_t *pDecodedLen)
    {
        if (!nSourceLen)
            return Z_DATA_ERROR;

        void *pFinalTarget = 0;
        if (!(pTarget >= (static_cast<const char *>(pSource) + nSourceLen) || pSource >= (static_cast<char *>(pTarget) + nTargetLen)))
        {
            pFinalTarget = pTarget;
            pTarget = new char[nTargetLen];
        }

        unsigned long nDecodedLength = static_cast<const unsigned long>(nTargetLen);

        int nErrorCode = uncompress(reinterpret_cast<unsigned char *>(pTarget), &nDecodedLength,
                                    reinterpret_cast<const unsigned char *>(pSource), static_cast<unsigned long>(nSourceLen));

        if (nErrorCode == Z_OK)
        {
            if (pDecodedLen)
            {
                *pDecodedLen = static_cast<size_t>(nDecodedLength);
            }

            if (pFinalTarget)
            {
                s_memcpy(pFinalTarget, nTargetLen, pTarget, nDecodedLength);
                delete[] pTarget;
            }
        }

        return nErrorCode;
    }
};