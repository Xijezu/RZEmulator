#include "ArRegion.h"

void ArRegionContainer::InitRegionSystem(uint32 width, uint32 height)
{
    m_nMapWidth          = width;
    m_nMapHeight         = height;
    m_nRegionWidth       = (uint32) ((width / AR_REGION_SIZE) + 1.0f);
    m_nRegionHeight      = (uint32) ((height / AR_REGION_SIZE) + 1.0f);
    m_nRegionBlockWidth  = (m_nRegionWidth / REGION_BLOCK_COUNT) + 1; // 5
    m_nRegionBlockHeight = (m_nRegionHeight / REGION_BLOCK_COUNT) + 1;
    //sLog->outBasic("%d", m_nRegionBlockHeight * m_nRegionBlockWidth);

    //m_RegionBlock = new ArRegionBlock[m_nRegionBlockHeight * m_nRegionBlockWidth];
// 	for (int i = 0; i < (m_nRegionBlockHeight*m_nRegionBlockWidth); i++) {
// 		m_RegionBlock[i] = new ArRegionBlock();
// 	}
}

bool ArRegionContainer::IsValidRegion(uint32 rx, uint32 ry, uint32 layer)
{
    return rx < m_nRegionWidth && ry < m_nRegionHeight && layer < 256;
}

uint32 ArRegionContainer::IsVisibleRegion(uint32 rx, uint32 ry, uint32 _rx, uint32 _ry)
{
    uint32 result = 0;

    uint32 cx = _rx - rx + 3;
    uint32 cy = _ry - ry + 3;
    if (cx <= 6 && cy <= 6)
        result = s_Matrix[cx + (7 * cy)];
    return result;
}

ArRegion *ArRegionContainer::GetRegion(WorldObject &pObject)
{
    return GetRegion((uint32) pObject.GetPositionX() / AR_REGION_SIZE, (uint32) pObject.GetPositionY() / AR_REGION_SIZE, pObject.GetLayer());
}

ArRegion *ArRegionContainer::GetRegion(uint32 rx, uint32 ry, uint32 layer)
{
    if (!IsValidRegion(rx, ry, layer))
        return nullptr;
    ArRegionBlock *rBlock = &m_RegionBlock[(rx / REGION_BLOCK_COUNT) + (ry / REGION_BLOCK_COUNT) * m_nRegionBlockWidth];

    return rBlock->getRegion((rx % REGION_BLOCK_COUNT), (ry % REGION_BLOCK_COUNT), layer);
}

void ArRegionContainer::DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, std::function<void (ArRegion*)> fn)
{
    uint right;
    uint top;
    uint bottom;
    uint left;

    left     = rx - 3;
    if (rx < 3)
        left = 0;

    top     = ry - 3;
    if (ry < 3)
        top = 0;

    right     = rx + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom     = ry + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;

    for (uint x = left; x <= right; ++x) {
        for (uint y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx, ry, x, y) != 0) {
                ArRegion *region = GetRegion(x, y, layer);
                if (region != nullptr)
                    fn(region);
            }
        }
    }
}

void ArRegionContainer::DoEachNewRegion(uint rx, uint ry, uint prx, uint pry, uint8_t layer, std::function<void (ArRegion*)> fn)
{
    uint right;
    uint top;
    uint bottom;
    uint left;

    left     = rx - 3;
    if (rx < 3)
        left = 0;

    top     = ry - 3;
    if (ry < 3)
        top = 0;

    right     = rx + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom     = ry + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;

    for (uint x = left; x <= right; ++x) {
        for (uint y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx, ry, x, y) != 0) {
                if (IsVisibleRegion(prx, pry, x, y) == 0) {
                    auto region = GetRegion(x, y, layer);
                    if (region != nullptr)
                        fn(region);
                }
            }
        }
    }
}

void ArRegionContainer::DoEachVisibleRegion(uint rx1, uint ry1, uint rx2, uint ry2, uint8_t layer, std::function<void(ArRegion *)> fn)
{
    uint right;
    uint top;
    uint bottom;
    uint left;

    left     = rx2 - 3;
    if ((rx2 - 3) >= (rx1 - 3))
        left = rx1 - 3;
    if (left <= 0)
        left = 0;

    top     = ry2 - 3;
    if ((ry2 - 3) >= (ry1 - 3))
        top = ry1 - 3;
    if (top <= 0)
        top = 0;

    right     = rx2 + 3;
    if ((rx1 + 3) >= (rx2 + 3))
        right = rx1 + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom     = ry2 + 3;
    if ((ry1 + 3) >= (ry2 + 3))
        bottom = ry1 + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;

    for (uint x = left; x <= right; ++x) {
        for (uint y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx1, ry1, x, y) != 0 || IsVisibleRegion(rx2, ry2, x, y) != 0) {
                auto region = GetRegion(x, y, layer);
                if (region != nullptr)
                    fn(region);
            }
        }
    }
}
