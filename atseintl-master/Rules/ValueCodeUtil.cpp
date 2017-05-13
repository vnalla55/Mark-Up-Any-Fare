#include "Rules/ValueCodeUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Currency.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/ValueCodeAlgorithm.h"
#include "FareCalc/FareCalcConsts.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include <stdlib.h>

namespace
{

struct DynamicValueCodeCheck
{
  bool operator()(const tse::NegPaxTypeFareRuleData& ruleData) const
  {
    return ruleData.valueCodeAlgorithm();
  }
};

struct Cat35ValueCodeCheck
{
  bool operator()(const tse::NegPaxTypeFareRuleData& ruleData) const
  {
    return ruleData.negFareRestExt() && !ruleData.negFareRestExt()->staticValueCode().empty();
  }
};

template <class T>
bool
hasValueCodeProperty(const tse::FarePath& fp, const T& checker)
{
  for (const tse::PricingUnit* pu : fp.pricingUnit())
  {
    for (const tse::FareUsage* fu : pu->fareUsage())
    {
      const tse::PaxTypeFare& paxTypeFare = *fu->paxTypeFare();
      if (paxTypeFare.isNegotiated())
      {
        tse::NegPaxTypeFareRuleData* ruleData =
            tse::ValueCodeUtil::getRuleDataForNetRemitCat35Fare(paxTypeFare);
        if (ruleData && checker(*ruleData))
          return true;
      }
    }
  }
  return false;
}

template <typename T>
class add_iterator : std::back_insert_iterator<T>
{
public:
  explicit add_iterator(T& t) : std::back_insert_iterator<T>(t) {}

  add_iterator& operator=(const T& t)
  {
    *std::back_insert_iterator<T>::container += t;
    return *this;
  }

  add_iterator& operator*() { return *this; }

  add_iterator& operator++() { return *this; }

  add_iterator operator++(int) { return *this; }
};

template <typename T>
inline add_iterator<T>
adder(T& t)
{
  return add_iterator<T>(t);
}
}

