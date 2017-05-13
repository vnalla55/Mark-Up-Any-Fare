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
#pragma once

#include "Common/DateTime.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Record2Types.h"

namespace tse
{
class PricingTrx;
class Itin;
class FareMarket;
class PaxTypeFare;
class Fare;
class FareInfo;
class ConstructedFareInfo;
class FareClassAppInfo;
class FareClassAppSegInfo;
class TariffCrossRefInfo;
class FareByRuleItemInfo;
class FareByRuleProcessingInfo;
class FareByRuleCtrlInfo;
class FBRPaxTypeFareRuleData;
class BaseFareRule;

class FareByRuleFareCreator
{
  friend class FareByRuleFareCreatorTest;
  // overrides to external services
  virtual bool matchNationCurrency(const NationCode& nation, const CurrencyCode& currency) const;
  virtual const std::vector<const BaseFareRule*>&
  getBaseFareRule(const VendorCode& vendor, int itemNo) const;
  // end of overides

public:
  FareByRuleFareCreator(const PricingTrx& trx, const Itin& itin, const FareMarket& fareMarket);

  virtual ~FareByRuleFareCreator() = default;

  void initCreationData(const FareByRuleProcessingInfo* fbrProcessingInfo,
                        const std::vector<CategoryRuleItemInfo>* segQual,
                        const CategoryRuleItemInfo* catRuleItemInfo,
                        const CategoryRuleItemInfoSet* ruleItemInfoSet,
                        bool isLocationSwapped);

  FBRPaxTypeFareRuleData* createFBRPaxTypeFareRuleData(
      PaxTypeFare& ptf,
      const std::map<PaxTypeFare*, std::set<BookingCode> >& baseFareInfoBkcAvailMap,
      bool isMinMaxFare,
      PaxTypeFare* baseFare = nullptr) const;

  void setBkgCodes(FBRPaxTypeFareRuleData* ruleData, PaxTypeFare* baseFare) const;

  FareClassAppInfo* createFareClassAppInfo(const DateTime& effDate, const DateTime& expDate) const;
  FareClassAppInfo* cloneFareClassAppInfo(const DateTime& effDate,
                                          const DateTime& expDate,
                                          const FareClassAppInfo& baseFCAppInfo,
                                          const FareClassCode& fareClass) const;

  FareInfo* createFareInfo(const MoneyAmount& fareAmt,
                           const CurrencyCode& currency,
                           const CurrencyCode& fmOrigCurrency,
                           const CurrencyCode& fmDestCurrency,
                           const DateTime& effDate,
                           const DateTime& expDate) const;

  FareInfo*
  createFareInfo(const DateTime& effDate, const DateTime& expDate, PaxTypeFare* baseFare) const;

  Fare* createFare(bool isReversed,
                   const FareInfo* fareInfo,
                   const TariffCrossRefInfo* tariffCrossRefInfo,
                   const ConstructedFareInfo* constructedFareInfo = nullptr) const;

  FareClassAppSegInfo* createFareClassAppSegInfo() const;
  FareClassAppSegInfo* cloneFareClassAppSegInfo(const FareClassAppSegInfo& original) const;

  TariffCrossRefInfo* createTariffCrossRefInfo(const DateTime& effDate,
                                               const DateTime& expDate,
                                               const PaxTypeFare* baseFare = nullptr) const;

private:
  const Directionality getDirectionality(const CurrencyCode& currency, bool reverseDirection) const;
  static FareClassCode
  updateFbrFareClass(const FareClassCode& fbrFareFc, const FareClassCode& fbrRuleFc);

  const PricingTrx& _trx;
  const Itin& _itin;
  const FareMarket& _fareMarket;

  const FareByRuleProcessingInfo* _fbrProcessingInfo = nullptr;
  const FareByRuleItemInfo* _fbrItemInfo = nullptr;
  const FareByRuleCtrlInfo* _fbrCtrlInfo = nullptr;
  const std::vector<CategoryRuleItemInfo>* _segQual = nullptr;
  const CategoryRuleItemInfo* _catRuleItemInfo = nullptr;
  const CategoryRuleItemInfoSet* _ruleItemInfoSet = nullptr;
  bool _isLocationSwapped = false;
};
}
