#include "Rules/TourCodeUtil.h"

#include "Common/TrxUtil.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/ValueCodeUtil.h"

#include <boost/bind.hpp>

namespace tse
{
TourCodeUtil::TourCodeUtil(const std::vector<const PaxTypeFare*>& paxTypeFares)
  : _paxTypeFares(paxTypeFares)
{
}

TourCodeUtil::~TourCodeUtil() {}

bool
TourCodeUtil::useOptimusNetRemit(PricingTrx& trx) const
{
  return TrxUtil::optimusNetRemitEnabled(trx) && hasTourCodeCombInd() && _paxTypeFares.size() > 1;
}

bool
TourCodeUtil::validate(PricingTrx& trx)
{
  std::vector<TourCodeUtil::TourCodeTuple>::iterator firstTC = collectTourCodes(trx);

  if (firstTC == _tourCodeTuples.end())
    return false;

  return (std::find_if(_tourCodeTuples.begin(),
                       firstTC,
                       !boost::bind<bool>(&TourCodeUtil::matchTourCodeCombination, _1, *firstTC)) ==
          firstTC) &&
         (std::find_if(firstTC + 1,
                       _tourCodeTuples.end(),
                       !boost::bind<bool>(&TourCodeUtil::matchTourCodeCombination, *firstTC, _1)) ==
          _tourCodeTuples.end());
}

bool
TourCodeUtil::hasTourCodeCombInd() const
{
  for (const PaxTypeFare* ptf : _paxTypeFares)
  {
    if (ptf->isNegotiated())
    {
      NegPaxTypeFareRuleData* negPTFRule = ValueCodeUtil::getRuleDataForNetRemitCat35Fare(*ptf);
      if (UNLIKELY(negPTFRule && negPTFRule->negFareRestExt() &&
          negPTFRule->negFareRestExt()->tourCodeCombInd() != RuleConst::DISPLAY_OPTION_BLANK))
        return true;
    }
  }

  return false;
}

// return first found TC if exists, otherwise end.
std::vector<TourCodeUtil::TourCodeTuple>::iterator
TourCodeUtil::collectTourCodes(PricingTrx& trx)
{
  int ret = 0;
  int counter = 1;

  _tourCodes.reserve(_paxTypeFares.size());
  _tourCodeInds.reserve(_paxTypeFares.size());
  _tourCodeTypes.reserve(_paxTypeFares.size());
  _tourCodeTuples.reserve(_paxTypeFares.size());

  for (const PaxTypeFare* ptf : _paxTypeFares)
  {
    // allocate memory for results
    _tourCodes.push_back(std::string());
    _tourCodeInds.push_back(RuleConst::DISPLAY_OPTION_BLANK);
    _tourCodeTypes.push_back(RuleConst::DISPLAY_OPTION_BLANK);
    _tourCodeTuples.push_back(
        TourCodeTuple(_tourCodes.back(), _tourCodeInds.back(), _tourCodeTypes.back()));

    // get TC
    if (getTourCodeInfo(trx, ptf, _tourCodeTuples.back(), true))
    {
      if (!ret && !_tourCodes.back().empty())
      {
        if ((_tourCodeInds.back() == RuleConst::DISPLAY_OPTION_1ST && counter > 1) ||
            (_tourCodeInds.back() == RuleConst::DISPLAY_OPTION_2ND && counter > 2))
        {
          return _tourCodeTuples.end();
        }
        else
        {
          ret = counter;
        }
      }
    }
    else
      return _tourCodeTuples.end();

    counter++;
  }

  return ret ? (_tourCodeTuples.begin() + ret - 1) : _tourCodeTuples.end();
}

bool
TourCodeUtil::matchTourCodeCombination(const TourCodeUtil::TourCodeTuple& first,
                                       const TourCodeUtil::TourCodeTuple& second)
{
  const std::string& tc1 = std::get<0>(first);
  const std::string& tc2 = std::get<0>(second);
  Indicator tc1Ind = std::get<1>(first);
  Indicator tc2Ind = std::get<1>(second);
  Indicator tc1Type = std::get<2>(first);
  Indicator tc2Type = std::get<2>(second);

  if (!tc1.empty() && !tc2.empty())
  {
    return (tc1Ind == tc2Ind) && (tc1Ind != RuleConst::DISPLAY_OPTION_BLANK ||
                                  tc2Ind != RuleConst::DISPLAY_OPTION_BLANK || tc1 == tc2) &&
           (tc1Ind != RuleConst::DISPLAY_OPTION_ALL || tc2Ind != RuleConst::DISPLAY_OPTION_ALL ||
            tc1Type == tc2Type);
  }
  else if (!tc1.empty())
  {
    return tc1Ind == RuleConst::DISPLAY_OPTION_1ST || tc1Ind == RuleConst::DISPLAY_OPTION_BLANK;
  }
  else if (!tc2.empty())
  {
    return tc2Ind == RuleConst::DISPLAY_OPTION_2ND || tc2Ind == RuleConst::DISPLAY_OPTION_BLANK;
  }

  return true;
}

bool
TourCodeUtil::getTourCodeInfo(PricingTrx& trx,
                              const FareUsage* fareUsage,
                              TourCodeUtil::TourCodeTuple& codesTuple)
{
  return getTourCodeInfo(trx, fareUsage->paxTypeFare(), codesTuple, false);
}

bool
TourCodeUtil::getTourCodeInfo(PricingTrx& trx,
                              const PaxTypeFare* paxTypeFare,
                              TourCodeUtil::TourCodeTuple& codesTuple,
                              bool forcePassForNA)
{
  if (trx.dataHandle().getVendorType(paxTypeFare->vendor()) != RuleConst::SMF_VENDOR)
    return false;

  if (paxTypeFare->isNegotiated())
  {
    NegPaxTypeFareRuleData* ruleData = nullptr;
    const NegFareRest* negFareRest = nullptr;
    negFareRest = NegotiatedFareRuleUtil::getCat35Record3(paxTypeFare, ruleData);

    if (ruleData && negFareRest &&
        NegotiatedFareRuleUtil::isNetRemitFare(paxTypeFare->fcaDisplayCatType(), negFareRest) &&
        !negFareRest->tourBoxCode1().empty())
    {
      std::get<0>(codesTuple) = negFareRest->tourBoxCode1();
      if (ruleData->negFareRestExt())
        std::get<1>(codesTuple) = ruleData->negFareRestExt()->tourCodeCombInd();
      std::get<2>(codesTuple) = negFareRest->tourBoxCodeType1();

      return true;
    }
    return forcePassForNA;
  }
  return false;
}

bool
TourCodeUtil::createTourCodeForNetRemitSMF(PricingTrx& trx,
                                           const std::vector<const FareUsage*>& fareUsages,
                                           std::string& tourCode,
                                           Indicator& indTypeTC)
{
  tourCode = ValueCodeUtil::createStaticValueCode(
      trx, fareUsages, indTypeTC, &TourCodeUtil::getTourCodeInfo);

  if (!tourCode.empty())
    return true;

  return false;
}

void
TourCodeUtil::saveTourCodeForNetRemitSMF(PricingTrx& trx, const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages;
  ValueCodeUtil::getFareUsages(farePath, fareUsages);
  if (farePath.collectedNegFareData())
  {
    CollectedNegFareData* cNegFareData = nullptr;
    Indicator indTypeTC = ' ';
    std::string tourCode;
    bool rc = false;
    rc = createTourCodeForNetRemitSMF(trx, fareUsages, tourCode, indTypeTC);
    if (rc && !tourCode.empty())
    {
      cNegFareData = const_cast<CollectedNegFareData*>(farePath.collectedNegFareData());

      cNegFareData->tourCode() = tourCode;
      cNegFareData->indTypeTour() = indTypeTC;
    }
  }
  return;
}
}