namespace tse
{

namespace
{
}

ValueCodeUtil::ValueCodeUtil() {}

ValueCodeUtil::~ValueCodeUtil() {}

char
ValueCodeUtil::decodeValue(char value, const ValueCodeAlgorithm& valCodeAlg)
{
  switch (value)
  {
  case '.':
    return valCodeAlg.decimalChar();
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return valCodeAlg.digitToChar()[value - '0'];
  default:
    return ' ';
  }
}

std::string
ValueCodeUtil::getDecodedValueCode(const ValueCodeAlgorithm& valCodeAlg,
                                   const std::string& inputValue)
{
  std::string decodedValueCode = valCodeAlg.prefix();
  const Indicator* pos = std::find(valCodeAlg.digitToChar(),
                                   valCodeAlg.digitToChar() + ValueCodeAlgorithm::NUMBER_OF_DIGITS,
                                   ' ');
  if (pos == valCodeAlg.digitToChar() + ValueCodeAlgorithm::NUMBER_OF_DIGITS)
  {
    std::string::const_iterator inputValueIE =
        (valCodeAlg.decimalChar() == ' ') ? std::find(inputValue.begin(), inputValue.end(), '.')
                                          : inputValue.end();

    std::transform(inputValue.begin(),
                   inputValueIE,
                   std::back_inserter(decodedValueCode),
                   boost::bind<char>(&ValueCodeUtil::decodeValue, _1, boost::ref(valCodeAlg)));
  }

  decodedValueCode += valCodeAlg.suffix();

  return decodedValueCode;
}

NegPaxTypeFareRuleData*
ValueCodeUtil::getRuleDataForNetRemitCat35Fare(const PaxTypeFare& ptf)
{
  NegPaxTypeFareRuleData* ruleData = nullptr;
  const NegFareRest* negFareRest = nullptr;
  negFareRest = NegotiatedFareRuleUtil::getCat35Record3(&ptf, ruleData);
  if (negFareRest && NegotiatedFareRuleUtil::isNetRemitFare(ptf.fcaDisplayCatType(), negFareRest))
    return ruleData;
  return nullptr;
}

std::string
ValueCodeUtil::getHashedValueCodeData(const ValueCodeAlgorithm& valCodeAlg)
{
  std::string valueCode;
  std::copy(valCodeAlg.digitToChar(),
            valCodeAlg.digitToChar() + ValueCodeAlgorithm::NUMBER_OF_DIGITS,
            std::back_inserter(valueCode));

  return valCodeAlg.prefix() + valCodeAlg.decimalChar() + valueCode + valCodeAlg.suffix();
}

bool
ValueCodeUtil::sameHashedValueCodeData(const ValueCodeAlgorithm& valCodeAlg1,
                                       const ValueCodeAlgorithm& valCodeAlg2)
{
  const char* digitToChar1 = valCodeAlg1.digitToChar();
  const char* digitToChar2 = valCodeAlg2.digitToChar();
  for (; digitToChar1 != valCodeAlg1.digitToChar() + ValueCodeAlgorithm::NUMBER_OF_DIGITS &&
             digitToChar2 != valCodeAlg2.digitToChar() + ValueCodeAlgorithm::NUMBER_OF_DIGITS;
       ++digitToChar1, ++digitToChar2)
  {
    if (*digitToChar1 != *digitToChar2)
      return false;
  }

  if (valCodeAlg1.decimalChar() != valCodeAlg2.decimalChar() ||
      valCodeAlg1.prefix() != valCodeAlg2.prefix() || valCodeAlg1.suffix() != valCodeAlg2.suffix())
    return false;

  return true;
}

const ValueCodeAlgorithm*
ValueCodeUtil::getFirstDAVCandExcludeCat12(const FarePath& farePath, bool& excludeCat12)
{
  const ValueCodeAlgorithm* dvac = nullptr;

  for (const auto* pu : farePath.pricingUnit())
  {
    for (const auto* fu : pu->fareUsage())
    {
      const NegPaxTypeFareRuleData* negPTFRule = fu->paxTypeFare()->getNegRuleData();
      if (negPTFRule && negPTFRule->valueCodeAlgorithm())
      {
        dvac = negPTFRule->valueCodeAlgorithm();
        // ecludeCat12 is coded with DVAC only. this option should be the same for all fares.
        if (negPTFRule->fareProperties() &&
            negPTFRule->fareProperties()->excludeQSurchargeInd() == 'Y')
          excludeCat12 = true;

        break;
      }
    }
  }
  return dvac;
}

MoneyAmount
ValueCodeUtil::getConvertedTotalNetAmount(PricingTrx& trx,
                                          const FarePath& farePath,
                                          const MoneyAmount cat35NetTotalAmt,
                                          bool dvacWithBaseFare)
{

  CurrencyCode baseFareCurrency = farePath.baseFareCurrency();
  CurrencyCode calculationCurrency = farePath.calculationCurrency();
  const CurrencyCode paymentCurrency =
      (!trx.getOptions()->currencyOverride().empty())
          ? trx.getOptions()->currencyOverride()
          : CurrencyCode(trx.getRequest()->ticketingAgent()->currencyCodeAgent());
  CurrencyConversionFacade ccFacade;
  ccFacade.setRoundFare(true);

  MoneyAmount totalAmount = cat35NetTotalAmt;

  if (calculationCurrency != baseFareCurrency)
  {
    Money targetMoney(baseFareCurrency);
    targetMoney.value() = 0;
    if (const_cast<FarePath&>(farePath).applyNonIATARounding(trx))
      targetMoney.setApplyNonIATARounding();
    Money sourceMoneyCalculation(cat35NetTotalAmt, calculationCurrency);
    if (ccFacade.convert(
            targetMoney, sourceMoneyCalculation, trx, farePath.itin()->useInternationalRounding()))
    {
      totalAmount = targetMoney.value();
    }
    ccFacade.setRoundFare(true);
  }
  if (baseFareCurrency != paymentCurrency && !dvacWithBaseFare)
  {
    Money targetMoney(paymentCurrency);

    targetMoney.value() = 0;
    Money sourceMoneyCalculation(totalAmount, baseFareCurrency);
    if (ccFacade.convert(
            targetMoney, sourceMoneyCalculation, trx, farePath.itin()->useInternationalRounding()))
    {
      totalAmount = targetMoney.value();
    }
    else
      totalAmount = 0;
  }

  return totalAmount;
}

int
ValueCodeUtil::getNumDecimalForPaymentCurrency(PricingTrx& trx)
{
  int noDec = 0;
  CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!trx.getOptions()->currencyOverride().empty())
    paymentCurrency = trx.getOptions()->currencyOverride();

