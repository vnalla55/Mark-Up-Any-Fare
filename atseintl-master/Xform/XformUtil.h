// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/FreeBaggageUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/XMLConstruct.h"
#include "FareCalc/FareCalcCollector.h"

#include <vector>

namespace tse
{
#define MAX_NUM_OB_FEE  50

class CalcTotals;
class FarePath;
class PricingTrx;

class XformMsgFormatter
{
  friend class XformUtilTest;
public:
  static void splitBaggageText(const std::string& text,
                               const uint32_t maxLineLen,
                               std::vector<std::string>& output,
                               bool notRemoveEmptyLine = false);

  static void splitGreenScreeText(const std::string& text,
                                  const uint32_t maxLineLen,
                                  std::vector<std::string>& output);

  static void formatFc(const std::string& tagHead,
                       int tagSize,
                       const std::string& replaceString,
                       std::string& text);

protected:
  static void splitText(const std::string& text,
                        const uint32_t maxLineLen,
                        std::vector<std::string>& output,
                        bool trimToLeft = false,
                        bool notRemSpacesFromEmptyLine = false,
                        bool skipToLongLine = false);

  static void splitNewLine(const std::string& text,
                           std::vector<std::string>& output,
                           bool trimToLeft);

  static void splitLineLen(const uint32_t maxLineLen, std::vector<std::string>& output);
};

class MAFUtil
{

friend class XformUtilTest;

public:
  static void checkElementONV(PricingTrx& pricingTrx,
                              FareCalcCollector& fareCalcCollector,
                              XMLConstruct& xml);

  static void checkElementONV(PricingTrx& pricingTrx, const CalcTotals* calc, XMLConstruct& xml);

private:
  static void constructElementONV(XMLConstruct& xml);

};

class OBFeeUtil
{
public:
  static void getNumberOfOBFees(PricingTrx& trx,
                                const FarePath& farePath,
                                size_t& maxNumFType,
                                size_t& maxNumTType,
                                size_t& maxNumRType);
};

class TaxUtil
{
public:
  static bool getTaxRounding(PricingTrx& pricingTrx,
      const CurrencyCode& currencyCode,
      RoundingFactor& roundingFactor,
      CurrencyNoDec& roundingNoDec,
      RoundingRule& roundingRule);

  static void convertTaxCurrency(PricingTrx& pricingTrx,
      const Money& sourceMoney,
      Money& targetMoney);

  static void roundTaxCurrency(PricingTrx& pricingTrx, Money& targetMoney);
};

class BaggageResponseBuilder
{
  friend class XformUtilTest;

  typedef FareCalcCollector::BaggageTagMap BaggageTagMap;

public:
  BaggageResponseBuilder(PricingTrx& trx,
                         const BaggageTagMap& baggageTagMap,
                         const std::string& responseString);
  BaggageResponseBuilder(PricingTrx& trx, const CalcTotals& calcTotals);
  BaggageResponseBuilder(PricingTrx& trx,
                         const FarePath& fp,
                         const PaxTypeCode ptc = PaxTypeCode());

  BaggageResponseBuilder(const BaggageResponseBuilder&) = delete;
  BaggageResponseBuilder& operator=(const BaggageResponseBuilder&) = delete;

  void setCalcTotals(const CalcTotals& calcTotals);
  void setFarePath(const FarePath& fp, const PaxTypeCode paxType);
  void setBaggageTag(const std::string& baggageTag);
  void setIsUsDotForMultiTkt(PricingTrx& trx, const Itin* subItin);

  // Returns baggage text stored in price quote (PQ)
  bool getPqResponse(std::vector<std::string>& output, const Trx* trx = nullptr) const;
  // Returns baggage text displayed in green screen (for WPA and WP entries)
  bool getWpaGsResponse(std::string& output) const;
  bool getGsResponse(std::vector<std::string>& output) const;

private:
  void initialize(PricingTrx& trx);

  const CalcTotals* findCalcTotals(const BaggageFcTagType& paxType) const;
  BaggageFcTagType findLastBaggageId(const std::string& responseString) const;

  bool getPqResponseImpl(std::vector<std::string>& output, const Trx* trx) const;
  void getGsResponseImpl(std::vector<std::string>& output,
                         bool addEmbargoes) const;

  std::string formatPassengerInfo() const;

  const BaggageTagMap* _baggageTagMap = nullptr; // BAGGAGETEXT00X -> calc totals
  const FarePath* _farePath = nullptr;
  PaxTypeCode _paxType;
  BaggageFcTagType _baggageId;
  BaggageFcTagType _lastBaggageId;

  uint32_t _lineLength = 0;
  bool _warnMessages = false;
  bool _truePsgrTypeInd = false;
  bool _isUsDot = false;
  bool _useShortFooterMsg = false;
};

// util class for building tkt cxr reponse
class TicketingCxrResponseUtil
{
  public:
  static void prepareMessage(XMLConstruct& construct,
      const std::string& msgType,
      const uint16_t msgCode,
      const std::string& msgText);

  static void prepareResponseText(const std::string& responseString,
      XMLConstruct& construct,
      uint16_t& codeNum,
      const bool noSizeLImit=false);
};

class SplitFormatterUtil
{
  public:
  static std::string
  formatAmount(const MoneyAmount amount, const CurrencyNoDec decimalPlaces);

  static std::string
  formatAmount(const std::pair<MoneyAmount, CurrencyNoDec>& amountAndPrecision);
};

}

