//-------------------------------------------------------------------
//
//  File:        TagWarEngine.h
//  Created:     August 22, 2007
//  Authors:     Daniel Rolka
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"

#include <vector>

namespace tse
{

class TagWarEngine
{
public:
  friend class TagWarEngineTest;

  static const FareApplication _faMatrix[(int)PT_TAG_WAR_MATRIX_SIZE][(int)FCS_TAG_WAR_MATRIX_SIZE];

  TagWarEngine();
  ~TagWarEngine();
  static FareApplication
  getFA(ProcessTagPermutation& perm, FCChangeStatus status, bool travelCommenced);
  static FareApplication getMatrixValue(int tag, FCChangeStatus status, bool travelCommenced);
};
}

