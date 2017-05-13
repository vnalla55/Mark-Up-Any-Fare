//-------------------------------------------------------------------
//
//  File:        TagWarEngine.cpp
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
#include "RexPricing/TagWarEngine.h"

namespace tse
{

const FareApplication
TagWarEngine::_faMatrix[(int)PT_TAG_WAR_MATRIX_SIZE][(int)FCS_TAG_WAR_MATRIX_SIZE] = {
  // UK                UU                   UN                   FL                   UC
  { UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA }, // process tag 0, not supported
  { UNKNOWN_FA, KEEP, KEEP, KEEP, KEEP }, // process tag 1
  { UNKNOWN_FA, HISTORICAL, HISTORICAL, HISTORICAL, HISTORICAL }, // process tag 2
  { UNKNOWN_FA, CURRENT, CURRENT, KEEP, CURRENT }, // process tag 3
  { UNKNOWN_FA, KEEP, CURRENT, KEEP, CURRENT }, // process tag 4
  { UNKNOWN_FA, CURRENT, CURRENT, CURRENT, CURRENT }, // process tag 5
  { UNKNOWN_FA, TRAVEL_COMMENCEMENT, TRAVEL_COMMENCEMENT, TRAVEL_COMMENCEMENT,
    TRAVEL_COMMENCEMENT }, // process tag 6
  { UNKNOWN_FA, CURRENT, CURRENT, CURRENT, CURRENT }, // process tag 7
  { UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA, UNKNOWN_FA }, // process tag 8, not supported
  { UNKNOWN_FA, CURRENT, CURRENT, HISTORICAL, CURRENT }, // process tag 9
  { UNKNOWN_FA, KEEP, KEEP, KEEP, CURRENT }, // process tag 10
  { UNKNOWN_FA, KEEP, HISTORICAL, KEEP, HISTORICAL } // process tag 11
};

TagWarEngine::TagWarEngine() { ; }

TagWarEngine::~TagWarEngine() { ; }

FareApplication
TagWarEngine::getFA(ProcessTagPermutation& perm, FCChangeStatus status, bool travelCommenced)
{
  std::vector<ProcessTagInfo*>::const_iterator iter;
  FareApplication curFA, retFA;
  ProcessTagInfo* winnerTag = nullptr;

  curFA = retFA = UNKNOWN_FA;
  int curProcessTag = 0;

  for (iter = perm.processTags().begin(); iter < perm.processTags().end(); iter++)
  {
    if (!(*iter)->reissueSequence()->orig())
      continue;

    curProcessTag = (*iter)->processTag();

    curFA = TagWarEngine::getMatrixValue(curProcessTag, status, travelCommenced);
    if (retFA < curFA ||
        (curFA == HISTORICAL && retFA == curFA && curProcessTag > winnerTag->processTag()))
    {
      retFA = curFA;
      winnerTag = *iter;
    }
  }

  if (winnerTag != nullptr)
  {
    perm.fareApplWinnerTags().insert(std::make_pair(retFA, winnerTag));
  }

  return retFA;
}

FareApplication
TagWarEngine::getMatrixValue(int tag, FCChangeStatus status, bool travelCommenced)
{
  if (tag == TRAVEL_COMENCEMENT_AIR_FARES && !travelCommenced)
    return CURRENT;

  return TagWarEngine::_faMatrix[tag][(int)status];
}
}