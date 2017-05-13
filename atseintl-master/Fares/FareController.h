//-------------------------------------------------------------------
//
//  File:        FareController.h
//  Created:     Dec 4, 2003
//  Authors:     Abu Islam, Mark Kasprowicz
//
//  Description: Base class for all Fare Controllers
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
//-------------------------------------------------------------------

#pragma once

#include "Common/PaxTypeUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DBAccess/Record2Types.h"
#include "Fares/Record1Resolver.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Rules/DayTimeApplication.h"
#include "Rules/Eligibility.h"
#include "Rules/FlightApplication.h"
#include "Rules/FootNoteRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleItem.h"
#include "Rules/SalesRestrictionRule.h"
#include "Rules/SeasonalApplication.h"
#include "Rules/TravelRestrictions.h"

#include <boost/logic/tribool.hpp>

#include <map>
#include <memory>

namespace tse
{
class PricingTrx;
class PaxType;
class FareMarket;
class FareInfo;
class ConstructedFareInfo;
class PaxTypeFare;
class FareClassAppInfo;
class Itin;

namespace
{
struct FareLessByFareInfo;
}

class FareController : public Record1Resolver
{
  friend class FareControllerTest;

protected:
  static constexpr Indicator APPL_DIR_WITHIN = 'W';
  static constexpr Indicator APPL_DIR_FROM = 'F';
  static constexpr Indicator APPL_DIR_BETWEEN = 'B';
  static constexpr Indicator APPL_DIR_ANY = ' ';

  static constexpr Indicator MULTILATERAL = 'M';
  static constexpr Indicator INDUSTRY = 'Y';

   enum TariffInhibits
  {
    NOT_INHIBIT = 'N',
    INHIBIT_PRICING_ONLY = 'D',
    INHIBIT_ALL = 'F'
  };

  Itin& _itin;

  Money _nuc;
  CurrencyConversionFacade _ccFacade;

  RuleControllerWithChancelor<FareMarketRuleController> _prevalidationRuleController;
  RuleControllerWithChancelor<FareMarketRuleController> _fareGroupSRValidationRuleController;
  std::unique_ptr<FareMarketRuleController> _cat23PrevalidationRuleController = nullptr;
  std::unique_ptr<FootNoteRuleController> _footNotePrevalidationRuleController = nullptr;

  DayTimeApplication _dayTimeApplication;
  SeasonalApplication _seasonalApplication;
  Eligibility _eligibilityApplication;
  TravelRestrictionsObserverWrapper _travelApplication;
  SalesRestrictionRuleWrapper _salesRestrictionRule;
  FlightApplication _flightApplication;

  bool _checkItinCalculationCurrency = false;
  bool _matchedNonJCB;
  // the last fare to be processed by resolveTariffCrossRef(). If
  // a future call to resolveTariffCrossRef uses a fare with the same
  // key, we can re-use the TariffCrossRefInfo instead of having
  // to hit the cache again.
  mutable const Fare* _lastTariffCrossRefFare = nullptr;

  RuleItem::ProcessingPhase _phase = RuleItem::ProcessingPhase::UnKnown;

  bool isFdTrx() const { return (_fdTrx != nullptr); }

  virtual bool isInternationalFare(const Fare& fare) const;
  bool resolveTariffCrossRef(Fare& fare) const;

  virtual bool resolveTariffInhibits(const Fare& fare) const;

  virtual bool
  resolveFareTypeMatrix(PaxTypeFare& paxTypeFare, const FareClassAppInfo& fcaInfo) const;

  bool matchGlobalDirectionality(const Fare& fare, const FareMarket& fareMarket) const;

  bool checkAge(const PaxTypeFare& ptFare, const PaxType& paxType) const;

  virtual bool createPaxTypeFares(Fare* fare);
  void createPaxTypeFareESV(const Fare* fare, PaxTypeBucket* paxTypeCortage);
  virtual bool validateAndAddPaxTypeFares(const std::vector<PaxTypeFare*>& ptFares);
  virtual void prevalidatePaxTypeFare(PricingTrx& trx, Itin& itin, PaxTypeFare& ptf);

  void findFares(std::vector<Fare*>& fares);

  bool findFaresESV(std::vector<Fare*>& fares);

  bool addFaresToVectorESV(const std::vector<const FareInfo*>& fareInfoVec,
                           const LocCode& origin,
                           std::vector<Fare*>& fares);

  virtual void findFares(const CarrierCode& carrier, std::vector<Fare*>& fares);

  void findFares(const CarrierCode& carrier,
                 const LocCode& origin,
                 const LocCode& destination,
                 std::vector<Fare*>& fares);

