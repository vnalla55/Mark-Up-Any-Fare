//-------------------------------------------------------------------
//
//  File:        ValueCodeUtil.h
//  Created:     November 08, 2010
//  Authors:     Artur Krezel
//
//  Description:
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <tuple>

namespace tse
{
class ValueCodeAlgorithm;
class NegPaxTypeFareRuleData;
class PaxTypeFare;
class PricingTrx;
class FarePath;
class FareUsage;

class ValueCodeUtil
{
  friend class ValueCodeUtilTest;

public:
  ValueCodeUtil();
  virtual ~ValueCodeUtil();

  typedef bool (*GetInfoFun)(PricingTrx&,
                             const FareUsage*,
                             std::tuple<std::string&, Indicator&, Indicator&>&);

  static std::string
  getDecodedValueCode(const ValueCodeAlgorithm& valCodeAlg, const std::string& inputValue);
  static NegPaxTypeFareRuleData* getRuleDataForNetRemitCat35Fare(const PaxTypeFare& ptf);
  static std::string getHashedValueCodeData(const ValueCodeAlgorithm& valCodeAlg);
  static bool sameHashedValueCodeData(const ValueCodeAlgorithm& valCodeAlg1,
                                      const ValueCodeAlgorithm& valCodeAlg2);

  static bool decodeValueCode(PricingTrx& trx, const FarePath& farePath);
  static bool hasDynamicValueCodeFare(const FarePath& farePath);
  static bool hasCat35ValueCode(const FarePath& farePath);
  static bool validateDynamicValueCodeCombination(PricingTrx& trx, const FarePath& farePath);
  static bool validateStaticValueCodeCombination(PricingTrx& trx, const FarePath& farePath);

  static std::string getCat18ValueCode(const FareUsage& fu);
  static void saveStaticValueCode(PricingTrx& trx, const FarePath& farePath);
  static std::string createStaticValueCode(PricingTrx& trx,
                                           const std::vector<const FareUsage*>& fareUsages,
                                           Indicator& type,
                                           GetInfoFun getInfo = &getNonEmptyStaticValueCodeInfo);

  static void getFareUsages(const FarePath& fp, std::vector<const FareUsage*>& fareUsages);

private:
  static char decodeValue(char value, const ValueCodeAlgorithm& valCodeAlg);
  static const ValueCodeAlgorithm*
  getFirstDAVCandExcludeCat12(const FarePath& farePath, bool& excludeCat12);

  static MoneyAmount getConvertedTotalNetAmount(PricingTrx& trx,
                                                const FarePath& farePath,
                                                const MoneyAmount cat35NetTotalAmt,
                                                bool dVACProcess);

  static int getNumDecimalForPaymentCurrency(PricingTrx& trx);

  static int getNumDecimalForBaseFareCurrency(PricingTrx& trx, const FarePath& farePath);

  static const std::string getFormattedTotalNetAmt(const MoneyAmount amt, const int numDecimal);
  static bool
  validateDynamicValueCodeCombination(PricingTrx& trx, std::vector<const FareUsage*>& fareUsages);
  static bool
  validateStaticValueCodeCombination(PricingTrx& trx, std::vector<const FareUsage*>& fareUsages);
  static bool
  matchStaticValueCodeCombination(PricingTrx& trx, const FareUsage* fu1, const FareUsage* fu2);
  static bool matchStaticValueCodeCombinationWithCat18(const FareUsage* fu1,
                                                       const FareUsage* fu2,
                                                       Indicator passOn,
                                                       Indicator cat35VcInd,
                                                       const std::string& cat35Vc);
  static bool getDynamicValueCodeInfo(PricingTrx& trx,
                                      const FareUsage& fareUsage,
                                      const ValueCodeAlgorithm*& valueCodeAlgorithm,
                                      bool& excludeQSurchargeInd);
  static bool isNetRemitExcludeCat12(const NegPaxTypeFareRuleData* ruleData);
  static CarrierCode getCarrier(const PaxTypeFare& paxTypeFare);
  static std::string
  getStaticValueCode(const FareUsage& fu, const NegPaxTypeFareRuleData* negPTFRule);
  static std::string collectStaticValueCodes(GetInfoFun getInfo,
                                             PricingTrx& trx,
                                             const FareUsage* fareUsage,
                                             std::set<std::string>& uniqueCodes);

  static bool
  getNonEmptyStaticValueCodeInfo(PricingTrx& trx,
                                 const FareUsage* fareUsage,
                                 std::tuple<std::string&, Indicator&, Indicator&>& codesTuple);

  static bool getStaticValueCodeInfo(PricingTrx& trx,
                                     const FareUsage* fareUsage,
                                     std::tuple<std::string&, Indicator&, Indicator&>& codesTuple,
                                     bool forcePassForNA);
};
}

