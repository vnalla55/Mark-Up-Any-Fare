//-------------------------------------------------------------------
//
//  File:        PaxTypeFareRuleData.h
//  Created:     August 12, 2004
//  Design:      Mark Kasprowicz
//  Authors:
//
//  Description: Class wraps PaxTypeFare Rule Creation Records
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/SmallBitSet.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DataHandle.h"

#include <vector>

namespace tse
{
class RuleItemInfo;
class PaxTypeFare;
class DataHandle;
class FBRPaxTypeFareRuleData;
class NegPaxTypeFareRuleData;

class PaxTypeFareRuleData
{
public:
  PaxTypeFareRuleData() = default;
  PaxTypeFareRuleData(const PaxTypeFareRuleData&) = delete;
  PaxTypeFareRuleData& operator=(const PaxTypeFareRuleData&) = delete;

  virtual ~PaxTypeFareRuleData() = default;

  const CategoryRuleInfo* categoryRuleInfo() const { return _categoryRuleInfo; }
  const CategoryRuleInfo*& categoryRuleInfo() { return _categoryRuleInfo; }

  bool isLocationSwapped() const { return _isLocationSwapped; }
  bool& isLocationSwapped() { return _isLocationSwapped; }

  const CategoryRuleItemInfoSet* categoryRuleItemInfoSet() const
  {
    return _categoryRuleItemInfoSet;
  }
  const CategoryRuleItemInfoSet*& categoryRuleItemInfoSet() { return _categoryRuleItemInfoSet; }

  const std::vector<CategoryRuleItemInfo>* categoryRuleItemInfoVec() const
  {
    return _categoryRuleItemInfoVec;
  }
  const std::vector<CategoryRuleItemInfo>*& categoryRuleItemInfoVec()
  {
    return _categoryRuleItemInfoVec;
  }

  const CategoryRuleItemInfo* categoryRuleItemInfo() const { return _categoryRuleItemInfo; }
  const CategoryRuleItemInfo*& categoryRuleItemInfo() { return _categoryRuleItemInfo; }

  const RuleItemInfo* ruleItemInfo() const { return _ruleItemInfo; }
  const RuleItemInfo*& ruleItemInfo() { return _ruleItemInfo; }
  PaxTypeFare* baseFare() const { return _baseFare; }
  PaxTypeFare*& baseFare() { return _baseFare; }

  virtual PaxTypeFareRuleData* clone(DataHandle& dataHandle) const;
  void copyTo(PaxTypeFareRuleData& cloneObj) const;

  void setSoftPassDiscount(bool isSoftPass) { _ruleStatus.set(RS_SoftPassDiscount, isSoftPass); }
  bool isSoftPassDiscount() const { return _ruleStatus.isSet(RS_SoftPassDiscount); }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers;}

  virtual FBRPaxTypeFareRuleData* toFBRPaxTypeFareRuleData() { return nullptr; }

  virtual const FBRPaxTypeFareRuleData* toFBRPaxTypeFareRuleData() const { return nullptr; }

  virtual NegPaxTypeFareRuleData* toNegPaxTypeFareRuleData() { return nullptr; }

  virtual const NegPaxTypeFareRuleData* toNegPaxTypeFareRuleData() const { return nullptr; }

protected:
  // 'original' refers to the fare used to create this fare
  PaxTypeFare* _baseFare = nullptr; // original Base Fare
  const CategoryRuleInfo* _categoryRuleInfo = nullptr; // Rec2
  bool _isLocationSwapped = false; // true if matching Geo in Rec2 with loc swapped
  const CategoryRuleItemInfoSet* _categoryRuleItemInfoSet = nullptr; // original Rec2 set
  const std::vector<CategoryRuleItemInfo>* _categoryRuleItemInfoVec =
      nullptr; // original Record 2 vector
  const CategoryRuleItemInfo* _categoryRuleItemInfo = nullptr; // original Rec2 segment
  const RuleItemInfo* _ruleItemInfo = nullptr; // Rec3
  std::vector<CarrierCode> _validatingCarriers;

  enum RuleState
  {
    RS_Null = 0x00,
    RS_SoftPassDiscount = 0x01
  };

  using RuleStatus = SmallBitSet<uint8_t, RuleState>;

private:
  RuleStatus _ruleStatus = RuleState::RS_Null;
};

} // tse namespace