  const tse::Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency( paymentCurrency );

  if (currency != nullptr)
  {
    noDec = currency->noDec();
  }
  return noDec;
}
int
ValueCodeUtil::getNumDecimalForBaseFareCurrency(PricingTrx& trx, const FarePath& farePath)
{
  int noDec = 0;
  CurrencyCode baseFareCurrency = farePath.baseFareCurrency();

  const tse::Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency( baseFareCurrency );

  if (currency != nullptr)
  {
    noDec = currency->noDec();
  }
  return noDec;
}

const std::string
ValueCodeUtil::getFormattedTotalNetAmt(const MoneyAmount amt, const int numDecimal)
{
  if (amt == 0 && numDecimal == 0)
    return ("0");

  char buffor[50];

  std::sprintf(buffor, "%-.*f", numDecimal, amt);

  return buffor;
}

bool
ValueCodeUtil::decodeValueCode(PricingTrx& trx, const FarePath& farePath)
{
  if (farePath.collectedNegFareData())
  {
    bool dvacWithBaseFare = false;

    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);
    if (fcConfig && fcConfig->valueCodeBase() != FareCalcConsts::FC_NO)
      dvacWithBaseFare = true;

    CollectedNegFareData* cNegFareData = nullptr;
    MoneyAmount cat35NetTotalAmt;
    MoneyAmount cat35ConvertedNetTotalAmt;
    // all DVAC's should be the same at this point (prefix/value/suffix)
    bool excludeCat12 = false;
    const ValueCodeAlgorithm* valCodeAlg = getFirstDAVCandExcludeCat12(farePath, excludeCat12);
    if (valCodeAlg == nullptr)
      return false; // no need to decode by DVAC

    cNegFareData = const_cast<CollectedNegFareData*>(farePath.collectedNegFareData());
    cat35NetTotalAmt = cNegFareData->netTotalAmt();
    cat35NetTotalAmt += cNegFareData->totalMileageCharges();
    cat35NetTotalAmt += cNegFareData->otherSurchargeTotalAmt();
    if (!excludeCat12)
      cat35NetTotalAmt += cNegFareData->cat12SurchargeTotalAmt();

    cat35ConvertedNetTotalAmt =
        getConvertedTotalNetAmount(trx, farePath, cat35NetTotalAmt, dvacWithBaseFare);
    int numDecimal = 0;

    if (dvacWithBaseFare)
      numDecimal = getNumDecimalForBaseFareCurrency(trx, farePath);
    else
      numDecimal = getNumDecimalForPaymentCurrency(trx);

    const std::string formattedTotalNetAmt =
        getFormattedTotalNetAmt(cat35ConvertedNetTotalAmt, numDecimal);
    std::string valueCode = getDecodedValueCode(*valCodeAlg, formattedTotalNetAmt);

    if (!valueCode.empty())
    {
      cNegFareData->valueCode() = valueCode; // save DVAC
      return true;
    }
  }
  return false;
}

bool
ValueCodeUtil::validateDynamicValueCodeCombination(PricingTrx& trx, const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages;
  getFareUsages(farePath, fareUsages);
  return validateDynamicValueCodeCombination(trx, fareUsages);
}

bool
ValueCodeUtil::validateStaticValueCodeCombination(PricingTrx& trx, const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages;
  getFareUsages(farePath, fareUsages);
  return validateStaticValueCodeCombination(trx, fareUsages);
}

