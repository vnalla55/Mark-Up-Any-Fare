//----------------------------------------------------------------------------
//  Copyright Sabre 2010
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
#include "Fares/FareByRuleFareCreator.h"

#include "Common/CurrencyUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Fares/FareByRuleController.h"
#include "Fares/FareByRuleProcessingInfo.h"

namespace tse
{
FareByRuleFareCreator::FareByRuleFareCreator(const PricingTrx& trx,
                                             const Itin& itin,
                                             const FareMarket& fareMarket)
  : _trx(trx), _itin(itin), _fareMarket(fareMarket)
{
}

// overrides to external services
bool
FareByRuleFareCreator::matchNationCurrency(const NationCode& nation, const CurrencyCode& currency)
    const
{
  return CurrencyUtil::matchNationCurrency(nation, currency, _itin.travelDate());
}

const std::vector<const BaseFareRule*>&
FareByRuleFareCreator::getBaseFareRule(const VendorCode& vendor, int itemNo) const
{
  return _trx.dataHandle().getBaseFareRule(vendor, itemNo, _itin.travelDate());
}
// end of overides

void
FareByRuleFareCreator::initCreationData(const FareByRuleProcessingInfo* fbrProcessingInfo,
                                        const std::vector<CategoryRuleItemInfo>* segQual,
                                        const CategoryRuleItemInfo* catRuleItemInfo,
                                        const CategoryRuleItemInfoSet* ruleItemInfoSet,
                                        bool isLocationSwapped)
{
  _fbrProcessingInfo = fbrProcessingInfo;
  _fbrItemInfo = fbrProcessingInfo->fbrItemInfo();
  _fbrCtrlInfo = fbrProcessingInfo->fbrCtrlInfo();
  _segQual = segQual;
  _catRuleItemInfo = catRuleItemInfo;
  _ruleItemInfoSet = ruleItemInfoSet;
  _isLocationSwapped = isLocationSwapped;
}

FBRPaxTypeFareRuleData*
FareByRuleFareCreator::createFBRPaxTypeFareRuleData(
    PaxTypeFare& ptf,
    const std::map<PaxTypeFare*, std::set<BookingCode> >& baseFareInfoBkcAvailMap,
    bool isMinMaxFare,
    PaxTypeFare* baseFare) const
{
  FBRPaxTypeFareRuleData* ruleData = ptf.getFbrRuleData(RuleConst::FARE_BY_RULE);

  if (LIKELY(ruleData == nullptr))
  {
    _trx.dataHandle().get(ruleData);
  }

  if (LIKELY(ruleData != nullptr))
  {
    ptf.setRuleData(RuleConst::FARE_BY_RULE, _trx.dataHandle(), ruleData);
    ruleData->categoryRuleInfo() = _fbrCtrlInfo; // Record 2 Cat 25
    ruleData->isLocationSwapped() = _isLocationSwapped; // Record 2 Cat 25 isLocationSwapped
    ruleData->categoryRuleItemInfoSet() = _ruleItemInfoSet; // Record 2 Cat 25 set
    ruleData->categoryRuleItemInfoVec() = _segQual; // Record 2 Cat 25 qualifiers
    ruleData->categoryRuleItemInfo() = _catRuleItemInfo; // Record 2 Cat 25 segment
    ruleData->ruleItemInfo() = _fbrItemInfo; // Record 3 Cat 25
    ruleData->fbrApp() = _fbrProcessingInfo->fbrApp(); // Record 8
    ruleData->isR8LocationSwapped() =
        _fbrProcessingInfo->isR8LocationSwapped(); // Record 8 Location Swapped
    ruleData->baseFare() = nullptr; // No base fare for Specified

    if (baseFare)
    {
      ruleData->baseFare() = baseFare; // Base fare
      ruleData->isMinMaxFare() =
          isMinMaxFare; // true if fare created from a min/max limit (F/IND H/L)
      ruleData->isBaseFareAvailBkcMatched() = false;

      if (!baseFareInfoBkcAvailMap.empty())
      {
        std::map<PaxTypeFare*, std::set<BookingCode> >::const_iterator baseFareIter =
            baseFareInfoBkcAvailMap.find(baseFare);

        if (LIKELY(baseFareIter != baseFareInfoBkcAvailMap.end()))
        {
          ruleData->isBaseFareAvailBkcMatched() = true;
          ruleData->baseFareInfoBookingCodes() = baseFareIter->second;
        }
      }
      setBkgCodes(ruleData, baseFare);
    }

    ruleData->isSpanishResidence() = false;
    if (UNLIKELY(_fbrProcessingInfo->isResidenceFiledInR8() ||
        _fbrProcessingInfo->isResidenceFiledInR3Cat25()))
    {
      ruleData->isSpanishResidence() = true;
    }
  }

  return ruleData;
}

void
FareByRuleFareCreator::setBkgCodes(FBRPaxTypeFareRuleData* ruleData, PaxTypeFare* baseFare) const
{
  if (UNLIKELY(baseFare->isFareClassAppSegMissing()))
  {
    baseFare->cat25BasePaxFare() = true;

    const std::vector<const BaseFareRule*>& baseFareRules =
        getBaseFareRule(_fbrCtrlInfo->vendorCode(), _fbrItemInfo->baseTableItemNo()); // table 989

    std::vector<const BaseFareRule*>::const_iterator curBaseFareRule = baseFareRules.begin();

    const FareClassAppInfo* fcaInfo = baseFare->fareClassAppInfo();

    // iterate through this vector of FareClassAppSegInfo records
    // and find the one that matches passanger type
    FareClassAppSegInfoList::const_iterator fcas =
        FareByRuleController::matchFareClassAppInfoToPaxType(*fcaInfo, **curBaseFareRule);
    if (fcas == fcaInfo->_segs.end())
    {
      fcas = fcaInfo->_segs.begin();
    }

    ruleData->baseFareBookingCodeTblItemNo() = (*fcas)->_bookingCodeTblItemNo;

    std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
    for (int i = 0; i < FareClassAppSegInfo::BK_CODE_SIZE; ++i)
    {
      if (!(*fcas)->_bookingCode[i].empty())
      {
        bkgCodes.push_back((*fcas)->_bookingCode[i]);
      }
    }

    ruleData->setBaseFarePrimeBookingCode(bkgCodes);
  }
  else
  {
    ruleData->baseFareBookingCodeTblItemNo() = baseFare->fcasBookingCodeTblItemNo();

    std::vector<BookingCode> bkgCodes; //  Empty Vector of booking codes
    baseFare->getPrimeBookingCode(bkgCodes);
    ruleData->setBaseFarePrimeBookingCode(bkgCodes);
  }
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleFareCreator::createFareClassAppInfo
//
// Description:  This method dynamically creates a
//               FareClassAppInfo object and initializes it.
//
// @return FareClassAppInfo* - a pointer to the new object
//                             0, otherwise
// ----------------------------------------------------------------------------
FareClassAppInfo*
FareByRuleFareCreator::createFareClassAppInfo(const DateTime& effDate, const DateTime& expDate)
    const
{
  const FareByRuleApp& fbrApp = *_fbrProcessingInfo->fbrApp();

  FareClassAppInfo* fcAppInfo = nullptr;
  _trx.dataHandle().get(fcAppInfo);

  if (LIKELY(fcAppInfo != nullptr))
  {
    fcAppInfo->_vendor = fbrApp.vendor();
    fcAppInfo->_carrier = fbrApp.carrier();
    fcAppInfo->_ruleTariff = fbrApp.ruleTariff();
    fcAppInfo->_ruleNumber = fbrApp.ruleNo();

    fcAppInfo->_fareClass = _fbrItemInfo->resultFareClass1();

    fcAppInfo->_effectiveDate = effDate;
    fcAppInfo->_expirationDate = expDate;

    fcAppInfo->_location1Type = LOCTYPE_CITY;
    fcAppInfo->_location1 = _fareMarket.origin()->loc();
    fcAppInfo->_location2Type = LOCTYPE_CITY;
    fcAppInfo->_location2 = _fareMarket.destination()->loc();

    fcAppInfo->_owrt = _fbrItemInfo->resultowrt();
    fcAppInfo->_routingNumber = _fbrItemInfo->resultRouting();

    fcAppInfo->_fareType = _fbrItemInfo->resultFareType1();
    fcAppInfo->_seasonType = _fbrItemInfo->resultseasonType();
    fcAppInfo->_dowType = _fbrItemInfo->resultdowType();
    fcAppInfo->_pricingCatType = _fbrItemInfo->resultpricingcatType();
    fcAppInfo->_displayCatType = _fbrItemInfo->resultDisplaycatType();

    fcAppInfo->_segCount = 1;
  }

  return fcAppInfo;
}

FareClassAppInfo*
FareByRuleFareCreator::cloneFareClassAppInfo(const DateTime& effDate,
                                             const DateTime& expDate,
                                             const FareClassAppInfo& baseFCAppInfo,
                                             const FareClassCode& fareClass) const
{
  const FareByRuleApp& fbrApp = *_fbrProcessingInfo->fbrApp();

  FareClassAppInfo* fcAppInfo = nullptr;
  _trx.dataHandle().get(fcAppInfo);

  fcAppInfo->_vendor = fbrApp.vendor();
  fcAppInfo->_carrier = baseFCAppInfo._carrier;

  fcAppInfo->_ruleTariff = fbrApp.ruleTariff();
  fcAppInfo->_ruleNumber = fbrApp.ruleNo();

  fcAppInfo->_footnote1 = baseFCAppInfo._footnote1;
  fcAppInfo->_footnote2 = baseFCAppInfo._footnote2;
  fcAppInfo->_fareClass = fareClass;

  fcAppInfo->_effectiveDate = effDate;
  fcAppInfo->_discDate = baseFCAppInfo._discDate;
  fcAppInfo->_expirationDate = expDate;

  fcAppInfo->_MCN = baseFCAppInfo._MCN;
  fcAppInfo->_textTBLItemNo = baseFCAppInfo._textTBLItemNo;

  fcAppInfo->_location1Type = baseFCAppInfo._location1Type;
  fcAppInfo->_location1 = baseFCAppInfo._location1;

  fcAppInfo->_location2Type = baseFCAppInfo._location2Type;
  fcAppInfo->_location2 = baseFCAppInfo._location2;

  if (_fbrItemInfo->resultowrt() != FareByRuleController::BLANK)
  {
    fcAppInfo->_owrt = _fbrItemInfo->resultowrt();
  }
  fcAppInfo->_routingAppl = baseFCAppInfo._routingAppl;

  if (!_fbrItemInfo->resultRouting().empty())
    fcAppInfo->_routingNumber = _fbrItemInfo->resultRouting();
  else
    fcAppInfo->_routingNumber = baseFCAppInfo._routingNumber;

  if (!_fbrItemInfo->resultFareType1().empty())
    fcAppInfo->_fareType = _fbrItemInfo->resultFareType1();
  else
    fcAppInfo->_fareType = baseFCAppInfo._fareType;

  if (_fbrItemInfo->resultseasonType() != FareByRuleController::BLANK)
    fcAppInfo->_seasonType = _fbrItemInfo->resultseasonType();
  else
    fcAppInfo->_seasonType = baseFCAppInfo._seasonType;

  if (_fbrItemInfo->resultdowType() != FareByRuleController::BLANK)
    fcAppInfo->_dowType = _fbrItemInfo->resultdowType();
  else
    fcAppInfo->_dowType = baseFCAppInfo._dowType;

  if (_fbrItemInfo->resultpricingcatType() != FareByRuleController::BLANK)
    fcAppInfo->_pricingCatType = _fbrItemInfo->resultpricingcatType();
  else
    fcAppInfo->_pricingCatType = baseFCAppInfo._pricingCatType;

  if (_fbrItemInfo->resultDisplaycatType() != FareByRuleController::BLANK)
    fcAppInfo->_displayCatType = _fbrItemInfo->resultDisplaycatType();
  else
    fcAppInfo->_displayCatType = baseFCAppInfo._displayCatType;

  fcAppInfo->_seqNo = baseFCAppInfo._seqNo;
  fcAppInfo->_unavailTag = baseFCAppInfo._unavailTag;

  fcAppInfo->_segCount = 1;

  return fcAppInfo;
}

FareInfo*
FareByRuleFareCreator::createFareInfo(const MoneyAmount& fareAmt,
                                      const CurrencyCode& currency,
                                      const CurrencyCode& fmOrigCurrency,
                                      const CurrencyCode& fmDestCurrency,
                                      const DateTime& effDate,
                                      const DateTime& expDate) const
{
  const FareByRuleApp& fbrApp = *_fbrProcessingInfo->fbrApp();

  FareInfo* fareInfo = nullptr;
  _trx.dataHandle().get(fareInfo);

  if (UNLIKELY(fareInfo == nullptr))
  {
    return nullptr;
  }

  fareInfo->vendor() = fbrApp.vendor();
  fareInfo->carrier() = fbrApp.carrier();
  fareInfo->vendorFWS() = fbrApp.vendorFWS();

  bool reverseDirection = false;

  if (_fareMarket.boardMultiCity() < _fareMarket.offMultiCity())
  {
    fareInfo->market1() = _fareMarket.boardMultiCity();
    fareInfo->market2() = _fareMarket.offMultiCity();
  }
  else
  {
    fareInfo->market1() = _fareMarket.offMultiCity();
    fareInfo->market2() = _fareMarket.boardMultiCity();
    reverseDirection = true;
  }

  fareInfo->fareClass() = _fbrItemInfo->resultFareClass1();
  fareInfo->fareTariff() = fbrApp.ruleTariff();

  fareInfo->effDate() = effDate;
  fareInfo->expireDate() = expDate;
  fareInfo->discDate() = expDate;
  fareInfo->createDate() = _fbrItemInfo->createDate();

  fareInfo->owrt() = _fbrItemInfo->resultowrt();

  fareInfo->ruleNumber() = fbrApp.ruleNo();
  fareInfo->routingNumber() = _fbrItemInfo->resultRouting();

  if (_fbrItemInfo->fareInd() == FareByRuleItemInfo::SPECIFIED)
  {
    fareInfo->fareAmount() = (fareInfo->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
                              !_trx.getOptions()->isRtw())
                                 ? fareAmt / 2
                                 : fareAmt;
    fareInfo->directionality() = getDirectionality(currency, reverseDirection);
  }
  else
  {
    if (fareInfo->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      fareInfo->fareAmount() = !_trx.getOptions()->isRtw()? fareAmt / 2 : fareAmt;
      fareInfo->directionality() = BOTH;
    }
    else if (fareInfo->owrt() == ONE_WAY_MAY_BE_DOUBLED)
    {
      fareInfo->fareAmount() = fareAmt;
      fareInfo->directionality() = BOTH;
    }
    else
    {
      fareInfo->fareAmount() = fareAmt;
      if (_fareMarket.direction() == FMDirection::OUTBOUND)
      {
        fareInfo->directionality() =
            (fareInfo->market1() == _fareMarket.boardMultiCity() ? FROM : TO);
      }

      else if (_fareMarket.direction() == FMDirection::INBOUND)
      {
        fareInfo->directionality() =
            (fareInfo->market1() == _fareMarket.boardMultiCity() ? TO : FROM);
      }

      else
      {
        if (currency == fmOrigCurrency)
          fareInfo->directionality() =
              (fareInfo->market1() == _fareMarket.boardMultiCity() ? FROM : TO);
        else if (currency == fmDestCurrency)
          fareInfo->directionality() =
              (fareInfo->market1() == _fareMarket.boardMultiCity() ? TO : FROM);
        else
        {
          fareInfo->directionality() = BOTH;
        }
      }
    }
  }

  if (_fbrItemInfo->specifiedCur1() == currency)
  {
    fareInfo->noDec() = _fbrItemInfo->specifiedNoDec1();
  }
  else
  {
    fareInfo->noDec() = _fbrItemInfo->specifiedNoDec2();
  }

  fareInfo->currency() = currency;

  fareInfo->globalDirection() = _fbrProcessingInfo->isUsCaRuleTariff()
                                    ? _fareMarket.getGlobalDirection()
                                    : _fbrItemInfo->resultglobalDir();

  return fareInfo;
}

FareInfo*
FareByRuleFareCreator::createFareInfo(const DateTime& effDate,
                                      const DateTime& expDate,
                                      PaxTypeFare* baseFare) const
{
  const FareByRuleApp& fbrApp = *_fbrProcessingInfo->fbrApp();
  FareInfo* fareInfo;

  if (baseFare->isConstructed())
  {
    // base fare is ATPCO or SITA constructed fare

    FareInfo* newATPCOFareInfo;

    SITAFareInfo* newSITAFareInfo;

    if (UNLIKELY(baseFare->vendor() == SITA_VENDOR_CODE))
    {
      _trx.dataHandle().get(newSITAFareInfo);
      fareInfo = newSITAFareInfo;
    }
    else
    {
      _trx.dataHandle().get(newATPCOFareInfo);
      fareInfo = newATPCOFareInfo;
    }

    baseFare->fare()->fareInfo()->clone(*fareInfo);

    fareInfo->market1() = baseFare->market1();
    fareInfo->market2() = baseFare->market2();

    if (!_fbrItemInfo->resultRouting().empty())
      fareInfo->routingNumber() = _fbrItemInfo->resultRouting();
    else
    {
      fareInfo->routingNumber() = CAT25_EMPTY_ROUTING;
      baseFare->cat25BasePaxFare() = true;
    }
  }
  else
  {
    // base fare is ATPCO or SITA published fare
    fareInfo = baseFare->fare()->fareInfo()->clone(_trx.dataHandle());

    if (!_fbrItemInfo->resultRouting().empty())
      fareInfo->routingNumber() = _fbrItemInfo->resultRouting();
  }

  fareInfo->vendor() = fbrApp.vendor();
  fareInfo->carrier() = fbrApp.carrier();
  fareInfo->vendorFWS() = fbrApp.vendorFWS();

  if (!_fbrItemInfo->resultFareClass1().empty())
  {
    fareInfo->fareClass() =
        updateFbrFareClass(fareInfo->fareClass(), _fbrItemInfo->resultFareClass1());
  }

  fareInfo->fareTariff() = fbrApp.ruleTariff();
  fareInfo->effDate() = effDate;
  fareInfo->expireDate() = expDate;

  if (_fbrItemInfo->resultowrt() != FareByRuleController::BLANK)
  {
    fareInfo->owrt() = _fbrItemInfo->resultowrt();
  }

  fareInfo->ruleNumber() = fbrApp.ruleNo();

  if (_fbrItemInfo->resultglobalDir() != GlobalDirection::XX &&
      _fbrItemInfo->resultglobalDir() != GlobalDirection::ZZ &&
      !_fbrProcessingInfo->isUsCaRuleTariff())
  {
    fareInfo->globalDirection() = _fbrItemInfo->resultglobalDir();
  }

  fareInfo->footNote1().empty();
  fareInfo->footNote2().empty();
  return fareInfo;
}

Fare*
FareByRuleFareCreator::createFare(bool isReversed,
                                  const FareInfo* fareInfo,
                                  const TariffCrossRefInfo* tariffCrossRefInfo,
                                  const ConstructedFareInfo* constructedFareInfo) const
{
  Fare* fare = nullptr;
  _trx.dataHandle().get(fare);

  fare->initialize(
      Fare::FS_FBRBaseFare, fareInfo, _fareMarket, tariffCrossRefInfo, constructedFareInfo);

  if (isReversed)
  {
    fare->status().set(Fare::FS_ReversedDirection);
  }

  return fare;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleFareCreator::createFareClassAppSegInfo
//
// Description:  This method dynamically creates a
//               FareClassAppSegInfo object and initializes it.
//
// @return FareClassAppSegInfo* - a pointer to the new object
//                             0, otherwise
// ----------------------------------------------------------------------------
FareClassAppSegInfo*
FareByRuleFareCreator::createFareClassAppSegInfo() const
{
  FareClassAppSegInfo* fcAppSegInfo = nullptr;
  _trx.dataHandle().get(fcAppSegInfo);

  if (LIKELY(fcAppSegInfo != nullptr))
  {
    fcAppSegInfo->_directionality = DIR_IND_NOT_DEFINED;

    fcAppSegInfo->_bookingCodeTblItemNo = _fbrItemInfo->bookingCodeTblItemNo();

    fcAppSegInfo->_tktCode = _fbrItemInfo->tktCode();
    fcAppSegInfo->_tktCodeModifier = _fbrItemInfo->tktCodeModifier();

    fcAppSegInfo->_tktDesignator = _fbrItemInfo->tktDesignator();
    fcAppSegInfo->_tktDesignatorModifier = _fbrItemInfo->tktDesignatorModifier();

    fcAppSegInfo->_paxType = _fbrItemInfo->paxType();

    fcAppSegInfo->_bookingCode[0] = _fbrItemInfo->bookingCode1();
    fcAppSegInfo->_bookingCode[1] = _fbrItemInfo->bookingCode2();
    fcAppSegInfo->_bookingCode[2] = _fbrItemInfo->bookingCode3();
    fcAppSegInfo->_bookingCode[3] = _fbrItemInfo->bookingCode4();
    fcAppSegInfo->_bookingCode[4] = _fbrItemInfo->bookingCode5();
    fcAppSegInfo->_bookingCode[5] = _fbrItemInfo->bookingCode6();
    fcAppSegInfo->_bookingCode[6] = _fbrItemInfo->bookingCode7();
    fcAppSegInfo->_bookingCode[7] = _fbrItemInfo->bookingCode8();
  }

  return fcAppSegInfo;
}

FareClassAppSegInfo*
FareByRuleFareCreator::cloneFareClassAppSegInfo(const FareClassAppSegInfo& original) const
{
  FareClassAppSegInfo* fcAppSegInfo = original.clone(_trx.dataHandle());

  fcAppSegInfo->_bookingCodeTblItemNo = _fbrItemInfo->bookingCodeTblItemNo();

  if (!_fbrItemInfo->tktCode().empty() || !_fbrItemInfo->resultFareClass1().empty())
  {
    fcAppSegInfo->_tktCode = _fbrItemInfo->tktCode();
  }
  if (UNLIKELY(_fbrItemInfo->tktCodeModifier() != FareByRuleController::BLANK))
  {
    fcAppSegInfo->_tktCodeModifier = _fbrItemInfo->tktCodeModifier();
  }
  if (!_fbrItemInfo->tktDesignator().empty())
  {
    fcAppSegInfo->_tktDesignator = _fbrItemInfo->tktDesignator();
  }
  if (UNLIKELY(_fbrItemInfo->tktDesignatorModifier() != FareByRuleController::BLANK))
  {
    fcAppSegInfo->_tktDesignatorModifier = _fbrItemInfo->tktDesignatorModifier();
  }

  fcAppSegInfo->_paxType = _fbrItemInfo->paxType();

  if (!_fbrItemInfo->bookingCode1().empty())
  {
    fcAppSegInfo->_bookingCode[0] = _fbrItemInfo->bookingCode1();
    fcAppSegInfo->_bookingCode[1] = _fbrItemInfo->bookingCode2();
    fcAppSegInfo->_bookingCode[2] = _fbrItemInfo->bookingCode3();
    fcAppSegInfo->_bookingCode[3] = _fbrItemInfo->bookingCode4();
    fcAppSegInfo->_bookingCode[4] = _fbrItemInfo->bookingCode5();
    fcAppSegInfo->_bookingCode[5] = _fbrItemInfo->bookingCode6();
    fcAppSegInfo->_bookingCode[6] = _fbrItemInfo->bookingCode7();
    fcAppSegInfo->_bookingCode[7] = _fbrItemInfo->bookingCode8();
  }
  else
  {
    fcAppSegInfo->_bookingCode[0].clear();
    fcAppSegInfo->_bookingCode[1].clear();
    fcAppSegInfo->_bookingCode[2].clear();
    fcAppSegInfo->_bookingCode[3].clear();
    fcAppSegInfo->_bookingCode[4].clear();
    fcAppSegInfo->_bookingCode[5].clear();
    fcAppSegInfo->_bookingCode[6].clear();
    fcAppSegInfo->_bookingCode[7].clear();
  }

  return fcAppSegInfo;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleFareCreator::createTariffCrossRefInfo
//
// Description:  This method dynamically creates a
//               TariffCrossRefInfo object and initializes it.
//
// @return TariffCrossRefInfo* - a pointer to the new object
//                               0, otherwise
// ----------------------------------------------------------------------------
TariffCrossRefInfo*
FareByRuleFareCreator::createTariffCrossRefInfo(const DateTime& effDate,
                                                const DateTime& expDate,
                                                const PaxTypeFare* baseFare) const
{
  const FareByRuleApp& fbrApp = *_fbrProcessingInfo->fbrApp();

  TariffCrossRefInfo* tcrInfo = nullptr;
  _trx.dataHandle().get(tcrInfo);

  tcrInfo->vendor() = fbrApp.vendor();
  tcrInfo->carrier() = fbrApp.carrier();

  GeoTravelType geoTravelType = _fareMarket.geoTravelType();
  tcrInfo->crossRefType() = DOMESTIC;

  if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
  {
    tcrInfo->crossRefType() = INTERNATIONAL;
  }

  tcrInfo->globalDirection() = _fbrItemInfo->resultglobalDir();
  tcrInfo->fareTariff() = fbrApp.ruleTariff();
  tcrInfo->effDate() = effDate;
  tcrInfo->expireDate() = expDate;
  tcrInfo->discDate() = expDate;
  tcrInfo->tariffCat() = _fbrProcessingInfo->tcrInfo()->tariffCat();
  tcrInfo->ruleTariff() = fbrApp.ruleTariff();
  tcrInfo->routingTariff1() = _fbrItemInfo->resultRoutingTariff();
  tcrInfo->routingTariff2() = -1;

  if (baseFare)
  {
    if (!_fbrItemInfo->resultRouting().empty() && _fbrItemInfo->resultRoutingTariff() != 0)
    {
      tcrInfo->routingTariff1() = _fbrItemInfo->resultRoutingTariff();
      tcrInfo->routingTariff2() = -1;
    }
    else
    {
      tcrInfo->routingTariff1() = baseFare->tcrRoutingTariff1();
      tcrInfo->routingTariff2() = baseFare->tcrRoutingTariff2();
      tcrInfo->routingTariff1Code() = baseFare->tcrRoutingTariff1Code();
      tcrInfo->routingTariff2Code() = baseFare->tcrRoutingTariff2Code();
    }

    if (_fbrProcessingInfo->isUsCaRuleTariff())
    {
      tcrInfo->globalDirection() = baseFare->globalDirection();
    }
  }
  else
  {
    if (_fbrProcessingInfo->isUsCaRuleTariff())
    {
      tcrInfo->globalDirection() = _fareMarket.getGlobalDirection();
    }
  }

  return tcrInfo;
}

const Directionality
FareByRuleFareCreator::getDirectionality(const CurrencyCode& currency, bool reverseDirection) const
{
  if (_fareMarket.geoTravelType() == GeoTravelType::Transborder)
    return BOTH;

  bool matchOriginCurrency = matchNationCurrency(_fareMarket.origin()->nation(), currency);
  bool matchDestinationCurrency =
      matchNationCurrency(_fareMarket.destination()->nation(), currency);

  if (matchOriginCurrency && !matchDestinationCurrency)
    return reverseDirection ? TO : FROM;
  else if (!matchOriginCurrency && matchDestinationCurrency)
    return reverseDirection ? FROM : TO;
  return BOTH;
}

FareClassCode
FareByRuleFareCreator::updateFbrFareClass(const FareClassCode& baseFareFc,
                                          const FareClassCode& fbrRuleFc)
{
  FareClassCode result;
  if (!baseFareFc.empty() && !fbrRuleFc.empty() && fbrRuleFc[0] == FareByRuleController::ASTERISK)
  {
    result = fbrRuleFc;
    result[0] = baseFareFc[0];
  }
  else if (!fbrRuleFc.empty() && fbrRuleFc[0] == '-')
  {
    result = baseFareFc + fbrRuleFc.substr(1, fbrRuleFc.size() - 1);
    if (result.length() > 8)
    {
      result = result.substr(0, 8);
    }
  }
  else
  {
    size_t srchIndex(fbrRuleFc.find_last_of('-'));
    if (srchIndex != std::string::npos)
    {
      result = fbrRuleFc.substr(0, srchIndex) + baseFareFc.substr(1, baseFareFc.length() - 1);
    }
    else
    {
      result = fbrRuleFc;
    }
  }
  return result;
}
}