  virtual const std::vector<const FareInfo*>&
  findCityPairFares(const LocCode& origin, const LocCode& destination, const CarrierCode& carrier);

  Fare* createFare(const FareInfo* fi,
                   const LocCode& origin,
                   const Fare::FareState initialStatus,
                   const ConstructedFareInfo* cfi = nullptr);
  bool sfaDiagRequested(PricingTrx& trx, std::vector<FareClassCode>& fareClasses);

  bool addFareToPaxTypeBucket(PaxTypeFare& ptFare);
  bool validateFareGroup(PaxTypeFare& ptFare);
  bool validateFareGroupCorpID(PaxTypeFare& ptFare,
                               std::vector<PaxType*>::iterator& atPaxTypeIT,
                               std::vector<PaxType*>& atPaxTypeVec);
  bool validateFareGroupPCC(PaxTypeFare& ptFare);

// Validating Carrier - this method will be obsolete after Cat35 change impl
  virtual bool validateQualifiers(PaxTypeFare& ptFare,
                                  std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                  const VendorCode& vendor,
                                  const CategoryRuleInfo* ruleInfo = nullptr,
                                  DiagCollector* diagCollector = nullptr,
                                  bool* const softPassFlag = nullptr);
  virtual bool validateQualifiers(PaxTypeFare& ptFare,
                                  std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                  const VendorCode& vendor,
                                  const CarrierCode& cxr,
                                  const CategoryRuleInfo* ruleInfo = nullptr,
                                  DiagCollector* diagCollector = nullptr,
                                  bool* const softPassFlag = nullptr);
  virtual bool validateQualifiers(PaxTypeFare& ptFare,
                                  std::vector<CategoryRuleItemInfo>& seqQualifiers,
                                  const VendorCode& vendor,
                                  std::vector<CarrierCode>& inputValCxrList,
                                  std::vector<CarrierCode>& passedValCxrList,
                                  const CategoryRuleInfo* ruleInfo = nullptr,
                                  DiagCollector* diagCollector = nullptr,
                                  bool* const softPassFlag = nullptr);
  virtual Record3ReturnTypes validateQualifier(PaxTypeFare& ptFare,
                                               const CategoryRuleItemInfo& ruleItemInfo,
                                               const VendorCode& vendor,
                                               const CarrierCode& cxr,
                                               const CategoryRuleInfo* ruleInfo = nullptr);
// Validating Carrier - this method will be obsolete after Cat35 change impl
  virtual Record3ReturnTypes validateQualifier(PaxTypeFare& ptFare,
                                               const CategoryRuleItemInfo& ruleItemInfo,
                                               const VendorCode& vendor,
                                               const CategoryRuleInfo* ruleInfo = nullptr);

  Record3ReturnTypes validateEligibilityQualifier(PaxTypeFare& ptFare,
                                                  uint32_t itemcat,
                                                  const CategoryRuleInfo* ruleInfo,
                                                  const EligibilityInfo* eligibilityInfo,
                                                  Eligibility& eligibilityApp);

  void printEligibilityInfo(DiagCollector& diag,
                            const PaxTypeFare& ptFare,
                            const EligibilityInfo& eligibilityInfo,
                            int categoryNumber) const;

  Record3ReturnTypes checkAccountCode(PaxTypeFare& ptFare,
                                      const EligibilityInfo& eligibilityInfo,
                                      Eligibility& eligibilityApp,
                                      int categoryNumber) const;

  Record3ReturnTypes setIncompleteR3Rule(PaxTypeFare& ptFare, uint32_t itemcat) const;

  Record3ReturnTypes validateDayTimeQualifier(PaxTypeFare& ptFare,
                                              uint32_t itemcat,
                                              const CategoryRuleInfo* ruleInfo,
                                              const DayTimeAppInfo* dayTimeInfo,
                                              DayTimeApplication& dayTimeApp);

  Record3ReturnTypes validateSeasonalQualifier(PaxTypeFare& ptFare,
                                               const CategoryRuleItemInfo& ruleItem,
                                               const CategoryRuleInfo* ruleInfo,
                                               const SeasonalAppl* seasonalInfo,
                                               SeasonalApplication& seasonalApp);

  void setQualifyFltAppRuleData(PaxTypeFare& ptFare, const CategoryRuleInfo* ruleInfo) const;

  Record3ReturnTypes validateFlightApplicationQualifier(PaxTypeFare& ptFare,
                                                        uint32_t itemcat,
                                                        const VendorCode& vendor,
                                                        const CategoryRuleInfo* ruleInfo,
                                                        const FlightAppRule* flightInfo,
                                                        FlightApplication& flightApp);

