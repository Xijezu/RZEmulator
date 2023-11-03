#pragma once

#include <memory.h>
#include <string.h>
#include <string>
#include <zlib.h>

#include "Common.h"

#pragma once

#include <cstddef>
#include <numeric>

namespace SimpleCipher {
    template<class _T1, class _T2>
    struct MagicNumOp {
        _T1 operator()(const _T1 &_Left, const _T2 &_Right) const { return (_Left + static_cast<_T1>(_Right) * 65539); }
    };

    template<typename S, typename T>
    int Encrypt(size_t nOriginalLength, const S &src, T &dest)
    {
        size_t sz = src.size();

        {
            if (!sz)
                return 0;
            else if (sz == 1) {
                dest.resize(1);
                dest[0] = src[0];

                return 0;
            }
        }

        uint32_t magicnum = uint32_t(std::accumulate(src.begin(), src.end(), sz, MagicNumOp<uint32_t, typename S::value_type>()));

        dest.resize(sz + 4 * 2);

        int nStart = magicnum % int(sz);
        int nJump = (magicnum & 0xfff0) + 1;

        {
            nJump = int((nJump % sz) / 2 - 2);
            if (nJump <= 1) {
                if (sz <= 4)
                    nJump = int(sz - 1);
                else if (sz <= 16)
                    nJump = int(sz - 2);
                else
                    nJump = int(sz / 2 - 1);
            }

            std::vector<int> vDivisorList;
            {
                int nMaxValue = int(sz);
                std::vector<int> vQuotientList;
                for (int nDivisor = 2; nDivisor < nMaxValue; ++nDivisor) {
                    if (sz % nDivisor)
                        continue;

                    vDivisorList.push_back(nDivisor);
                    nMaxValue = int(sz / nDivisor);
                    if (nDivisor != nMaxValue)
                        vQuotientList.push_back(nMaxValue);
                }
                vDivisorList.reserve(vDivisorList.size() + vQuotientList.size());
                for (std::vector<int>::const_reverse_iterator it = vQuotientList.rbegin(); it != vQuotientList.rend(); ++it) {
                    vDivisorList.push_back((*it));
                }
            }

            for (; nJump > 1; --nJump) {
                std::vector<int>::const_iterator it;
                for (it = vDivisorList.begin(); it != vDivisorList.end(); ++it) {
                    if (!(nJump % (*it)))
                        break;
                }

                if (it == vDivisorList.end())
                    break;
            }
        }

        uint8_t ucValue = *src.begin();

        dest[nStart] = static_cast<typename T::value_type>(ucValue ^ magicnum);
        nStart += nJump;
        nStart %= sz;

        for (size_t i = 1; i < sz; ++i) {
            assert(!dest[nStart]);
            dest[nStart] = static_cast<typename T::value_type>((src[i] - ucValue) ^ magicnum);
            ucValue = src[i];

            nStart += nJump;
            nStart %= sz;
        }
        dest[sz + 0] = typename T::value_type(((nOriginalLength >> 24) & 0xff) ^ magicnum);
        dest[sz + 1] = typename T::value_type(((nOriginalLength >> 16) & 0xff) ^ magicnum);
        dest[sz + 2] = typename T::value_type(((nOriginalLength >> 8) & 0xff) ^ magicnum);
        dest[sz + 3] = typename T::value_type(((nOriginalLength) & 0xff) ^ magicnum);

        dest[sz + 4] = typename T::value_type((magicnum >> 24) & 0xff);
        dest[sz + 5] = typename T::value_type((magicnum >> 16) & 0xff);
        dest[sz + 6] = typename T::value_type((magicnum >> 8) & 0xff);
        dest[sz + 7] = typename T::value_type((magicnum) & 0xff);

        return 0;
    };

