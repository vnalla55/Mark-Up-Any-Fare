/*---------------------------------------------------------------------------
 *  File:    FareMarketMerger.h
 *  Created: Oct 03, 2004
 *  Authors: Mohammad Hossan
 *
 *  Copyright Sabre 2003
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/ShoppingUtil.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "Diagnostic/Diag993Collector.h"
#include "Pricing/GroupsFilter.h"
#include "Pricing/MergedFareMarket.h"
#include "RexPricing/RexPaxTypeFareValidator.h"
#include "Rules/RuleConst.h"

#include <list>
#include <vector>

namespace tse
{

class Itin;
class PricingTrx;
class FareMarket;

class FareMarketMerger
{
public:
  FareMarketMerger(PricingTrx& trx,
                   Itin& itin,
                   bool searchAlwaysLowToHigh,
                   const CarrierCode* carrier,
                   uint16_t brandIndex = 0,
                   BrandCode* brandCode = nullptr,
                   flexFares::GroupId groupId = 0,
                   Diag993Collector* diagPtr = nullptr)
    : _trx(trx),
      _itin(itin),
      _searchAlwaysLowToHigh(searchAlwaysLowToHigh),
      _carrier(carrier),
      _brandIndex(brandIndex),
      _brandCode(brandCode),
      _groupId(groupId),
      _diagPtr(diagPtr)
  {
    _diag993InCbsMode = (_diagPtr != nullptr) &&
        (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "CBS");
  }

  ~FareMarketMerger() {}

  bool buildAllMergedFareMarket(std::vector<MergedFareMarket*>& mergedFareMarketVect);
  bool buildAllMergedFareMarket_old(std::vector<MergedFareMarket*>& mergedFareMarketVect);

  //--------------------------------------------------------------

  class IndustryFare : std::unary_function<PaxTypeFare*, bool>
  {
  public:
    bool operator()(const PaxTypeFare* ptf) { return (ptf->carrier() == INDUSTRY_CARRIER); }
  };

  class CxrPrefFareComparator
  {
  public:
    CxrPrefFareComparator(PaxTypeBucket& cortege, uint16_t paxTypeNum, bool compareCabins);
    bool operator()(const PaxTypeFare* lptf, const PaxTypeFare* rptf);

  private:
    PaxTypeBucket& _cortege;
    uint16_t _paxTypeNum;
    PaxTypeFare::CabinComparator _cabinComparator;
  };

  //--------------------------------------------------------------

  class ValidPaxTypeFareCopier : public std::unary_function<PaxTypeFare*, void>
  {
  public:
    ValidPaxTypeFareCopier(PricingTrx& trx,
                           Itin* itin,
                           uint16_t paxTypeNum,
                           std::vector<PaxTypeFare*>& ptfVect,
                           const skipper::CarrierBrandPairs& carriersBrands,
                           const CarrierCode* carrier = nullptr,
                           RexPaxTypeFareValidator* rexPTFValidator = nullptr,
                           uint16_t brandIndex = 0,
                           const BrandCode* brandCode = nullptr,
                           flexFares::GroupId groupId = 0,
                           Diag993Collector* diagPtr = nullptr,
                           bool* hardPassExists = nullptr)
      : _paxTypeFareVect(ptfVect),
        _paxTypeNum(paxTypeNum),
        _checkFareRetrievalDate(false),
        _fareBytesData(nullptr),
        _trx(trx),
        _itin(itin),
        _carriersBrands(carriersBrands),
        _carrier(carrier),
        _rexPTFValidator(rexPTFValidator),
        _brandIndex(brandIndex),
        _brandCode(brandCode),
        _groupId(groupId),
        _hardPassExists(hardPassExists),
        _validFaresCount(0),
        _tag2FareIndicator(false),
        _diagPtr(diagPtr)

    {
      _isPrivateFares = trx.getOptions()->isPrivateFares();
      _isPublishedFares = trx.getOptions()->isPublishedFares();
      _fareGroupRequested = trx.getRequest()->fareGroupRequested();

      if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
      {
        RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(trx);
        if (rexTrx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
        {
          _checkFareRetrievalDate = true;
          _fareBytesData = &rexTrx.fareBytesData();
        }
      }
    }

    bool isValidForFareBytes(PaxTypeFare* ptf);
    bool isValidForBrandCombination(PaxTypeFare* ptf);

    bool isValidForBrand(PaxTypeFare* ptf)
    {
      if (_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing())
        return isValidForBrandCombination(ptf);

      if (*_brandCode == NO_BRAND)
        return true;

      if (*_brandCode == ANY_BRAND)
        return ptf->hasValidBrands();

      if (*_brandCode == ANY_BRAND_LEG_PARITY)
      {
        if (!ptf->isValidForRequestedBrands(_trx, _itin->brandFilterMap(), false))
          return false;
      }

      PaxTypeFare::BrandStatus brandStatus = ptf->getBrandStatus(_trx, _brandCode);
      if (brandStatus == PaxTypeFare::BS_FAIL)
        return false;

      if (_hardPassExists && (brandStatus == PaxTypeFare::BS_HARD_PASS))
        *_hardPassExists = true;

      return true;
    }

    bool isValidForCarrier(PaxTypeFare* ptf)
    {
      if (LIKELY(!_carrier ||
          !_trx.isIataFareSelectionApplicable() ||
          (PricingTrx::IS_TRX != _trx.getTrxType())))
        return true;

      ItinIndex::Key carrierKey;
      ShoppingUtil::createCxrKey(*_carrier, carrierKey);

      if (!_trx.isAltDates())
      {
        ptf->setComponentValidationForCarrier(carrierKey, false, 0);
        if (!ptf->isFlightBitmapInvalid())
          return true;

        return false;
      }

      // Alt dates code
      if (ptf->durationFlightBitmapPerCarrier().empty())
        return true;

      typedef VecMap<uint64_t, PaxTypeFare::FlightBitmap> DurationFltBitmap;
      const VecMap<uint32_t, DurationFltBitmap>::const_iterator i =
        ptf->durationFlightBitmapPerCarrier().find(carrierKey);

      if (i == ptf->durationFlightBitmapPerCarrier().end())
        return true;

      for (const DurationFltBitmap::value_type& durFltBm : i->second)
        if (!ptf->isFlightBitmapInvalid(durFltBm.second))
          return true;

      return false; // not valid for any durations for _carrier.
    }

    bool isValidForFlexFares(PaxTypeFare* ptf);
    void operator()(PaxTypeFare* paxTypeFare);

    uint32_t validFares() { return _validFaresCount; }
    bool tag2FareIndicator() { return _tag2FareIndicator; }
    bool isMatchedAcctCodeCorpID(const PaxTypeFare* ptf, flexFares::GroupId groupID);
    bool isJumpCabinLogicValidForFlexFares(const PaxTypeFare* ptf, flexFares::GroupId grpId);

  private:
    std::vector<PaxTypeFare*>& _paxTypeFareVect;
    const uint16_t _paxTypeNum;
    bool _isPrivateFares;
    bool _isPublishedFares;
    bool _fareGroupRequested;
    bool _checkFareRetrievalDate;
    RexPricingTrx::BoardOffToFareBytesData* _fareBytesData;
    PricingTrx& _trx;
    Itin* _itin;
    const skipper::CarrierBrandPairs& _carriersBrands;
    const CarrierCode* _carrier;
    RexPaxTypeFareValidator* _rexPTFValidator;
    uint16_t _brandIndex;
    const BrandCode* _brandCode;
    flexFares::GroupId _groupId;
    bool* _hardPassExists;
    uint32_t _validFaresCount;
    bool _tag2FareIndicator;
    Diag993Collector* _diagPtr;
  };

protected:
  bool mergeFareMarket(std::vector<FareMarket*>& allGovCxrFM,
                       MergedFareMarket& mfm);

  bool mergeFareMarket_old(std::vector<FareMarket*>& allGovCxrFM,
                           MergedFareMarket& mfm);

  void fillMergedFmDataFromFm(FareMarket& mergedFm, const FareMarket& fm);
  IbfErrorMessage mergeSoldoutStatuses(FareMarket* mergedFm,
                                       MergedFareMarket& mfm,
                                       const std::vector<FareMarket*>& fareMarkets);

  bool copyFaresToMergedFareMarket(FareMarket& fm,
                                   FareMarket& mergedFM,
                                   MergedFareMarket& mfm,
                                   Diag993Collector* diagPtr = nullptr);

  bool changeBrandForSoldoutAllowedForLeg(const MergedFareMarket& mfm) const;
  bool useAnyBrandWhenMerging(const MergedFareMarket& mfm, IbfErrorMessage status) const;
  bool fareMarketHasValidFares(const FareMarket& fm, const BrandCode& brandCode) const;

  void getHighestTPMCarriers(std::map< uint16_t, std::set<CarrierCode> >& ret);

  void copyFareMarketInfo(FareMarket& fm, MergedFareMarket& mfm);
  void copyPaxTypeBucketInfo(PaxTypeBucket& fromCortege, PaxTypeBucket& toCortege);
  bool addPaxTypeFare(FareMarket& fromFM, FareMarket& toFM, MergedFareMarket& mfm);
  void copyPaxTypeFares(FareMarket& fromFM,
                        PaxTypeBucket& fromCortege,
                        PaxTypeBucket& toCortege,
                        skipper::CarrierBrandPairs& carriersBrands,
                        bool& hardPassExists,
                        const uint16_t paxTypeNum,
                        const BrandCode* brandCode);

  bool sameMarket(const FareMarket& fm1, const FareMarket& fm2);
  bool sameTravelSegs(const std::vector<TravelSeg*>& travelSegVect1,
                      const std::vector<TravelSeg*>& travelSegVect2);

  bool isFareMarketValid(const FareMarket& fm, std::set<CarrierCode>& highestTPMCarriers);
  bool isCxrFarePreferred(std::vector<FareMarket*>& allGovCxrFM);
  void getAllCxrFareTypes(MergedFareMarket& mfm, FareMarket& fm);

  void addCxrFareTypes(
      PaxTypeBucket& paxTypeCortege,
      std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>>& fareTypeMap,
      const GeoTravelType geoTravelType,
      std::set<CarrierCode>& govCxr);
  void addFareTypeBitSet(CarrierCode valCxr,
                         std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>& fareTypeBitSetByVC,
                         const MergedFareMarket::CxrFareTypeBitSet& bitSet);

  void sortFareMarket(MergedFareMarket& mfm, FareMarket& fm);

  void reArrange(std::vector<PaxTypeFare*>& pxTypeFareVect);
  void insertYYFare(std::list<PaxTypeFare*>& pxTypeFareList, PaxTypeFare* paxTypeFare);

  void setTag2FareIndicator(MergedFareMarket& mfm);
  void setTag2FareIndicatorForDummyFM(std::vector<MergedFareMarket*>& mergedFareMarketVect);

  static constexpr char CXR_FARE_PREF_IND = 'C';

  PricingTrx& _trx;
  Itin& _itin;
  bool _searchAlwaysLowToHigh;
  const CarrierCode* _carrier;
  uint16_t _brandIndex;
  BrandCode* _brandCode;
  flexFares::GroupId _groupId;
  Diag993Collector* _diagPtr;
  bool _diag993InCbsMode;

public:
  FareMarketMerger(const FareMarketMerger& rhs);
  FareMarketMerger& operator=(const FareMarketMerger& rhs);
};
}

