//----------------------------------------------------------------------------
//  File:        Diag460Collector.h
//  Authors:     LeAnn Perez
//  Created:     Aug 2004
//
//  Description: Display paxTypeFares with Routing Validation results
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/RoutingDiagCollector.h"

namespace tse
{
class FareMarket;

class Diag460Collector : public RoutingDiagCollector
{
public:
  friend class Diag460CollectorTest;

  explicit Diag460Collector(Diagnostic& root) : RoutingDiagCollector(root) {}
  Diag460Collector() {}

  void displayPaxTypeFares(PricingTrx& trx, FareMarket& fareMarket, const TravelRoute& tvlRoute);

protected:
  class PaxTypeFareFilter : public std::unary_function<const PaxTypeFare*, bool>
  {
  public:
    PaxTypeFareFilter(const FareClassCode fareClass, const FareClassCode fareBasis)
      : _fareClass(fareClass), _fareBasis(fareBasis)
    {
    }
    bool operator()(const PaxTypeFare* ptf) const
    {
      return (_fareClass.empty() || ptf->fareClass() == _fareClass) &&
             (_fareBasis.empty() || ptf->createFareBasis(nullptr) == _fareBasis);
    }

  private:
    FareClassCode _fareClass;
    FareClassCode _fareBasis;
  };

  void displayHeader();
  void displayRoutingStatus(FareMarket& fareMarket,
                            const TravelRoute& tvlRoute,
                            const PaxTypeFareFilter&);

private:
  FareClassCode _fareClass;
  FareClassCode _fareBasis;
};

} // namespace tse