bool
ValueCodeUtil::validateDynamicValueCodeCombination(PricingTrx& trx,
                                                   std::vector<const FareUsage*>& fareUsages)
{
  if (fareUsages.empty())
    return true;

  const ValueCodeAlgorithm* valueCodeAlgorithm = nullptr;
  bool isFirstFareNetRemitExcludeCat12 = false;

  if (!getDynamicValueCodeInfo(
          trx, *fareUsages.front(), valueCodeAlgorithm, isFirstFareNetRemitExcludeCat12))
    return false;

  const PaxTypeFare* firstPTF = fareUsages.front()->paxTypeFare();
  CarrierCode carrier = getCarrier(*firstPTF);
  bool sameCarriers = true;
  bool hasEmpty = false;

  std::vector<const FareUsage*>::const_iterator fuIt = fareUsages.begin(),
                                                fuItEnd = fareUsages.end();
  for (fuIt++; fuIt != fuItEnd; fuIt++)
  {
    const FareUsage& currentFu = *(*fuIt);
    const PaxTypeFare* currentPTF = currentFu.paxTypeFare();
    const ValueCodeAlgorithm* currentValueCodeAlgorithm = nullptr;
    bool isCurrentFareNetRemitExcludeCat12 = false;

    if (!getDynamicValueCodeInfo(
            trx, currentFu, currentValueCodeAlgorithm, isCurrentFareNetRemitExcludeCat12))
      return false;

    if (isFirstFareNetRemitExcludeCat12 != isCurrentFareNetRemitExcludeCat12)
      return false;

    if (sameCarriers)
      sameCarriers = getCarrier(*currentPTF) == carrier;

    if (valueCodeAlgorithm)
    {
      if (currentValueCodeAlgorithm)
      {
        if (!sameHashedValueCodeData(*valueCodeAlgorithm, *currentValueCodeAlgorithm))
          return false;
      }
      else // check static
      {
        if (!sameCarriers)
          return false;
        hasEmpty = true;
      }
    }
    else
    {
      hasEmpty = true;
      if (currentValueCodeAlgorithm)
      {
        if (!sameCarriers)
          return false;
        valueCodeAlgorithm = currentValueCodeAlgorithm;
      }
    }
    if (hasEmpty && !sameCarriers)
      return false;
  }
  return true;
}

bool
ValueCodeUtil::validateStaticValueCodeCombination(PricingTrx& trx,
                                                  std::vector<const FareUsage*>& fareUsages)
{
  if (fareUsages.empty())
    return true;

  std::vector<const FareUsage*>::iterator firstCat35VC = fareUsages.begin();
  NegPaxTypeFareRuleData* nfrData = nullptr;
  Cat35ValueCodeCheck cat35check;
  int cnt = 1;

  do
  {
    nfrData = ValueCodeUtil::getRuleDataForNetRemitCat35Fare(*(*firstCat35VC)->paxTypeFare());
    if (nfrData && cat35check(*nfrData))
    {
      Indicator staticVCInd = nfrData->negFareRestExt()->staticValueCodeCombInd();
      if ((staticVCInd == RuleConst::DISPLAY_OPTION_1ST && cnt > 1) ||
          (staticVCInd == RuleConst::DISPLAY_OPTION_2ND && cnt > 2))
      {
        return false;
      }
      break;
    }

    if (++firstCat35VC == fareUsages.end())
      return true; // No Cat35 found

    cnt++;
  } while (1);

  return (std::find_if(fareUsages.begin(),
                       firstCat35VC,
                       !boost::bind<bool>(&ValueCodeUtil::matchStaticValueCodeCombination,
                                          boost::ref(trx),
                                          _1,
                                          *firstCat35VC)) == firstCat35VC) &&
         (std::find_if(firstCat35VC + 1,
                       fareUsages.end(),
                       !boost::bind<bool>(&ValueCodeUtil::matchStaticValueCodeCombination,
                                          boost::ref(trx),
                                          *firstCat35VC,
                                          _1)) == fareUsages.end());
}

bool
ValueCodeUtil::matchStaticValueCodeCombination(PricingTrx& trx,
                                               const FareUsage* fu1,
                                               const FareUsage* fu2)
{
  if (fu1 == fu2)
    return true;

  std::string vc1, vc2;
  Indicator ind;
  Indicator vc1Ind = RuleConst::DISPLAY_OPTION_BLANK, vc2Ind = RuleConst::DISPLAY_OPTION_BLANK;
  std::tuple<std::string&, Indicator&, Indicator&> codesTuple1(vc1, vc1Ind, ind);
  std::tuple<std::string&, Indicator&, Indicator&> codesTuple2(vc2, vc2Ind, ind);

  if (!getStaticValueCodeInfo(trx, fu1, codesTuple1, true) ||
      !getStaticValueCodeInfo(trx, fu2, codesTuple2, true))
  {
    return false;
  }

  if (!vc1.empty() && !vc2.empty())
  {
    return (vc1Ind == vc2Ind) && (vc1Ind != RuleConst::DISPLAY_OPTION_BLANK ||
                                  vc2Ind != RuleConst::DISPLAY_OPTION_BLANK || vc1 == vc2);
  }
  else if (!vc1.empty())
  {
    return matchStaticValueCodeCombinationWithCat18(
        fu2, fu1, RuleConst::DISPLAY_OPTION_1ST, vc1Ind, vc1);
  }
  else if (!vc2.empty())
  {
    return matchStaticValueCodeCombinationWithCat18(
        fu1, fu2, RuleConst::DISPLAY_OPTION_2ND, vc2Ind, vc2);
  }

  return true;
}

