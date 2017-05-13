//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DataModel/RepricingTrx.h"

#include "Common/PaxTypeUtil.h"
#include "DataModel/PaxType.h"

namespace tse
{

void
RepricingTrx::addReqPaxType(PaxType* reqPaxType)
{
  if (reqPaxType == nullptr)
    return;

  PaxType* paxType = nullptr;
  _dataHandle.get(paxType);
  if (paxType != nullptr)
  {
    PaxTypeUtil::initialize(*this,
                            *paxType,
                            reqPaxType->paxType(),
                            reqPaxType->number(),
                            reqPaxType->age(),
                            reqPaxType->stateCode(),
                            static_cast<uint16_t>(_paxType.size() + 1));

    _paxType.push_back(paxType);
  }
}

void
RepricingTrx::setupFootNotePrevalidation()
{
  _footNotePrevalidationAllowed = !_skipRuleValidation && _footNotePrevalidationEnabled;
}
}
