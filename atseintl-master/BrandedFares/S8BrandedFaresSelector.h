//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.h
//  Created:     2013
//  Authors:
//
//  Description:
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

#include "BrandedFares/BrandedFareDiagnostics.h"
#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/BrandedFaresValidator.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class SvcFeesFareIdInfo;
class BrandProgram;
class BrandInfo;

struct BrandProgramComparator : std::binary_function<const BrandProgram*, const BrandProgram*, bool>
{
  bool operator()(const BrandProgram* brandProgram1, const BrandProgram* brandProgram2) const;
};

struct SecSvcFeesFareIdInfo
{
  SvcFeesFareIdInfo* _svcFeesFareIdInfo = nullptr;
  bool _status = false;
};

class S8BrandedFaresSelector : boost::noncopyable, public BrandedFaresValidator
{
  friend class S8BrandedFaresSelectorTest;
  friend class S8BrandedFaresValidatorTest;

public:
  S8BrandedFaresSelector(PricingTrx& trx);

  PaxTypeFare::BrandStatus validateFare(const PaxTypeFare* paxTypeFare,
                                        const BrandProgram* brandPr,
                                        const BrandInfo* brand,
                                        bool& needBrandSeparator,
                                        BrandedFareDiagnostics& diagnostics,
                                        bool skipHardPassValidation = false) const override;

protected:

  PricingTrx& _trx;

private:


  StatusT189 validateT189Data(const PaxTypeFare* paxTypeFare,
                              SvcFeesFareIdInfo* svcFeesFareIdInfo,
                              const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
                              std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec,
                              const BrandProgram* brandPr,
                              bool skipHardPassValidation = false) const;
  bool matchOneWayRoundTrip(const Indicator& owrtFromRule, const Indicator& owrtFromFare) const;
  bool matchPassengerType(const PaxTypeCode& passengerTypeFromRule,
                          const PaxTypeCode& passengerTypeFromFare,
                          const BrandProgram* brandPr) const;
  bool matchRouting(const std::string& routingFromRule, const PaxTypeFare& ptf) const;
  bool matchBookingCode(const BookingCode& bookingCode,
                        const std::vector<BookingCode>& primeBookingCodeVec) const;
  void getOneCharBookingCode(BookingCode& bookingCode) const;
  bool matchSource(const Indicator& sourceFromRule, const VendorCode& vendor) const;
  bool matchRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                          const TariffCategory& tariffCategory) const;
  bool matchRuleTariff(const TariffNumber& ruleTariff, const PaxTypeFare& ptf) const;
  bool matchRule(const RuleNumber& ruleFromRule, const RuleNumber& ruleFromFare) const;
  StatusT189
  isMinAndMaxFare(SvcFeesFareIdInfo* svcFeesFareIdInfo, const PaxTypeFare& ptf, bool fareRange1) const;
  StatusT189 validateBookingCode(const PaxTypeFare* paxTypeFare,
                                 const SvcFeesFareIdInfo* svcFeesFareIdInfo,
                                 const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
                                 bool& matchSecondaryT189,
                                 std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec,
                                 bool skipHardPassValidation = false) const;
  void
  getFBRBookingCode(const PaxTypeFare* paxTypeFare, std::vector<BookingCode>& primeBookingCodeVec) const;
  bool processT189Secondary(const std::vector<BookingCode>& primeBookingCodeVec,
                            const std::vector<SvcFeesFareIdInfo*>& fareIDdataSecondaryT189,
                            std::vector<SecSvcFeesFareIdInfo*>& secondarySvcFeesFareIdInfoVec) const;

  static const ServiceRuleTariffInd RULE_TARIFF_IND_PUBLIC;
  static const ServiceRuleTariffInd RULE_TARIFF_IND_PRIVATE;
  static constexpr Indicator CHAR_BLANK = ' ';
};
} // tse
