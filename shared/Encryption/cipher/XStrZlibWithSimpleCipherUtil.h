#pragma once

#include <string>
#include <string.h>
#include "SimpleCipher.h"
#include "Common.h"
#include "ZLibEncoder.h"

bool MXEncrypt(const char *pszFmtTag, const unsigned int nVersion, const char *pSource, size_t nSourceLength, std::vector<uint8_t> &vResult)
{
	size_t nFmtTagLength = strlen(pszFmtTag);
	if (nFmtTagLength > 8)
	{
		assert(0);
		return false;
	}

	// The required buffer for compression size is calculated as described on http://www.zlib.net/manual.html#compress.
	std::vector<uint8_t> vBuffer((nSourceLength) ? static_cast<size_t>(1.001 * (nSourceLength + 12) + 1) : 0);
	uint8_t *pBuffer = &vBuffer.front();

	if (nSourceLength)
	{
		size_t nWrittenLength = 0;

		if (XZlibEncoder::Encode(pSource, nSourceLength, pBuffer, vBuffer.size(), &nWrittenLength))
		{
			assert(0);
			return false;
		}

		if (nWrittenLength > vBuffer.size())
		{
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

struct XStrZlibWithSimpleCipherUtil
{
	static std::string Encrypt(const char *str)
	{
		if (!strlen(str))
			return "";

		std::vector<uint8_t> vEncrypted;
		MXEncrypt("EV", 0x00030000, str, strlen(str), vEncrypted);

		std::string strEncrypt = "";
		for (std::vector<uint8_t>::const_iterator it = vEncrypted.begin(); it != vEncrypted.end(); ++it)
		{
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
		struct MemoryDeallocator
		{
			MemoryDeallocator(uint8_t *_pBuffer) : pBuffer(_pBuffer) {}
			~MemoryDeallocator() { delete[] pBuffer; }

			uint8_t *pBuffer;
		} md(pBuffer);

		for (int i = 0; i < nContentsLength; ++i)
		{
			sscanf(&str[2 * i], "%02x", &pBuffer[i]);
		}

		std::vector<uint8_t> vDecrypted;
		//XEncrypt::Decrypt(XEncrypt::ET_ZLIB_WITH_SIMPLE_CIPHER, "EV", 0x00030000, pBuffer, nContentsLength, vDecrypted);

		std::string strDecrypt = "";
		for (std::vector<uint8_t>::const_iterator it = vDecrypted.begin(); it != vDecrypted.end(); ++it)
		{
			strDecrypt += (*it);
		}

		return strDecrypt;
	}
};
