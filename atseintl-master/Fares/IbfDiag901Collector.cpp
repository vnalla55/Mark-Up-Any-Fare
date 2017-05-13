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

#include "Fares/IbfDiag901Collector.h"

#include "BrandedFares/BrandProgram.h"
#include "Common/ErrorResponseException.h"
#include "Common/ShoppingUtil.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"

#include <sstream>

namespace tse
{

BuildMarketRequestData::BuildMarketRequestData(IAIbfUtils::OdcTuple odc,
                                               size_t fmCount,
                                               std::vector<PaxTypeCode> paxTypes)
  : odcTuple(odc), fareMarketsCount(fmCount), paxes(paxTypes)
{
}

IbfDiag901Collector::IbfDiag901Collector(PricingTrx& trx) : _trx(trx), _diag(trx, Diagnostic901)
{
  if (_diag.isActive() && trx.diagnostic().diagnosticType() == Diagnostic901)
  {
    _isActive = true;
    if (_trx.diagnostic().diagParamMapItem("IBF") == "DETAILS")
    {
      _detailsEnabled = true;
    }
  }
}

void
IbfDiag901Collector::collectDataSentToPricing(const IAIbfUtils::OdcTuple& odcTuple,
                                              const size_t fareMarketsCount,
                                              const std::vector<PaxTypeCode>& paxes)
{
  _dataSentToPricingVec.push_back(BuildMarketRequestData(odcTuple, fareMarketsCount, paxes));
}

void
IbfDiag901Collector::collectFareMarket(const IndexVector& legs,
                                       const FareMarket* const fm,
                                       bool isThru)
{
  _fareMarketsPerLeg[legs].push_back(std::make_pair(fm, isThru));
}

void
IbfDiag901Collector::collectCommonBrandsOnLegs(BrandCodeSetVec& brandsPerLeg)
{
  _brandsPerLeg = brandsPerLeg;
}

void
IbfDiag901Collector::collectBrandsRemovedFromFareMarket(const FareMarket* fm,
                                                        const std::vector<int>& brandsRemoved)
{
  _brandsRemovedPerMarket[fm] = brandsRemoved;
}

void
IbfDiag901Collector::collectCommonBrandsForAllLegs(BrandCodeSet& brands)
{
  _brandsCommonForAllLegs = brands;
}
void
IbfDiag901Collector::flush(Diag901Stage stage)
{
  // ITIN ANALYZER
  if (stage == ITIN_ANALYZER)
  {
    if (_detailsEnabled)
    {
      _diag << " IBF: ITIN ANALYZER:\n\n";
      _diag << " Sent buildMarketRequest to Pricing with following parameters:\n\n";
      for (BuildMarketRequestData data : _dataSentToPricingVec)
      {
        std::ostringstream oss;
        oss << data.odcTuple;
        oss << " Fare Markets count: " << data.fareMarketsCount;
        oss << " PaxTypes: ";
        for (PaxTypeCode pax : data.paxes)
          oss << pax << " ";

        _diag << oss.str() << "\n";
      }
    }
    return;
  }

  if (stage == FCO_BRAND_EXTRACT)
  {
    // FARE COLLECTOR
    std::vector<QualifiedBrand>& brandsFromTrx = _trx.brandProgramVec();
    _diag << "\n IBF: FARE COLLECTOR:\n\n";
    BrandedDiagnosticUtil::displayAllBrandIndices(_diag.collector(), brandsFromTrx);
    _diag << "* * * * * * * * * * * * * * * * * * * * * * * * * * * *\n";
    _diag << "*******************************************************\n\n";

    // Looking for brands

    if (_noBrandsFound)
    {
      _diag << " No Brands Found \n\n";
      _diag.collector().flushMsg();
      return;
    }

    if (_detailsEnabled)
    {
      std::map<const IndexVector, std::vector<std::pair<const FareMarket*, bool> > >::iterator
      leg_iterator;
      for (leg_iterator = _fareMarketsPerLeg.begin(); leg_iterator != _fareMarketsPerLeg.end();
           ++leg_iterator)
      {
        _diag << "Looking for brands on legs: ";
        for (int i : leg_iterator->first)
        {
          _diag << i << " ";
        }
        _diag << "\n";
        std::vector<std::pair<const FareMarket*, bool> >::iterator fmIterator;
        for (fmIterator = leg_iterator->second.begin(); fmIterator != leg_iterator->second.end();
             ++fmIterator)
        {
          _diag << "    Processing fare Market: ";
          if (!(fmIterator->second))
            _diag << "(local) ";
          _diag << fmIterator->first->origin()->loc() << "-"
                << fmIterator->first->governingCarrier() << "-"
                << fmIterator->first->destination()->loc();
          _diag << "\n";
          _diag << "    Brands/Programs available: ";
          BrandCodeSet commonBrands;
          for (auto& elem : fmIterator->first->brandProgramIndexVec())
          {
            _diag << elem << " ";
            commonBrands.insert(_trx.brandProgramVec()[elem].second->brandCode());
          }
          _diag << "\n";
          _diag << "    Unique brand codes: ";
          for (const auto& commonBrand : commonBrands)
            _diag << commonBrand << " ";
          _diag << "\n";
        }
        _diag << "\n";
      }
    }

    _diag << "Brands Per Leg - Summary\n";
    for (unsigned int i = 0; i < _brandsPerLeg.size(); ++i)
    {
      _diag << "    Leg " << i << " Brands: ";
      for (const BrandCode& brand : _brandsPerLeg[i])
        _diag << brand << " ";

      _diag << "\n";
    }
    _diag << "\n";

    _diag << "Brands common for all legs: ";
    for (const BrandCode& brand : _brandsCommonForAllLegs)
      _diag << brand << " ";

    _diag << "\n\n";

    if (!_detailsEnabled)
    {
      _diag.collector().flushMsg();
      return;
    }
    return;
  }
  // Brand Removal
  _diag << "*******************************************************\n";
  _diag << " Validating brand parity\n";
  _diag << "*******************************************************\n";
  _diag << " Brand Statuses:\n";
  _diag << "   'F' - BS_FAIL\n";
  _diag << "   'H' - BS_HARD_PASS\n";
  _diag << "   'S' - BS_SOFT_PASS\n";
  _diag << "*******************************************************\n";

  std::map<const FareMarket*, std::vector<int> >::iterator fareMarkets;
  for (fareMarkets = _brandsRemovedPerMarket.begin(); fareMarkets != _brandsRemovedPerMarket.end();
       ++fareMarkets)
  {
    const FareMarket* fm = fareMarkets->first;
    std::vector<int> brandsRemoved = fareMarkets->second;
    _diag << "*******************************************************\n";
    _diag << "  Fare Market : " << fm->origin()->loc() << "-" << fm->governingCarrier() << "-"
          << fm->destination()->loc() << "\n";
    _diag << "    Brands/Programs Removed : ";
    if (brandsRemoved.size() == 0)
      _diag << " None";
    BrandCodeSet commonBrands;
    for (unsigned int brandId : brandsRemoved)
    {
      _diag << brandId << " ";
      commonBrands.insert(_trx.brandProgramVec()[brandId].second->brandCode());
    }
    _diag << "\n";
    if (!brandsRemoved.size() == 0)
    {
      _diag << "    Unique brand codes: ";
      for (const auto& commonBrand : commonBrands)
        _diag << commonBrand << " ";
      _diag << "\n";
    }

    _diag << "    Brands/Programs Valid : ";
    BrandCodeSet validBrandCodes;
    for (unsigned int brandId : fm->brandProgramIndexVec())
    {
      _diag << brandId << " ";
      validBrandCodes.insert(_trx.brandProgramVec()[brandId].second->brandCode());
    }
    _diag << "\n";
    _diag << "    Valid Brand Codes : ";
    for (const auto& validBrandCode : validBrandCodes)
      _diag << validBrandCode << " ";
    _diag << "\n";
    if (fm->failCode() != ErrorResponseException::NO_ERROR)
    {
      ErrorResponseException error(fm->failCode());
      const char* errorCode = error.what();
      _diag << "\n    Error Code = " << errorCode << "\n";
      continue;
    }
    _diag << "    \n----------------------------------------\n";
    _diag << "    Fares and Brands(Programs):\n";
    for (PaxTypeFare* fare : fm->allPaxTypeFare())
    {
      _diag << "      Fare: " << fare->createFareBasis(_trx, false);

      if (fare->getBrandStatusVec().size() > 0)
      {
        for (uint16_t i = 0; i < fare->fareMarket()->brandProgramIndexVec().size(); ++i)
        {
          int brandIndex = fare->fareMarket()->brandProgramIndexVec()[i];
          _diag << "\n         -" << ShoppingUtil::getBrandCode(_trx, brandIndex) << " ("
                << ShoppingUtil::getProgramId(_trx, brandIndex) << ") - "
                << static_cast<char>(fare->getBrandStatusVec()[i].first);
        }
      }
      else
        _diag << "\tNo brands";
      _diag << "\n";
    }
  }

  _diag.collector().flushMsg();
}
} // end namespace tse
