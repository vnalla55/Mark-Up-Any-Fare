//----------------------------------------------------------------
//
//  File:    SurchargesRule.h
//  Authors:
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
//-----------------------------------------------------------------
#pragma once

#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/FarePath.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleItem.h"
#include "Rules/RuleUtil.h"

namespace tse
{
class Diag312Collector;
class DiagManager;
class FareUsage;
class LocKey;
class NoPNRPricingTrx;
class PricingTrx;
class PricingUnit;
class RuleItemInfo;
class SurchargeData;
class SurchargesInfo;
class TravelSeg;

class SurchargesRule : public RuleApplicationBase
{
  friend class FakeSurchargesRule;
  friend class SurchargesRuleTest;

public:
  Record3ReturnTypes
  validate(PricingTrx& trx, const SurchargesInfo& surchInfo, const PaxTypeFare& fare);

  Record3ReturnTypes
  validate(PricingTrx&, FarePath&, const PricingUnit&, FareUsage&, const SurchargesInfo*);

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override
  {
    return FAIL;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override
  {
    return FAIL;
  }

  void initalizeCat25Ptr(const PaxTypeFare* cat25Fare)
  {
    if (cat25Fare)
      _cat25Fare = cat25Fare;
  }

  class SurchargeSeg
  {
  public:
    SurchargeSeg() = default;
    SurchargeSeg(TravelSeg* ts,
                 const LocCode* fcBrdCity,
                 const LocCode* fcOffCity,
                 bool singleSector,
                 const LocCode* loc = nullptr)
      : _ts(ts),
        _fcBrdCity(fcBrdCity),
        _fcOffCity(fcOffCity),
        _singleSector(singleSector),
        _loc(loc)
    {
    }

    TravelSeg* _ts = nullptr;
    const LocCode* _fcBrdCity = nullptr;
    const LocCode* _fcOffCity = nullptr;
    bool _singleSector = false;
    const LocCode* _loc = nullptr;
  };
  static constexpr Indicator SectorPortionSector = 'S';
  static constexpr Indicator SectorPortionPortion = 'P';

private:
  using travelSegVector = std::vector<TravelSeg*>;
  using travelSegConstIt = travelSegVector::const_iterator;

  bool isDiagEnabled(PricingTrx& trx, const PaxTypeFare& paxTypeFare, uint32_t itemNo);

  Record3ReturnTypes validateSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                                      PricingTrx& trx,
                                      FarePath*,
                                      const PricingUnit*,
                                      FareUsage*,
                                      PaxTypeFare&,
                                      const SurchargesInfo* surchInfo,
                                      RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                      Diag312Collector* diag);

  bool processSectionPortion(PricingTrx& trx,
                             FarePath*,
                             const PricingUnit*,
                             FareUsage*,
                             PaxTypeFare&,
                             const SurchargesInfo* surchInfo,
                             RuleUtil::TravelSegWrapperVector& applTravelSegment,
                             Diag312Collector* diag);

  bool populateApplTravelSegment(PricingTrx& trx,
                                 RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                 const std::vector<TravelSeg*> tsVec,
                                 Diag312Collector* diag) const;

  void commonTravelSeg(RuleUtil::TravelSegWrapperVector& tvlSegsFirst,
                       RuleUtil::TravelSegWrapperVector& tvlSegsSecond);

  void markTransferPoint(RuleUtil::TravelSegWrapperVector& tvlSegsBtwAnd,
                         RuleUtil::TravelSegWrapperVector& tvlSegsToFromVia,
                         PricingTrx& trx);

  bool matchT986(PricingTrx& trx,
                 const SurchargesInfo* surchInfo,
                 const VendorCode& vendor,
                 RuleUtil::TravelSegWrapperVector& applTravelSegment,
                 Diag312Collector* diag);

  bool matchRBD(const SurchargesInfo* surchInfo,
                RuleUtil::TravelSegWrapperVector& applTravelSegment,
                Diag312Collector* diag,
                PricingTrx& trx,
                FareUsage* fUsagep);

  Record3ReturnTypes getSurchargeSegments(std::vector<SurchargeSeg>& surchargeSegments,
                                          PricingTrx& trx,
                                          FarePath* farePath,
                                          const PricingUnit* pricingUnit,
                                          FareUsage* fare,
                                          PaxTypeFare& ptFare,
                                          const SurchargesInfo* surchInfo,
                                          Diag312Collector* diagPtr);

  inline const PaxType& getPaxType(const FarePath* farePath, const PaxTypeFare& ptf) const
  {
    return farePath ? *farePath->paxType() : *ptf.actualPaxType();
  }

  bool validatePaxTypeData(PricingTrx&,
                           const PaxType* aPax,
                           const VendorCode vendor,
                           const SurchargesInfo&,
                           bool dg,
                           Diag312Collector* diag);

  Record3ReturnTypes validateNonSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                                         PricingTrx&,
                                         FarePath*,
                                         FareUsage*,
                                         const PaxTypeFare& fare,
                                         const SurchargesInfo&,
                                         RuleUtil::TravelSegWrapperVector&,
                                         Diag312Collector* diag);

  Indicator getTravelPortion(const SurchargesInfo& surchInfo, const PricingTrx& trx) const;

  virtual bool
  checkDateRange(PricingTrx&, const SurchargesInfo&, TravelSeg&, bool, NoPNRPricingTrx* noPnrTrx = nullptr);

  virtual bool checkDOWandTime(PricingTrx&,                     // Remove TRX parameter from this function when removing this fallback flag
                               const SurchargesInfo&,
                               TravelSeg&,
                               bool,
                               bool firstTravelSegment,
                               NoPNRPricingTrx* noPnrTrx = nullptr);

