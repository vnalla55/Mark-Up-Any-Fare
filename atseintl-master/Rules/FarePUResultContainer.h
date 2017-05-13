//-------------------------------------------------------------------
//
//  File:        FarePUResultContainer.h
//  Created:     Feb 24, 2013
//  Authors:	 Simon Li
//
//  Description: PaxTypeFare PricingUnit scope Rule result reuse
//
//  Copyright Sabre 2013
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

#include <map>

namespace tse
{
class PricingTrx;
class PaxTypeFare;
class FareUsage;
class PricingUnit;

class FarePUResultContainer
{
public:
  bool isWPNC() const { return _isWPNC; }
  bool& isWPNC() { return _isWPNC; }

  bool getPreviousResult(FareUsage& fu, const uint16_t key, bool& isValid) const;
  void saveResultForReuse(const FareUsage& fu, const uint16_t key, bool isValid);
  void clear();

  uint16_t buildRebookStatBitMap(const PricingUnit& pu);

  static FarePUResultContainer* createFarePUResultContainer(PricingTrx& trx);

private:
  std::map<const PaxTypeFare*, std::map<uint16_t, bool> > _resultByPtfWPNC;
  typedef std::map<const PaxTypeFare*, std::map<uint16_t, bool> >::const_iterator FareResultWPNCCI;
  typedef std::map<uint16_t, bool>::const_iterator ResultByKeyCI;
  std::map<const PaxTypeFare*, bool> _resultByPtf;
  typedef std::map<const PaxTypeFare*, bool>::const_iterator FareResultCI;

  bool _isWPNC;
};
}