    template<class S, class T>
    int Decrypt(const S &src, T &dest, size_t *pnOriginalLength)
    {
        S temp;

        size_t nSrcSize = src.size();
        if (nSrcSize == 1) {
            if (!nSrcSize) {
                *pnOriginalLength = 0;

                return 0;
            }
            else if (nSrcSize == 1) {
                dest.resize(1);
                dest[0] = src[0];
                *pnOriginalLength = 1;

                return 0;
            }
        }
        else if (nSrcSize < 10) {
            assert(0);
            return 0;
        }

        size_t sz = nSrcSize - 4 * 2;

        unsigned int magicnum;

        temp.resize(src.size() + 4);

        magicnum = static_cast<uint8_t>(src[sz + 4]);
        magicnum = magicnum * 256 + static_cast<uint8_t>(src[sz + 5]);
        magicnum = magicnum * 256 + static_cast<uint8_t>(src[sz + 6]);
        magicnum = magicnum * 256 + static_cast<uint8_t>(src[sz + 7]);

        for (size_t i = 0; i < sz + 4; ++i)
            temp[i] = static_cast<typename S::value_type>(src[i] ^ magicnum);

        *pnOriginalLength = static_cast<uint8_t>(temp[sz + 0]);
        *pnOriginalLength = *pnOriginalLength * 256 + static_cast<uint8_t>(temp[sz + 1]);
        *pnOriginalLength = *pnOriginalLength * 256 + static_cast<uint8_t>(temp[sz + 2]);
        *pnOriginalLength = *pnOriginalLength * 256 + static_cast<uint8_t>(temp[sz + 3]);

        int nStart = magicnum % int(sz);
        int nJump = (magicnum & 0xfff0) + 1;

        {
            nJump = int((nJump % sz) / 2 - 2);
            if (nJump <= 1) {
                if (sz <= 4)
                    nJump = int(sz - 1);
                else if (sz <= 16)
                    nJump = int(sz - 2);
                else
                    nJump = int(sz / 2 - 1);
            }

            std::vector<int> vDivisorList;
            {
                int nMaxValue = int(sz);
                std::vector<int> vQuotientList;
                for (int nDivisor = 2; nDivisor < nMaxValue; ++nDivisor) {
                    if (sz % nDivisor)
                        continue;

                    vDivisorList.push_back(nDivisor);
                    nMaxValue = int(sz / nDivisor);
                    if (nDivisor != nMaxValue)
                        vQuotientList.push_back(nMaxValue);
                }

                vDivisorList.reserve(vDivisorList.size() + vQuotientList.size());
                for (std::vector<int>::const_reverse_iterator it = vQuotientList.rbegin(); it != vQuotientList.rend(); ++it) {
                    vDivisorList.push_back((*it));
                }
            }

            for (; nJump > 1; --nJump) {
                std::vector<int>::const_iterator it;
                for (it = vDivisorList.begin(); it != vDivisorList.end(); ++it) {
                    if (!(nJump % (*it)))
                        break;
                }

                if (it == vDivisorList.end())
                    break;
            }
        }

        uint8_t ucValue = 0;

        dest.resize(src.size());

        for (size_t i = 0; i < sz; ++i) {
            ucValue += temp[nStart];
            dest[i] = ucValue;
            nStart += nJump;
            nStart %= sz;
        }

        return 0;
    }
}; // namespace SimpleCipher


