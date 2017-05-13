//-------------------------------------------------------------------
//
//  Authors:    Michal Mlynek
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

#include "BrandedFares/BrandInfo.h"
#include "Common/IAIbfUtils.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag901Collector.h"
#include "Diagnostic/DiagManager.h"

namespace tse
{

struct BuildMarketRequestData
{
  BuildMarketRequestData(IAIbfUtils::OdcTuple odcTuple,
                         size_t fmCount,
                         std::vector<PaxTypeCode> paxTypes);

  IAIbfUtils::OdcTuple odcTuple;
  size_t fareMarketsCount;
  std::vector<PaxTypeCode> paxes;
};

enum Diag901Stage
{
  ITIN_ANALYZER,
  FCO_BRAND_EXTRACT,
  FCO_BRAND_PARITY
};

class IbfDiag901Collector : public Diag901Collector
{
public:
  IbfDiag901Collector(PricingTrx& trx);
  bool isActive() const { return _isActive; }
  void collectDataSentToPricing(const IAIbfUtils::OdcTuple& odcTuple,
                                const size_t fareMarketsCount,
                                const std::vector<PaxTypeCode>& paxes);

  void collectFareMarket(const IndexVector&, const FareMarket* const, bool isThru);
  void collectCommonBrandsOnLegs(BrandCodeSetVec& brandsPerLeg);
  void collectCommonBrandsForAllLegs(BrandCodeSet& brands);
  void
  collectBrandsRemovedFromFareMarket(const FareMarket* fm, const std::vector<int>& brandsRemoved);
  void noBrandsFound() { _noBrandsFound = true; }
  void flush(Diag901Stage);

private:
  IbfDiag901Collector(const IbfDiag901Collector& right);
  IbfDiag901Collector& operator=(const IbfDiag901Collector& right);

  std::vector<BuildMarketRequestData> _dataSentToPricingVec;
  BrandCodeSetVec _brandsPerLeg;
  BrandCodeSet _brandsCommonForAllLegs;
  std::map<const IndexVector, std::vector<std::pair<const FareMarket*, bool> > > _fareMarketsPerLeg;
  std::map<const FareMarket*, std::vector<int> > _brandsRemovedPerMarket;

  PricingTrx& _trx;
  DiagManager _diag;
  bool _isActive = false;
  bool _detailsEnabled = false;
  bool _noBrandsFound = false;
};

} // end namespace tse
