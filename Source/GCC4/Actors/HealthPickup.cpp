#pragma once
//========================================================================
// HealthPickup.cpp - An health pickup
//========================================================================


#include "GameCodeStd.h"
#include "HealthPickup.h"
#include "../Utilities/String.h"

const char* HealthPickup::g_Name = "HealthPickup";

bool HealthPickup::VInit(TiXmlElement* pData)
{
    return true;
}

TiXmlElement* HealthPickup::VGenerateXml(void)
{
    TiXmlElement* pComponentElement = GCC_NEW TiXmlElement(VGetName());
    return pComponentElement;
}

void HealthPickup::VApply(WeakActorPtr pActor)
{
    StrongActorPtr pStrongActor = MakeStrongPtr(pActor);
    if (pStrongActor)
    {
        GCC_LOG("Actor", "Applying health pickup to actor id " + ToStr(pStrongActor->GetId()));
    }
}