void *s_memcpy(void *dest, size_t dest_size, const void *src, size_t copy_size)
{
    // assert( src != NULL );

    if (src != NULL && copy_size > 0) {
        if (dest != NULL && dest_size >= copy_size) {
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

struct XZlibEncoder {

    static int Encode(const void *pSource, const size_t nSourceLen, void *pTarget, const size_t nTargetLen, size_t *pEncodedLen)
    {
        if (!nSourceLen)
            return Z_DATA_ERROR;

        void *pFinalTarget = 0;
        if (!(pTarget >= (static_cast<const char *>(pSource) + nSourceLen) || pSource >= (static_cast<char *>(pTarget) + nTargetLen))) {
            pFinalTarget = pTarget;
            pTarget = new char[nTargetLen];
        }

        unsigned long nEncodedLength = static_cast<unsigned long>(nTargetLen);

        int nErrorCode = compress(reinterpret_cast<unsigned char *>(pTarget), &nEncodedLength, reinterpret_cast<const unsigned char *>(pSource), static_cast<unsigned long>(nSourceLen));

        if (nErrorCode == Z_OK) {
            if (pEncodedLen) {
                *pEncodedLen = static_cast<size_t>(nEncodedLength);
            }

            if (pFinalTarget) {
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
        if (!(pTarget >= (static_cast<const char *>(pSource) + nSourceLen) || pSource >= (static_cast<char *>(pTarget) + nTargetLen))) {
            pFinalTarget = pTarget;
            pTarget = new char[nTargetLen];
        }

        unsigned long nDecodedLength = static_cast<unsigned long>(nTargetLen);

        int nErrorCode = uncompress(reinterpret_cast<unsigned char *>(pTarget), &nDecodedLength, reinterpret_cast<const unsigned char *>(pSource), static_cast<unsigned long>(nSourceLen));

        if (nErrorCode == Z_OK) {
            if (pDecodedLen) {
                *pDecodedLen = static_cast<size_t>(nDecodedLength);
            }

            if (pFinalTarget) {
                s_memcpy(pFinalTarget, nTargetLen, pTarget, nDecodedLength);
                delete[] pTarget;
            }
        }

        return nErrorCode;
    }
};

bool MXEncrypt(const char *pszFmtTag, const unsigned int nVersion, const char *pSource, size_t nSourceLength, std::vector<uint8_t> &vResult)
{
    size_t nFmtTagLength = strlen(pszFmtTag);
    if (nFmtTagLength > 8) {
        assert(0);
        return false;
    }

    // The required buffer for compression size is calculated as described on http://www.zlib.net/manual.html#compress.
    std::vector<uint8_t> vBuffer((nSourceLength) ? static_cast<size_t>(1.001 * (nSourceLength + 12) + 1) : 0);
    uint8_t *pBuffer = &vBuffer.front();

    if (nSourceLength) {
        size_t nWrittenLength = 0;

        if (XZlibEncoder::Encode(pSource, nSourceLength, pBuffer, vBuffer.size(), &nWrittenLength)) {
            assert(0);
            return false;
        }

        if (nWrittenLength > vBuffer.size()) {
            assert(0);
            return false;
        }

        vBuffer.resize(nWrittenLength);
        // Preventing pBuffer to be invalid by reallocation by resize()
        pBuffer = &vBuffer.front();
    }

    std::vector<uint8_t> vOutput;
    SimpleCipher::Encrypt(nSourceLength, vBuffer, vOutput);

    vResult.clear();
    vResult.resize(nFmtTagLength + sizeof(unsigned int));
    s_memcpy(&vResult.front(), vResult.size(), pszFmtTag, nFmtTagLength);
    s_memcpy(&vResult.front() + nFmtTagLength, vResult.size() - nFmtTagLength, &nVersion, sizeof(unsigned int));
    vResult.insert(vResult.end(), vOutput.begin(), vOutput.end());
    return true;
}

struct XStrZlibWithSimpleCipherUtil {
    static std::string Encrypt(const char *str)
    {
        if (!strlen(str))
            return "";

        std::vector<uint8_t> vEncrypted;
        MXEncrypt("EV", 0x00030000, str, strlen(str), vEncrypted);

        std::string strEncrypt = "";
        for (std::vector<uint8_t>::const_iterator it = vEncrypted.begin(); it != vEncrypted.end(); ++it) {
            char szBuf[3] = {
                0,
            };
            sprintf(szBuf, "%02x", (*it));
            strEncrypt += szBuf;
        }

        return strEncrypt;
    }

    static std::string Decrypt(const char *str)
    {
        if (!strlen(str) || strlen(str) % 2 != 0)
            return "";

        int nContentsLength = static_cast<int>(strlen(str) / 2);
        uint8_t *pBuffer = new uint8_t[nContentsLength + 3];
        struct MemoryDeallocator {
            MemoryDeallocator(uint8_t *_pBuffer)
                : pBuffer(_pBuffer)
            {
            }
            ~MemoryDeallocator() { delete[] pBuffer; }

            uint8_t *pBuffer;
        } md(pBuffer);

        for (int i = 0; i < nContentsLength; ++i) {
            sscanf(&str[2 * i], "%02x", &pBuffer[i]);
        }

        std::vector<uint8_t> vDecrypted;
        // XEncrypt::Decrypt(XEncrypt::ET_ZLIB_WITH_SIMPLE_CIPHER, "EV", 0x00030000, pBuffer, nContentsLength, vDecrypted);

        std::string strDecrypt = "";
        for (std::vector<uint8_t>::const_iterator it = vDecrypted.begin(); it != vDecrypted.end(); ++it) {
            strDecrypt += (*it);
        }

        return strDecrypt;
    }
};
