#pragma once

#include <cstddef>
#include <numeric>

namespace SimpleCipher
{
template <class _T1, class _T2>
struct MagicNumOp
{
    _T1 operator()(const _T1 &_Left, const _T2 &_Right) const
    {
        return (_Left + static_cast<_T1>(_Right) * 65539);
    }
};

template <typename S, typename T>
int Encrypt(size_t nOriginalLength, const S &src, T &dest)
{
    size_t sz = src.size();

    // 1 ����Ʈ ������ �����ʹ� ���� �״�� ����
    {
        if (!sz)
            return 0;
        else if (sz == 1)
        {
            dest.resize(1);
            dest[0] = src[0];

            return 0;
        }
    }

    // MagicNum�� 4����Ʈ ����̸�, �����÷ο�� ���ǹǷ� ���������� ������ �����ϴ�.
    uint magicnum = uint(std::accumulate(src.begin(), src.end(), sz, MagicNumOp<uint, typename S::value_type>()));

    // nOriginalLength�� magicnum�� ���Ͽ� 8����Ʈ�� ����Ѵ�.
    dest.resize(sz + 4 * 2);

    int nStart = magicnum % int(sz);
    int nJump = (magicnum & 0xfff0) + 1;

    // nJump �� ����
    // * ����� ������� ���� sz���� Ŀ���� Ŀ�� ���� ���� �ð��� ���� �ɸ� �� ����(�ִ� sz���� ����Ͽ� ���� �ð� ����)
    // * �ܼ��� �����ϸ� nJump�� �Ҽ��̸� sz ���� ������ �ƹ��� nJump �� �ε����� �̵����ѵ� ������ ���� �� ������
    //   sz�� Ư�� �Ҽ��� ���(��� �ƴ�)�� ��쿡�� nJump�� �Ҽ����� nJump�� ����� �� ������, �ε��� ��ø�� �߻���.
    //   ���� nJump�� �Ҽ� ���οʹ� �����ϰ� sz�� ������� 1 �ܿ��� ���� ���ڿ��� ��.
    // * �ּ��� �ڵ庸�� ���Ƽ� �׷��� ���� �� �ڵ尡 �ƴ� -,. -;;
    {
        // SimpleCipher�� nJump ��� ������� sz�� ���� �������� 2�� ������ -2 �Ͽ� �ִ� sz / 2 - 2 ���� ���� ���� ������ ��.
        nJump = int((nJump % sz) / 2 - 2);
        // ���� nJump�� 1 �����̸� ������ �꿡 ���� nJump(�� �ִ밪)�� �ٽ� ���.
        if (nJump <= 1)
        {
            // �����Ͱ� 4 ����Ʈ ������ ��쿡�� nJump�� ������ sz-1
            // sz == 4 : 1, 3�� ��ȿ nJump�� �� �� �ִµ� 1�� ���ϴ� ���� �����Ƿ� 3.
            // sz == 3 : 1, 2�� ��ȿ nJump�� �� �� �ִµ� 1�� ���ϴ� ���� �����Ƿ� 2.
            // sz == 2 : 1 �ۿ� ��ȿ nJump�� ���� -,. -;
            // ���: 4����Ʈ ���ϴ� nJump = sz - 1 �� ����(�̶�⺸�ٴ� �׳��� ����)�� ����
            if (sz <= 4)
                nJump = int(sz - 1);
            // �����Ͱ� 16 ����Ʈ ������ ��쿡�� nJump�� sz-2 �����̸鼭 sz�� ������� ���� ���� ���ϵ��� ��
            // * ���⼭�� nJump�� �ִ밪�� �����ϰ� �� ������ ���� �� sz�� ������� ���� ���� �Ʒ��ʿ��� ����
            else if (sz <= 16)
                nJump = int(sz - 2);
            // �����Ͱ� 17 ����Ʈ �̻��� ��쿡��(���� �� ���ǿ� �ش� �� �Ǵ� ��� ���) nJump�� sz / 2 - 1 ���� ���� sz�� ������� ���� ���� ����
            else
                nJump = int(sz / 2 - 1);
        }

        // ���� ���� nJump ������ ���� �� sz�� ������� ���� ���� ���ϱ�...
        // sz�� ��� ��� �̸� ���صα�
        // sz�� ���� �� �ִ� ��� ���
        std::vector<int> vDivisorList;
        {
            int nMaxValue = int(sz);
            // vDivisorList�� n, (Num/n)�� 2���� ���ڰ� ���ÿ� ���� ������ ��ų �� �����Ƿ� (Num/n)�� ��(quotient) ����Ʈ�� ���� �־��ٰ� ���߿� vDivisorList�� ��ħ
            std::vector<int> vQuotientList;
            for (int nDivisor = 2; nDivisor < nMaxValue; ++nDivisor)
            {
                // ����� �ƴϸ� �н�
                if (sz % nDivisor)
                    continue;

                // Num�� n���� ������ �������� ��쿡 ����� n, (Num/n) 2���̸�, n�� (Num/n)���� ũ�ų� �������� �ش� n���� ū n�� �̹� (Num/n)���� �����ߴ� ����� ��.
                vDivisorList.push_back(nDivisor);
                nMaxValue = int(sz / nDivisor);
                // sz�� Ư�� ���� �������� ��� nDivisor == nMaxValue�� ��
                if (nDivisor != nMaxValue)
                    vQuotientList.push_back(nMaxValue);
            }

            // �� ����Ʈ�� ��� ����Ʈ�� ��ġ��
            // * ���� ū ������ ��������Ƿ� �������� vDivisorList�� �߰���
            vDivisorList.reserve(vDivisorList.size() + vQuotientList.size());
            for (std::vector<int>::const_reverse_iterator it = vQuotientList.rbegin(); it != vQuotientList.rend(); ++it)
            {
                vDivisorList.push_back((*it));
            }
        }

        // nJump ���� ���ų� ���� ���� �� ���� ū sz���� ����� ���� �� ���ϱ�
        for (; nJump > 1; --nJump)
        {
            std::vector<int>::const_iterator it;
            for (it = vDivisorList.begin(); it != vDivisorList.end(); ++it)
            {
                // sz�� ����� nJump�� ����� ��� ���� ����� ���ؼ��� �˻� ����
                if (!(nJump % (*it)))
                    break;
            }

            // ������� ������ ��� ���� nJump ���� ����ϵ��� ���� ����
            if (it == vDivisorList.end())
                break;
        }
    }

    uint8_t ucValue = *src.begin();

    dest[nStart] = static_cast<typename T::value_type>(ucValue ^ magicnum);
    nStart += nJump;
    nStart %= sz;

    for (size_t i = 1; i < sz; ++i)
    {
        assert(!dest[nStart]);
        dest[nStart] = static_cast<typename T::value_type>((src[i] - ucValue) ^ magicnum);
        ucValue = src[i];

        nStart += nJump;
        nStart %= sz;
    }
    dest[sz + 0] = typename T::value_type(((nOriginalLength >> 24) & 0xff) ^ magicnum);
    dest[sz + 1] = typename T::value_type(((nOriginalLength >> 16) & 0xff) ^ magicnum);
    dest[sz + 2] = typename T::value_type(((nOriginalLength >> 8) & 0xff) ^ magicnum);
    dest[sz + 3] = typename T::value_type(((nOriginalLength)&0xff) ^ magicnum);

    dest[sz + 4] = typename T::value_type((magicnum >> 24) & 0xff);
    dest[sz + 5] = typename T::value_type((magicnum >> 16) & 0xff);
    dest[sz + 6] = typename T::value_type((magicnum >> 8) & 0xff);
    dest[sz + 7] = typename T::value_type((magicnum)&0xff);

    return 0;
};

template <class S, class T>
int Decrypt(const S &src, T &dest, size_t *pnOriginalLength)
{
    S temp;

    // 1 ����Ʈ ������ �����ʹ� ���� �״�� ����
    size_t nSrcSize = src.size();
    if (nSrcSize == 1)
    {
        if (!nSrcSize)
        {
            *pnOriginalLength = 0;

            return 0;
        }
        else if (nSrcSize == 1)
        {
            dest.resize(1);
            dest[0] = src[0];
            *pnOriginalLength = 1;

            return 0;
        }
    }
    // ���� �����Ͱ� 2����Ʈ �̻��̸� 8 ����Ʈ�� �߰� ����(nOriginalLength, magicnum)�� �����Ƿ� 10 ����Ʈ �̻����� ���̰� �þ.
    else if (nSrcSize < 10)
    {
        assert(0);
        return 0;
    }

    // nOriginalLength�� magicnum�� ���Ͽ� 8����Ʈ�� ����Ѵ�.
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

    // nJump �� ����
    // * ����� ������� ���� sz���� Ŀ���� Ŀ�� ���� ���� �ð��� ���� �ɸ� �� ����(�ִ� sz���� ����Ͽ� ���� �ð� ����)
    // * �ܼ��� �����ϸ� nJump�� �Ҽ��̸� sz ���� ������ �ƹ��� nJump �� �ε����� �̵����ѵ� ������ ���� �� ������
    //   sz�� Ư�� �Ҽ��� ���(��� �ƴ�)�� ��쿡�� nJump�� �Ҽ����� nJump�� ����� �� ������, �ε��� ��ø�� �߻���.
    //   ���� nJump�� �Ҽ� ���οʹ� �����ϰ� sz�� ������� 1 �ܿ��� ���� ���ڿ��� ��.
    // * �ּ��� �ڵ庸�� ���Ƽ� �׷��� ���� �� �ڵ尡 �ƴ� -,. -;;
    {
        // SimpleCipher�� nJump ��� ������� sz�� ���� �������� 2�� ������ -2 �Ͽ� �ִ� sz / 2 - 2 ���� ���� ���� ������ ��.
        nJump = int((nJump % sz) / 2 - 2);
        // ���� nJump�� 1 �����̸� ������ �꿡 ���� nJump(�� �ִ밪)�� �ٽ� ���.
        if (nJump <= 1)
        {
            // �����Ͱ� 4 ����Ʈ ������ ��쿡�� nJump�� ������ sz-1
            // sz == 4 : 1, 3�� ��ȿ nJump�� �� �� �ִµ� 1�� ���ϴ� ���� �����Ƿ� 3.
            // sz == 3 : 1, 2�� ��ȿ nJump�� �� �� �ִµ� 1�� ���ϴ� ���� �����Ƿ� 2.
            // sz == 2 : 1 �ۿ� ��ȿ nJump�� ���� -,. -;
            // ���: 4����Ʈ ���ϴ� nJump = sz - 1 �� ����(�̶�⺸�ٴ� �׳��� ����)�� ����
            if (sz <= 4)
                nJump = int(sz - 1);
            // �����Ͱ� 16 ����Ʈ ������ ��쿡�� nJump�� sz-2 �����̸鼭 sz�� ������� ���� ���� ���ϵ��� ��
            // * ���⼭�� nJump�� �ִ밪�� �����ϰ� �� ������ ���� �� sz�� ������� ���� ���� �Ʒ��ʿ��� ����
            else if (sz <= 16)
                nJump = int(sz - 2);
            // �����Ͱ� 17 ����Ʈ �̻��� ��쿡��(���� �� ���ǿ� �ش� �� �Ǵ� ��� ���) nJump�� sz / 2 - 1 ���� ���� sz�� ������� ���� ���� ����
            else
                nJump = int(sz / 2 - 1);
        }

        // ���� ���� nJump ������ ���� �� sz�� ������� ���� ���� ���ϱ�...
        // sz�� ��� ��� �̸� ���صα�
        // sz�� ���� �� �ִ� ��� ���
        std::vector<int> vDivisorList;
        {
            int nMaxValue = int(sz);
            // vDivisorList�� n, (Num/n)�� 2���� ���ڰ� ���ÿ� ���� ������ ��ų �� �����Ƿ� (Num/n)�� ��(quotient) ����Ʈ�� ���� �־��ٰ� ���߿� vDivisorList�� ��ħ
            std::vector<int> vQuotientList;
            for (int nDivisor = 2; nDivisor < nMaxValue; ++nDivisor)
            {
                // ����� �ƴϸ� �н�
                if (sz % nDivisor)
                    continue;

                // Num�� n���� ������ �������� ��쿡 ����� n, (Num/n) 2���̸�, n�� (Num/n)���� ũ�ų� �������� �ش� n���� ū n�� �̹� (Num/n)���� �����ߴ� ����� ��.
                vDivisorList.push_back(nDivisor);
                nMaxValue = int(sz / nDivisor);
                // sz�� Ư�� ���� �������� ��� nDivisor == nMaxValue�� ��
                if (nDivisor != nMaxValue)
                    vQuotientList.push_back(nMaxValue);
            }

            // �� ����Ʈ�� ��� ����Ʈ�� ��ġ��
            // * ���� ū ������ ��������Ƿ� �������� vDivisorList�� �߰���
            vDivisorList.reserve(vDivisorList.size() + vQuotientList.size());
            for (std::vector<int>::const_reverse_iterator it = vQuotientList.rbegin(); it != vQuotientList.rend(); ++it)
            {
                vDivisorList.push_back((*it));
            }
        }

        // nJump ���� ���ų� ���� ���� �� ���� ū sz���� ����� ���� �� ���ϱ�
        for (; nJump > 1; --nJump)
        {
            std::vector<int>::const_iterator it;
            for (it = vDivisorList.begin(); it != vDivisorList.end(); ++it)
            {
                // sz�� ����� nJump�� ����� ��� ���� ����� ���ؼ��� �˻� ����
                if (!(nJump % (*it)))
                    break;
            }

            // ������� ������ ��� ���� nJump ���� ����ϵ��� ���� ����
            if (it == vDivisorList.end())
                break;
        }
    }

    uint8_t ucValue = 0;

    dest.resize(src.size());

    for (size_t i = 0; i < sz; ++i)
    {
        ucValue += temp[nStart];
        dest[i] = ucValue;
        nStart += nJump;
        nStart %= sz;
    }

    return 0;
}
}; // namespace SimpleCipher