  virtual SurchargeData* addSurcharge(PricingTrx&,
                                      const FarePath*,
                                      const PaxTypeFare& fare,
                                      FareUsage*,
                                      const SurchargesInfo&,
                                      SurchargeData*,
                                      TravelSeg&,
                                      const LocCode&,
                                      const LocCode&,
                                      bool singleSector = true);

  bool isDataCorrect(const SurchargesInfo&);

  bool isSideTrip(std::vector<TravelSeg*>::const_iterator itB,
                  std::vector<TravelSeg*>::const_iterator itE,
                  std::vector<TravelSeg*>::const_iterator itBegin,
                  std::vector<TravelSeg*>::const_iterator itLast,
                  bool checkOrigin,
                  std::vector<TravelSeg*>::const_iterator& itRet);

  bool isSideTripBtwAnd(std::vector<TravelSeg*>::const_iterator itB,
                        std::vector<TravelSeg*>::const_iterator itE,
                        std::vector<TravelSeg*>::const_iterator itBegin,
                        std::vector<TravelSeg*>::const_iterator itLast,
                        RuleUtil::TravelSegWrapperVector& secondQualifiedVc,
                        bool checkOrigin,
                        std::vector<TravelSeg*>::const_iterator& itRet);

  bool processSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                       PricingTrx& trx,
                       const FarePath* farePath,
                       FareUsage* fareUsage,
                       const PaxTypeFare& fare,
                       const SurchargesInfo& surchInfo,
                       RuleUtil::TravelSegWrapperVector& firstQualifiedVc,
                       RuleUtil::TravelSegWrapperVector& secondQualifiedVc,
                       Diag312Collector* diag);

  BookingCode getBookingCode(PricingTrx& trx, PaxTypeFare& fare, const FareUsage* fUsagep) const;

  bool checkDiscountedInfo(const PaxTypeFare& paxTypeFare,
                           VendorCode& vendor,
                           MoneyAmount& discPercent) const;

  bool matchSegmentGoverningCarrier(const CarrierCode& carrier, const TravelSeg* travelSeg) const;

  bool hongKongException(PricingTrx& trx,
                         const std::vector<TravelSeg*>& travelSegs,
                         const SurchargesInfo& surchInfo) const;

  bool shouldMatchDOW(Indicator tvlPortion, bool firstSeg) const;

  std::vector<TravelSeg*>::const_iterator
  findGoverningCarrierSegment(const std::vector<TravelSeg*>& tvlSegs,
                              std::vector<TravelSeg*>::const_iterator,
                              const CarrierCode govCxr);

  travelSegConstIt getGoverningCarrierIfAvaliable(const SurchargesInfo& surchInfo,
                                                  const PaxTypeFare& fare,
                                                  travelSegConstIt it,
                                                  bool hkgExcept,
                                                  bool checkGeoAndBtw = false);

  bool
  isNegativeSurcharge(const SurchargesInfo* surchInfo, DiagManager& diag, bool isDiagActive) const;

  template <class T>
  bool validatePerTransfer(T iCurrent,
                           T iBeg,
                           T iLast,
                           bool matchedCurrentSeg,
                           bool& originMatch,
                           bool& destMatch,
                           PricingTrx& trx);

  inline PaxTypeFare& pickProperFare(const PaxTypeFare& fare)
  {
    if (_cat25Fare)
      return const_cast<PaxTypeFare&>(*_cat25Fare);

    return const_cast<PaxTypeFare&>(fare);
  }

  SurchargeData* constructSurcharge(PricingTrx& trx);

  void addSurchargeForPerTransfer(PricingTrx& trx,
                                  const FarePath* farePath,
                                  const PaxTypeFare& fare,
                                  FareUsage* fareUsage,
                                  const SurchargesInfo& surchInfo,
                                  travelSegConstIt currentSegIt,
                                  bool singleSector,
                                  bool origCheck,
                                  bool destCheck,
                                  Diag312Collector* diag);

  void addSurchargeForPerTransfer(PricingTrx& trx,
                                  std::vector<SurchargeSeg>& surchargeSegments,
                                  travelSegConstIt currentSegIt,
                                  bool singleSector,
                                  bool origCheck,
                                  bool destCheck);

  // data members
  TSICode _tsi = 0;
  LocKey _locKey1;
  LocKey _locKey2;
  TSICode _tsiBtw = 0;
  LocKey _locKeyBtw1;
  LocKey _locKeyBtw2;
  TSICode _tsiAnd = 0;
  LocKey _locKeyAnd1;
  LocKey _locKeyAnd2;

  std::vector<TravelSeg*> _matchedRbdSegs;
  std::vector<TravelSeg*> _matched986Segs;

  const PaxTypeFare* _cat25Fare = nullptr;
  bool _perTransMatchedSeg = false;

  LocCode _transferPointDest;
};

template <class T>
inline bool
SurchargesRule::validatePerTransfer(T iCurrent,
                                    T iBeg,
                                    T iLast,
                                    bool matchedCurrentSeg,
                                    bool& originMatch,
                                    bool& destMatch,
                                    PricingTrx& trx)
{
  if (!matchedCurrentSeg)
  {
    return false; // check next segment
  }
  else
  {
    if (originMatch)
    {
      // only origin match - first seg
      if (!destMatch && iCurrent == iBeg)
        return false;

      // origin and dest. match - first segment
      if (destMatch && iCurrent == iBeg)
        originMatch = false;

      // origin and dest. match - last segment
      if (destMatch && (iCurrent == iLast))
        destMatch = false;
    }
    else //  when TSI --> destination
    {
      // there is no transfer, check next segment
      if (iCurrent == iLast)
        return false;
    }
  }
  _perTransMatchedSeg = true;

  return (originMatch || destMatch);
}

} // namespace tse

