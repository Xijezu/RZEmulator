#ifndef PROJECT_TERRAINPROPINFO_H
#define PROJECT_TERRAINPROPINFO_H

#include "Common.h"

enum PropType : int {
    PT_Unused    = 0,
    PT_UseNAF    = 1,
    PT_UseNX3    = 2,
    PT_SpeedTree = 3,
    PT_NPC       = 4
};

enum RenderType : int {
    RT_General  = 0,
    RT_Building = 1
};

struct PropInfo {
    int         nLineIndex;
    PropType    Type;
    RenderType  mRenderType;
    int         nCategory;
    float       fVisibleRatio;
    int         nShadowFlag;
    std::string strName;
};

class TerrainPropInfo {
public:
    TerrainPropInfo() = default;
    ~TerrainPropInfo() = default;

    bool Initialize(std::string szFileName);
    void Release();

    std::vector<PropInfo> m_pPropInfo{};
    std::vector<std::string> m_CategoryNames{};

private:
    int GetShadowFlag(std::string str);
    bool CheckPropFileType(std::string rname, std::string szTail);
    void SetPropInfo(int nLineIndex, uint16 wPropNum, PropType propType, RenderType renderType, int nCategory, float fVisibleRatio, std::string rName, int nShadowFlag);
};


#endif // PROJECT_TERRAINPROPINFO_H
