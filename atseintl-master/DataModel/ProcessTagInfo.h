//-------------------------------------------------------------------
//
//  File:        ProcessTagInfo.h
//  Created:     August 9, 2007
//  Authors:     Grzegorz Cholewiak
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

#include "DataModel/FareCompInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryChangesInfo.h"

namespace tse
{
enum ProcessTag
{
  KEEP_THE_FARES = 1,               //  1
  GUARANTEED_AIR_FARE,              //  2
  KEEP_FARES_FOR_TRAVELED_FC,       //  3
  KEEP_FARES_FOR_UNCHANGED_FC,      //  4
  NO_GUARANTEED_FARES,              //  5
  TRAVEL_COMENCEMENT_AIR_FARES,     //  6
  REISSUE_DOWN_TO_LOWER_FARE,       //  7
  CANCEL_AND_START_OVER,            //  8
  HISTORICAL_FARES_FOR_TRAVELED_FC, //  9
  KEEP_FOR_UNCH_CURRENT_FOR_CHNG,   // 10
  KEEP_UP_TO_FIRST_CHNG_THEN_HIST,  // 11
  PT_TAG_WAR_MATRIX_SIZE = 12
};

class ProcessTagInfo
{
public:
  ProcessTagInfo() : _paxTypeFare(nullptr), _overridingPTF(nullptr), _fareCompInfo(nullptr), _isValidTag3(false) {}
  ~ProcessTagInfo() {}

  static constexpr Indicator EXPND_A = 'A';
  static constexpr Indicator EXPND_B = 'B';

  static constexpr Indicator NO_RESTRICTION = ' ';
  static constexpr Indicator FIRST_FC = 'F';
  static constexpr Indicator ORIG_TO_STOPOVER = 'O';

  VoluntaryChangesInfoW* record3() { return &_record3; }
  const VoluntaryChangesInfoW* record3() const { return &_record3; }

  ReissueSequenceW* reissueSequence() { return &_reissueSequence; }
  const ReissueSequenceW* reissueSequence() const { return &_reissueSequence; }

  const PaxTypeFare*& paxTypeFare() { return _paxTypeFare; }
  const PaxTypeFare* paxTypeFare() const { return _paxTypeFare; }

  const PaxTypeFare*& overridingPTF() { return _overridingPTF; }
  const PaxTypeFare* overridingPTF() const { return _overridingPTF; }

  FareCompInfo*& fareCompInfo() { return _fareCompInfo; }
  FareCompInfo* fareCompInfo() const { return _fareCompInfo; }

  const FareMarket* fareMarket() const { return _fareCompInfo->fareMarket(); }
  uint16_t fareCompNumber() const { return _fareCompInfo->fareCompNumber(); }
  int itemNo() const { return _reissueSequence.itemNo(); }
  int seqNo() const { return _reissueSequence.seqNo(); }
  int processTag() const { return _reissueSequence.processingInd(); }

  bool& isValidTag3() { return _isValidTag3; }
  bool isValidTag3() const { return _isValidTag3; }

  bool isOverriden() const { return _record3.overriding() != nullptr; }

private:
  VoluntaryChangesInfoW _record3;
  ReissueSequenceW _reissueSequence;
  const PaxTypeFare* _paxTypeFare;
  const PaxTypeFare* _overridingPTF;
  FareCompInfo* _fareCompInfo;
  bool _isValidTag3;
};
}

