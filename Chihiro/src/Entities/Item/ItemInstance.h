#pragma once
/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "ItemTemplate.hpp"
#include <array>

class ItemInstance {
public:
    void Copy(const ItemInstance &pFrom)
    {
        SetOwnerHandle(pFrom.GetOwnerHandle());
        SetOwnSummonHandle(pFrom.GetOwnSummonHandle());
        SetUID(pFrom.GetUID());
        SetCode(pFrom.GetCode());
        SetIdx(pFrom.GetIndex());
        SetLevel(pFrom.GetLevel());
        SetEnhance(pFrom.GetEnhance());
        SetOwnerUID(pFrom.GetOwnerUID());
        SetOwnSummonUID(pFrom.GetOwnSummonUID());
        SetAuctionID(pFrom.GetAuctionID());
        SetItemKeepingID(pFrom.GetItemKeepingID());
        SetCount(pFrom.GetCount());
        SetCurrentEndurance(pFrom.GetCurrentEndurance());
        SetExpire(pFrom.GetExpire());
        SetFlag(pFrom.GetFlag());
        SetGenerateInfo(pFrom.GetGenerateCode());
        SetWearInfo(pFrom.GetItemWearType());
        SetSocket(pFrom.GetSocket());
    }

    /// Getters
    inline uint32_t GetOwnerHandle() const { return m_hOwnerHandle; }
    inline uint32_t GetOwnSummonHandle() const { return m_hOwnSummonHandle; }
    inline int64_t GetUID() const { return m_nUID; }
    inline int32_t GetCode() const { return m_nCode; }
    inline int32_t GetIndex() const { return m_nIdx; }
    inline int32_t GetLevel() const { return m_nLevel; }
    inline int32_t GetEnhance() const { return m_nEnhance; }
    inline int32_t GetEndurance() const { return m_nEndurance; }
    inline int32_t GetCurrentEndurance() const { return m_nCurrentEndurance; }
    inline int32_t GetOwnerUID() const { return m_nOwnerUID; }
    inline int32_t GetOwnSummonUID() const { return m_nOwnSummonUID; }
    inline int32_t GetAuctionID() const { return m_nAuctionID; }
    inline int32_t GetItemKeepingID() const { return m_nItemKeepingID; }
    inline int64_t GetCount() const { return m_nCount; }
    inline int64_t GetExpire() const { return m_nExpire; }
    inline int32_t GetFlag() const { return m_nFlag; }
    inline GenerateCode GetGenerateCode() const { return m_eGenerateInfo; }
    inline ItemWearType GetItemWearType() const { return m_nWearInfo; }
    inline std::array<int32_t, 4> GetSocket() const { return m_pSocket; }
    inline int32_t GetSocketIndex(int32_t nIdx) const { return m_pSocket[nIdx]; }

    /// Setters
    inline void SetOwnerHandle(uint32_t hOwnerHandle) { m_hOwnerHandle = hOwnerHandle; }
    inline void SetOwnSummonHandle(uint32_t hOwnSummonHandle) { m_hOwnSummonHandle = hOwnSummonHandle; }
    inline void SetUID(int64_t nUID) { m_nUID = nUID; }
    inline void SetCode(int32_t nCode) { m_nCode = nCode; }
    inline void SetIdx(int32_t nIdx) { m_nIdx = nIdx; }
    inline void SetLevel(int32_t nLevel) { m_nLevel = nLevel; }
    inline void SetEnhance(int32_t nEnhance) { m_nEnhance = nEnhance; }
    inline void SetEndurance(int32_t nEndurance) { m_nEndurance = nEndurance; }
    inline void SetCurrentEndurance(int32_t nCurrentEndurance) { m_nCurrentEndurance = nCurrentEndurance; }
    inline void SetOwnerUID(int32_t nOwnerUID) { m_nOwnerUID = nOwnerUID; }
    inline void SetOwnSummonUID(int32_t nOwnSummonUID) { m_nOwnSummonUID = nOwnSummonUID; }
    inline void SetAuctionID(int32_t nAuctionID) { m_nAuctionID = nAuctionID; }
    inline void SetItemKeepingID(int32_t nItemKeepingID) { m_nItemKeepingID = nItemKeepingID; }
    inline void SetCount(int64_t nCount) { m_nCount = nCount; }
    inline void SetExpire(int64_t nExpire) { m_nExpire = nExpire; }
    inline void SetFlag(int32_t nFlag) { m_nFlag = nFlag; }
    inline void SetGenerateInfo(GenerateCode eGenerateCode) { m_eGenerateInfo = eGenerateCode; };
    inline void SetWearInfo(ItemWearType eItemWearType) { m_nWearInfo = eItemWearType; }
    inline void SetSocket(std::array<int32_t, 4> pSocket) { std::copy(std::begin(pSocket), std::end(pSocket), std::begin(m_pSocket)); }
    inline void SetSocketIndex(int32_t nIdx, int32_t nValue) { m_pSocket[nIdx] = nValue; }

private:
    uint32_t m_hOwnerHandle{0};
    uint32_t m_hOwnSummonHandle{0};
    int64_t m_nUID{0};
    int32_t m_nCode{0};
    int32_t m_nIdx{0};
    int32_t m_nLevel{0};
    int32_t m_nEnhance{0};
    int32_t m_nEndurance{0};
    int32_t m_nCurrentEndurance{0};
    int32_t m_nOwnerUID{0};
    int32_t m_nOwnSummonUID{0};
    int32_t m_nAuctionID{0};
    int32_t m_nItemKeepingID{0};
    int64_t m_nCount{0};
    int64_t m_nExpire{0};
    int32_t m_nFlag{0};
    GenerateCode m_eGenerateInfo = GenerateCode::BY_UNKNOWN;
    ItemWearType m_nWearInfo{ItemWearType::WEAR_CANTWEAR};
    std::array<int32_t, 4> m_pSocket{0};
};