  Record3ReturnTypes
  validateTravelRestrictionsQualifier(PaxTypeFare& ptFare,
                                      uint32_t itemcat,
                                      const CategoryRuleInfo* ruleInfo,
                                      const TravelRestriction* travelInfo,
                                      TravelRestrictionsObserverWrapper& travelApp);

  Record3ReturnTypes validateSaleRestrictionsQualifier(PaxTypeFare& ptFare,
                                                       const CategoryRuleItemInfo& ruleItem,
                                                       const CategoryRuleInfo* ruleInfo,
                                                       const SalesRestriction* salesInfo,
                                                       SalesRestrictionRuleWrapper& salesApp,
                                                       const CarrierCode& cxr);
// Validating Carrier - this method will be obsolete after Cat35 change impl
  Record3ReturnTypes validateSaleRestrictionsQualifier(PaxTypeFare& ptFare,
                                                       const CategoryRuleItemInfo& ruleItem,
                                                       const CategoryRuleInfo* ruleInfo,
                                                       const SalesRestriction* salesInfo,
                                                       SalesRestrictionRuleWrapper& salesApp);

  virtual void processRuleItemInfoSet(CategoryRuleItemInfoSet& ruleItemInfoSet,
                                      std::vector<CategoryRuleItemInfo>& segQual,
                                      std::vector<CategoryRuleItemInfo>::iterator& o,
                                      std::vector<CategoryRuleItemInfo>::iterator& p);

  void addMatchedFareToPaxTypeBucket(PaxTypeFare& paxTypeFare);
  PaxTypeFare* clonePTFare(const PaxTypeFare& originalPTFare, const PaxType& newPaxType);
  void findCandidatePaxTypes(std::vector<PaxType*>& candidatePaxTypes, const bool& careAboutAge);

  void setOriginalFareAmountForRW(const FareInfo* fareInfo);

  bool isCFDInResponse(const std::vector<PaxTypeFare*>& ptFares,
                       const ConstructedFareInfo& cfi) const;

  void eliminateDuplicates(std::vector<Fare*>& cxrConstructedFares,
                           std::vector<Fare*>& cxrPublishedFares,
                           std::vector<Fare*>& resultFares);

  bool createAllPTFs(std::vector<Fare*>& fares);

  bool createAllPaxTypeFaresESV(std::vector<Fare*>& fares);

  bool isCmdPricedFM();

  bool prevalidateCat35If1(PaxTypeFare& ptFare,
                           VendorCode& vendor,
                           CarrierCode& cxr,
                           TariffNumber& tariff,
                           RuleNumber& ruleNumber);

  class PaxTypeFinder : public std::unary_function<PaxType*, bool>
  {
  public:
    PaxTypeFinder(const PaxTypeFare& paxTypeFare, bool xoFares, bool fbcSelected)
      : _matchesAll(paxTypeFare.isWebFare() &&
                    (paxTypeFare.fcasPaxType() == ADULT || paxTypeFare.fcasPaxType() == NEG)),
        _paxType(paxTypeFare.fcasPaxType()),
        _xoFares(xoFares),
        _fbcSelected(fbcSelected),
        _discounted(paxTypeFare.isDiscounted()),
        _isChild(fbcSelected ? PaxTypeUtil::isChild(paxTypeFare.fcasPaxType(), paxTypeFare.vendor())
                             : false),
        _isInfant(fbcSelected
                      ? PaxTypeUtil::isInfant(paxTypeFare.fcasPaxType(), paxTypeFare.vendor())
                      : false)
    {
    }

    bool operator()(PaxType* paxType) const
    {

      if ((_matchesAll || (_paxType.empty() && paxType->paxType() == ADULT)) && !_xoFares)
        return true;

      if (LIKELY(!_fbcSelected || _discounted))
      {
        if (_paxType != paxType->paxType())
        {
          return false;
        }
      }
      else
      {
        // Command Pricing
        // We only care that ADT can not use CNN fare,
        // CNN can not use INF fare
        if (_isChild && !paxType->paxTypeInfo()->isChild())
        {
          return false;
        }

        if (_isInfant && !paxType->paxTypeInfo()->isInfant())
        {
          return false;
        }
      }

      return true;
    }

  private:
    const bool _matchesAll;
    const PaxTypeCode& _paxType;
    const bool _xoFares;
    const bool _fbcSelected;
    const bool _discounted;
    const bool _isChild;
    const bool _isInfant;
  };
  template <typename T>
  class IsVendor : public std::unary_function<const T*, bool>
  {
    const VendorCode& _vendor;

