//-------------------------------------------------------------------
//
//  File:        FBRPaxTypeFareRuleData.h
//  Created:     August 12, 2004
//  Design:      Joe Yglesias
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

#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"

namespace tse
{
class FareByRuleApp;
class PaxTypeFare;
class DataHandle;

class FBRPaxTypeFareRuleData : public PaxTypeFareRuleData
{
public:
  FBRPaxTypeFareRuleData() = default;

  const FareByRuleApp* fbrApp() const { return _fbrApp; }
  const FareByRuleApp*& fbrApp() { return _fbrApp; }

  const long baseFareBookingCodeTblItemNo() const { return _baseFareBookingCodeTblItemNo; }
  long& baseFareBookingCodeTblItemNo() { return _baseFareBookingCodeTblItemNo; }

  bool getBaseFarePrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const;

  void setBaseFarePrimeBookingCode(std::vector<BookingCode>& baseFareBookingCodeVec);

  bool isSpecifiedFare() const
  {
    const FareByRuleItemInfo* fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(_ruleItemInfo);
    return (fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED ||
            fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_K ||
            fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_E ||
            fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED_F);
  }

  bool isR8LocationSwapped() const { return _isR8LocationSwapped; }
  bool& isR8LocationSwapped() { return _isR8LocationSwapped; }

  bool isMinMaxFare() const { return _isMinMaxFare; }
  bool& isMinMaxFare() { return _isMinMaxFare; }

  bool isBaseFareAvailBkcMatched() const { return _isBaseFareAvailBkcMatched; }
  bool& isBaseFareAvailBkcMatched() { return _isBaseFareAvailBkcMatched; }

  std::set<BookingCode>& baseFareInfoBookingCodes() { return _baseFareInfoBookingCodes; }
  const std::set<BookingCode>& baseFareInfoBookingCodes() const
  {
    return _baseFareInfoBookingCodes;
  }

  bool isSpanishResidence() const { return _isSpanishResidence; }
  bool& isSpanishResidence() { return _isSpanishResidence; }

  FBRPaxTypeFareRuleData* clone(DataHandle& dataHandle) const override;
  void copyTo(FBRPaxTypeFareRuleData& cloneObj) const;

  FBRPaxTypeFareRuleData* toFBRPaxTypeFareRuleData() override { return this; }

  const FBRPaxTypeFareRuleData* toFBRPaxTypeFareRuleData() const override { return this; }

private:
  const FareByRuleApp* _fbrApp = nullptr; // record 8
  long _baseFareBookingCodeTblItemNo = 0; // Base Fare Table 999
  BookingCode _baseFareBookingCode[FareClassAppSegInfo::BK_CODE_SIZE];
  bool _isR8LocationSwapped = false; // record 8 location swapped
  bool _isMinMaxFare = false; // true when cat 25 fare is from a min/max limit (F/IND H/L)
  bool _isBaseFareAvailBkcMatched = false; // true when base fare is matched with Bkg Code
  // followed by * in Table 989
  std::set<BookingCode> _baseFareInfoBookingCodes;
  bool _isSpanishResidence = false;
};

} // tse namespace

