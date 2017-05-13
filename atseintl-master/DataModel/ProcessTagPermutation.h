//-------------------------------------------------------------------
//
//  File:        ProcessTagPermutation.h
//  Created:     August 8, 2007
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <boost/unordered_map.hpp>

namespace tse
{

class ReissueCharges;
class PaxTypeFare;
class ProcessTagInfo;
class FareMarket;

enum FareApplication // Attention! Sorted by priority!
{
  UNKNOWN_FA = 0,
  TRAVEL_COMMENCEMENT,
  HISTORICAL,
  KEEP,
  CURRENT,
  CANCEL
};

class ProcessTagPermutation
{
public:
  virtual ~ProcessTagPermutation() = default;

  static constexpr Indicator NEW_TICKET_EQUAL_OR_HIGHER_BLANK = ' ';
  static constexpr Indicator NEW_TICKET_EQUAL_OR_HIGHER_B = 'B';
  static constexpr Indicator NEW_TICKET_EQUAL_OR_HIGHER_N = 'N';
  static constexpr Indicator NEW_TICKET_EQUAL_OR_HIGHER_BN = '2';

  static constexpr Indicator ELECTRONIC_TICKET_BLANK = ' ';
  static constexpr Indicator ELECTRONIC_TICKET_REQUIRED = 'R';
  static constexpr Indicator ELECTRONIC_TICKET_NOT_ALLOWED = 'N';
  static constexpr Indicator ELECTRONIC_TICKET_MIXED = 'M';

  static constexpr Indicator STOP_TRY_KEEP = 'Y';

  std::vector<ProcessTagInfo*>& processTags() { return _processTags; }
  const std::vector<ProcessTagInfo*>& processTags() const { return _processTags; }

  FareApplication fareTypeSelection(FCChangeStatus cs) const;

  void setFareTypeSelection(FCChangeStatus cs, FareApplication fa);

  FareApplication rebookFareTypeSelection(FCChangeStatus cs) const;

  void setRebookFareTypeSelection(FCChangeStatus cs, FareApplication fa);

  int& number() { return _number; }
  int number() const { return _number; }

  bool& mixedTags() { return _mixedTags; }
  bool mixedTags() const { return _mixedTags; }

  std::map<FareApplication, ProcessTagInfo*>& fareApplWinnerTags() { return _fareApplWinnerTags; }
  const std::map<FareApplication, ProcessTagInfo*>& fareApplWinnerTags() const
  {
    return _fareApplWinnerTags;
  }

  using PaxTypeFareApplication = boost::unordered_map<const PaxTypeFare*, FareApplication>;
  PaxTypeFareApplication& fareApplMap() { return _fareApplMap; }
  const PaxTypeFareApplication& fareApplMap() const { return _fareApplMap; }

  ReissueCharges*& reissueCharges() { return _reissueCharges; }
  const ReissueCharges* reissueCharges() const { return _reissueCharges; }
  bool needKeepFare() const;

  static constexpr Indicator ENDORSEMENT_BLANK = ' ';
  static constexpr Indicator ENDORSEMENT_W = 'W';
  static constexpr Indicator ENDORSEMENT_X = 'X';
  static constexpr Indicator ENDORSEMENT_Y = 'Y';
  static constexpr Indicator REISSUE_TO_LOWER_BLANK = ' ';
  static constexpr Indicator REISSUE_TO_LOWER_F = 'F';
  static constexpr Indicator REISSUE_TO_LOWER_R = 'R';

  static constexpr Indicator RESIDUAL_BLANK = ' ';
  static constexpr Indicator RESIDUAL_I = 'I';
  static constexpr Indicator RESIDUAL_N = 'N';
  static constexpr Indicator RESIDUAL_R = 'R';
  static constexpr Indicator RESIDUAL_S = 'S';

  static constexpr Indicator STOPCONN_BLANK = ' ';
  static constexpr Indicator STOPCONN_S = 'S';
  static constexpr Indicator STOPCONN_C = 'C';
  static constexpr Indicator STOPCONN_B = 'B';

  Indicator getEndorsementByte() const;
  bool isOverriden() const;
  Indicator getReissueToLowerByte() const;

  bool hasTag7only() const;
  bool tag1StopYonly() const;
  const ProcessTagInfo* firstWithT988() const;
  bool hasZeroT988() const;

  bool isNMostRestrictiveResidualPenaltyInd() const;
  Indicator getResidualPenaltyByte() const;

  Indicator formOfRefundInd() const;

  virtual Indicator checkTable988Byte156() const;
  virtual Indicator checkTable988Byte123();

  Indicator getStopoverConnectionByte() const;

  virtual const Indicator electronicTicket() const { return _electronicTicket; }
  bool needExpndKeepFare(const FareMarket& excFm) const;

  static bool requiresComparisonOfExcAndNewNonRef(Indicator endorsementByte)
  {
    return endorsementByte != ENDORSEMENT_W;
  }

  const MoneyAmount& getEstimatedChangeFee() const;
  void setEstimatedChangeFee(MoneyAmount estimatedChangeFee);

private:
  bool residualIndsSame() const;
  const ProcessTagInfo* findProcessTagInfo(const FareMarket& fm) const;

  std::vector<ProcessTagInfo*> _processTags;
  FareApplication _uu = FareApplication::UNKNOWN_FA;
  FareApplication _uc = FareApplication::UNKNOWN_FA;
  FareApplication _un = FareApplication::UNKNOWN_FA;
  FareApplication _fl = FareApplication::UNKNOWN_FA;

  FareApplication _rbun = FareApplication::UNKNOWN_FA;
  FareApplication _rbuu = FareApplication::UNKNOWN_FA;
  FareApplication _rbuc = FareApplication::UNKNOWN_FA;
  FareApplication _rbfl = FareApplication::UNKNOWN_FA;

  int _number = 0;
  bool _mixedTags = false;

  PaxTypeFareApplication _fareApplMap;
  std::map<FareApplication, ProcessTagInfo*> _fareApplWinnerTags;

  ReissueCharges* _reissueCharges = nullptr;
  MoneyAmount _estimatedChangeFee = 0.0;
  Indicator _electronicTicket = ' ';
};

} // tse

