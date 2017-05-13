#pragma once

#include "Diagnostic/DiagCollector.h"

#include <string>

#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class Diag202Collector : public DiagCollector
{
public:
  static const std::string FBR;
  static const std::string FNT;
  static const std::string GFR;

  static Diag202Collector* makeMe(PricingTrx& trx,
                                  const std::string* r2type = nullptr,
                                  const FareMarket* fareMarket = nullptr,
                                  const PaxTypeFare* ptf = nullptr);

  template <class Rec2Type>
  void
  printR2sMatchDetails(const std::string& r2Type,
                       PricingTrx& trx,
                       const std::vector<Rec2Type*>& r2vec,
                       const std::vector<std::pair<const Rec2Type*, GeoMatchResult>>& resultVec,
                       const FareMarket& fareMarket,
                       const CarrierCode& cxr,
                       const DateTime& travelDate,
                       const PaxTypeFare* ptf = nullptr);
};
}