bool
ValueCodeUtil::matchStaticValueCodeCombinationWithCat18(const FareUsage* fu1,
                                                        const FareUsage* fu2,
                                                        Indicator passOn,
                                                        Indicator cat35VcInd,
                                                        const std::string& cat35Vc)
{
  std::string cat18vc = getCat18ValueCode(*fu1);
  if (!cat18vc.empty())
  {
    return cat35VcInd == RuleConst::DISPLAY_OPTION_BLANK && cat18vc == cat35Vc;
  }
  return cat35VcInd == passOn || (cat35VcInd == RuleConst::DISPLAY_OPTION_BLANK &&
                                  fu1->paxTypeFare()->carrier() == fu2->paxTypeFare()->carrier());
}

bool
ValueCodeUtil::getDynamicValueCodeInfo(PricingTrx& trx,
                                       const FareUsage& fareUsage,
                                       const ValueCodeAlgorithm*& valueCodeAlgorithm,
                                       bool& excludeQSurchargeInd)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();
  if (trx.dataHandle().getVendorType(paxTypeFare->vendor()) != RuleConst::SMF_VENDOR)
    return false;

  if (paxTypeFare->isNegotiated())
  {
    const NegPaxTypeFareRuleData* ruleData = getRuleDataForNetRemitCat35Fare(*paxTypeFare);
    if (ruleData->valueCodeAlgorithm())
    {
      valueCodeAlgorithm = ruleData->valueCodeAlgorithm();
      excludeQSurchargeInd = isNetRemitExcludeCat12(ruleData);
    }
    else
    {
      std::string staticVC = getStaticValueCode(fareUsage, ruleData);
      if (!staticVC.empty())
        return false;
    }
    return true;
  }
  return false;
}

CarrierCode
ValueCodeUtil::getCarrier(const PaxTypeFare& paxTypeFare)
{
  CarrierCode carrier = paxTypeFare.carrier();
  if (carrier == INDUSTRY_CARRIER)
    carrier = paxTypeFare.fareMarket()->governingCarrier();
  return carrier;
}

bool
ValueCodeUtil::isNetRemitExcludeCat12(const NegPaxTypeFareRuleData* ruleData)
{
  return (ruleData->fareProperties() && ruleData->fareProperties()->excludeQSurchargeInd() == 'Y');
}

