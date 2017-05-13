//----------------------------------------------------------------------------
//  File:        Diag982Collector.h
//  Created:     2008-08-20
//    Authors:     Marek Sliwa
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

#include <iosfwd>
#include <map>

namespace tse
{
class Diag982Collector : public DiagCollector
{
public:
  Diag982Collector() : _pricingTrx(nullptr) {};

  Diag982Collector& operator<<(const PricingTrx& pricingTrx) override;
  void outputAltDates(const PricingTrx& trx);
  void showMIPItin(const std::map<int, Itin*>& itinMap);
  void displayItin(const Itin& itin);
  void displaySplittedItins(PricingTrx& trx,
                            const std::vector<Itin*>& splittedItins,
                            const Itin* excItin,
                            const std::vector<Itin*>& excRemovedItins,
                            const std::vector<Itin*>& excSplittedItins);
  void displayRemovedItins(const std::vector<Itin*>::const_iterator begin,
                           const std::vector<Itin*>::const_iterator end,
                           const std::string& msg);
  void displayItins(const PricingTrx& trx);
  void displayKeySegments(const PricingTrx::ClassOfServiceKey& key);

  std::vector<std::string> getMotherAvailabilityAsString(const PricingTrx& trx, const Itin& itin);

private:
  const PricingTrx* _pricingTrx;
};
}
