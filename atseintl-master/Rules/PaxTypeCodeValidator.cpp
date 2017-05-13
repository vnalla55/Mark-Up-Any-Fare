//-------------------------------------------------------------------
//
//  File:        PaxTypeCodeValidator.cpp
//  Created:     June 22, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc. All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Rules/PaxTypeCodeValidator.h"

#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxType.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{

bool
PaxTypeCodeValidator::validate(uint32_t itemNoR3,
                               const PaxType& paxType,
                               const PaxTypeCode& psgType)
{
  const bool result = (psgType.empty() || psgType == ADULT || paxType.paxType() == psgType);

  if (LIKELY(!_dc))
    return result;

  *_dc << "PTC: " << psgType << "   " << _label << " PTC: " << paxType.paxType() << '\n';

  if (!result)
    *_dc << "  FAILED ITEM " << itemNoR3 << " - PTC NOT MATCHED\n";

  return result;
}
}
