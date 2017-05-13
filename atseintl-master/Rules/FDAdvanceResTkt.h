//----------------------------------------------------------------
//
//  File:	       FDAdvanceResTkt.h
//
//  Authors:     Marco Cartolano
//  Created:     March 15, 2005
//  Description: FDAdvanceResTkt class for Fare Display
//
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#pragma once

#include "Rules/AdvanceResTkt.h"

namespace tse
{

class PricingTrx;
class FareDisplayInfo;
class PaxTypeFare;
class FareMarket;
class Itin;

class FDAdvanceResTkt : public AdvanceResTkt
{
  friend class FDAdvanceResTktTest;
  friend class FcDispFareUsageTest;

public:
  FDAdvanceResTkt();
  virtual ~FDAdvanceResTkt();

  virtual Record3ReturnTypes validate(PricingTrx& trx,
                                      Itin& itin,
                                      const PaxTypeFare& fare,
                                      const RuleItemInfo* ruleInfo,
                                      const FareMarket& fareMarket,
                                      bool isQualifiedCategory);

  static const ResPeriod DOLLAR_SIGNS;

private:
  Record3ReturnTypes determineAndValidateTvlSeg(FareDisplayTrx& fdTrx,
                                                const Itin& itin,
                                                const AdvResTktInfo& advanceResTktInfo,
                                                const FareMarket* fareMarket,
                                                const VendorCode& vendor,
                                                DiagManager& diag,
                                                Indicator owrt) const;

  void
  updateFareDisplayInfo(const AdvResTktInfo& advanceResTktInfo, FareDisplayInfo*& fdInfo) const;
  bool needReturnSegment(FareDisplayTrx& fdTrx, Indicator owrt) const;
};

} // namespace tse