  public:
    IsVendor(const VendorCode& vendor) : _vendor(vendor) {}
    bool operator()(const T* p) const { return p->vendor() == _vendor; }
  };

  template <typename T>
  class IsCarrier : public std::unary_function<const T*, bool>
  {
    const CarrierCode& _carrier;

  public:
    IsCarrier(const CarrierCode& carrier) : _carrier(carrier) {}
    bool operator()(T* p) const { return p->carrier() == _carrier; }
  };

  template <typename T>
  class IsFilteredOut : public std::unary_function<const T*, bool>
  {
    const CarrierCode& _excludeCxr;
    const VendorCode& _includeVendor;

  public:
    IsFilteredOut(const CarrierCode& carrier, const VendorCode& vendor)
      : _excludeCxr(carrier), _includeVendor(vendor)
    {
    }
    bool operator()(T* p) const
    {
      return (p->carrier() == _excludeCxr || p->vendor() != _includeVendor);
    }
  };

  VendorCode getVendorForCat31() const;

  template <typename T>
  const std::vector<T*>&
  filterByVendorForCat31(const std::vector<T*>& all, std::vector<T*>& filtered) const
  {
    const VendorCode vendor = getVendorForCat31();

    if (LIKELY(vendor == Vendor::EMPTY))
      return all;

    std::remove_copy_if(
        all.begin(), all.end(), std::back_inserter(filtered), std::not1(IsVendor<T>(vendor)));
    return filtered;
  }

  const std::vector<const FareInfo*>&
  filterByVendorForCat31AndDFF(const std::vector<const FareInfo*>& all,
                               std::vector<const FareInfo*>& filtered) const;

  template <typename T>
  const std::vector<T*>&
  filterByVendorCxrForCat31(const std::vector<T*>& all, std::vector<T*>& filtered) const
  {
    const VendorCode vendor = getVendorForCat31();
    const bool disableYY = TrxUtil::isDisableYYForExcItin(_trx);
    if (UNLIKELY(disableYY &&
        std::find_if(all.begin(), all.end(), IsCarrier<T>(INDUSTRY_CARRIER)) != all.end()))
    {
      if (vendor == Vendor::EMPTY)
        std::remove_copy_if(
            all.begin(), all.end(), std::back_inserter(filtered), IsCarrier<T>(INDUSTRY_CARRIER));
      else
        std::remove_copy_if(all.begin(),
                            all.end(),
                            std::back_inserter(filtered),
                            IsFilteredOut<T>(INDUSTRY_CARRIER, vendor));
    }
    else if (LIKELY(vendor == Vendor::EMPTY))
    {
      return all;
    }
    else
    {
      std::remove_copy_if(
          all.begin(), all.end(), back_inserter(filtered), std::not1(IsVendor<T>(vendor)));
    }

    return filtered;
  }

public:
  FareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  virtual ~FareController() = default;

  bool sortPaxTypeFares();
  Record1Resolver::CalcMoney& calcMoney() { return _calcMoney; }

  virtual char matchCurrency(Money& target,
                             const bool isIntl,
                             const Money& src1,
                             const Money& src2 = Money(0, NUC));

  void roundCurrency(MoneyAmount& amt, const CurrencyCode& cur, const bool isIntl);

  virtual void setAllowedVendors(const VendorCode& baseVend);
  bool isVendorAllowed(const VendorCode& candidateVend) const;

  RuleItem::ProcessingPhase phase() { return _phase; }
  void setPhase(RuleItem::ProcessingPhase phase) { _phase = phase; }

protected:
  std::vector<const VendorCode*> _allowedVend;
  boost::tribool _matchFaresDirectionality; // use tribool for lazy-initialization

  class GlobalDirectionStorage
  {
  public:
    explicit GlobalDirectionStorage(PaxTypeFare& ptFare)
      : _fareMarket(*ptFare.fareMarket()), _original(_fareMarket.getGlobalDirection())
    {
      _fareMarket.setGlobalDirection(ptFare.globalDirection());
    }

    ~GlobalDirectionStorage() { _fareMarket.setGlobalDirection(_original); }

  private:
    FareMarket& _fareMarket;
    GlobalDirection _original;
  };

  void initMatchFaresDirectionality();
  bool isCat15Qualified(std::vector<CategoryRuleItemInfo>& seqQualifiers);

private:
  std::once_flag _footNotePrevalidationEnabledSet;
};
} // namespace tse