void
ValueCodeUtil::getFareUsages(const FarePath& fp, std::vector<const FareUsage*>& fareUsages)
{
  for (const PricingUnit* pu : fp.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      fareUsages.push_back(fu);
}

bool
ValueCodeUtil::hasDynamicValueCodeFare(const FarePath& fp)
{
  return hasValueCodeProperty(fp, DynamicValueCodeCheck());
}

bool
ValueCodeUtil::hasCat35ValueCode(const FarePath& fp)
{
  return hasValueCodeProperty(fp, Cat35ValueCodeCheck());
}

std::string
ValueCodeUtil::getStaticValueCode(const FareUsage& fu, const NegPaxTypeFareRuleData* negPTFRule)
{ // this is for SMF only.
  std::string staticVC;

  if (negPTFRule && negPTFRule->negFareRestExt() &&
      !negPTFRule->negFareRestExt()->staticValueCode().empty())
    staticVC = negPTFRule->negFareRestExt()->staticValueCode();
  else // check cat18
  {
    staticVC = getCat18ValueCode(fu);
  }
  return staticVC;
}

std::string
ValueCodeUtil::getCat18ValueCode(const FareUsage& fu)
{
  std::string staticCat18VC;
  if (fu.tktEndorsement().size() >= 1)
  {
    std::vector<TicketEndorseItem>& tktEndorsement = const_cast<FareUsage&>(fu).tktEndorsement();

    Indicator tktLocInd = tktEndorsement.front().tktLocInd;
    if (tktLocInd == '1' || tktLocInd == '3' || tktLocInd == '5')
    {
      staticCat18VC = tktEndorsement.front().endorsementTxt;
    }
  }
  return staticCat18VC;
}

void
ValueCodeUtil::saveStaticValueCode(PricingTrx& trx, const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages;
  getFareUsages(farePath, fareUsages);
  if (farePath.collectedNegFareData())
  {
    CollectedNegFareData* cNegFareData = nullptr;
    Indicator type;
    std::string staticVC = createStaticValueCode(trx, fareUsages, type);
    if (!staticVC.empty())
    {
      cNegFareData = const_cast<CollectedNegFareData*>(farePath.collectedNegFareData());
      cNegFareData->valueCode() = staticVC;
    }
  }
  return;
}

std::string
ValueCodeUtil::createStaticValueCode(PricingTrx& trx,
                                     const std::vector<const FareUsage*>& fareUsages,
                                     Indicator& type,
                                     GetInfoFun getInfo)
{
  std::set<std::string> uniqueCodes;
  std::string code;
  Indicator ind;
  std::tuple<std::string&, Indicator&, Indicator&> codesTuple(code, ind, type);

  const std::vector<const FareUsage*>::const_iterator fuI =
      std::find_if(fareUsages.begin(),
                   fareUsages.end(),
                   boost::bind<bool>(getInfo, boost::ref(trx), _1, boost::ref(codesTuple)));

  if (fuI == fareUsages.end() || fareUsages.size() == 1)
    return code;

  if (ind == RuleConst::DISPLAY_OPTION_2ND)
  {
    if (fareUsages.size() > 1 && getInfo(trx, fareUsages[1], codesTuple))
      return code;
    return "";
  }
  else if (ind == RuleConst::DISPLAY_OPTION_ALL)
  {
    uniqueCodes.insert(code);
    std::transform(fuI + 1,
                   fareUsages.end(),
                   adder(code),
                   boost::bind<std::string>(&ValueCodeUtil::collectStaticValueCodes,
                                            getInfo,
                                            boost::ref(trx),
                                            _1,
                                            boost::ref(uniqueCodes)));
  }

  return code;
}

bool
ValueCodeUtil::getNonEmptyStaticValueCodeInfo(
    PricingTrx& trx,
    const FareUsage* fareUsage,
    std::tuple<std::string&, Indicator&, Indicator&>& codesTuple)
{
  return ValueCodeUtil::getStaticValueCodeInfo(trx, fareUsage, codesTuple, false);
}

bool
ValueCodeUtil::getStaticValueCodeInfo(
    PricingTrx& trx,
    const FareUsage* fareUsage,
    std::tuple<std::string&, Indicator&, Indicator&>& codesTuple,
    bool forcePassForNA)
{
  const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare();
  if (trx.dataHandle().getVendorType(paxTypeFare->vendor()) != RuleConst::SMF_VENDOR)
    return false;

  if (paxTypeFare->isNegotiated())
  {
    NegPaxTypeFareRuleData* negPTFRule = getRuleDataForNetRemitCat35Fare(*paxTypeFare);
    if (negPTFRule && negPTFRule->negFareRestExt() &&
        !negPTFRule->negFareRestExt()->staticValueCode().empty())
    {
      std::get<0>(codesTuple) = negPTFRule->negFareRestExt()->staticValueCode();
      std::get<1>(codesTuple) = negPTFRule->negFareRestExt()->staticValueCodeCombInd();
      return true;
    }
    return forcePassForNA;
  }
  return false;
}

std::string
ValueCodeUtil::collectStaticValueCodes(GetInfoFun getInfo,
                                       PricingTrx& trx,
                                       const FareUsage* fareUsage,
                                       std::set<std::string>& uniqueCodes)
{
  std::string code;
  Indicator ind;
  Indicator type;
  std::tuple<std::string&, Indicator&, Indicator&> codesTuple(code, ind, type);

  getInfo(trx, fareUsage, codesTuple);

  if (code.empty() || uniqueCodes.find(code) != uniqueCodes.end())
    return "";
  uniqueCodes.insert(code);

  return "/" + code;
}
}
