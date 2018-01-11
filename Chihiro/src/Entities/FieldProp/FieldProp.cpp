#include "FieldProp.h"
#include "Object.h"
#include "World.h"

FieldProp *FieldProp::Create(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint lifeTime)
{
    auto fp = new FieldProp{propDeleteHandler, pPropInfo};
    fp->nLifeTime = lifeTime;
    fp->SetCurrentXY(pPropInfo.x, pPropInfo.y);
    fp->m_PropInfo.layer = pPropInfo.layer;
    sWorld->AddObjectToWorld(fp);
    return fp;
}

bool FieldProp::IsUsable(Player *) const
{
    return false;
}

bool FieldProp::Cast() const
{
    return false;
}

bool FieldProp::UseProp(Player *) const
{
    return false;
}

uint FieldProp::GetCastingDelay() const
{
    return 0;
}

FieldProp::FieldProp(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo) : WorldObject(true)
{

}
