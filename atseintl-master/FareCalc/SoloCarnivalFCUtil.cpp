//----------------------------------------------------------------------------
//  File:        AirlineShoppingFCUtil.cpp
//  Created:     2011-09-20
//
//  Description: Solo Carnival Shopping Fare Calc utility class
//
//  Updates:
//
//  Copyright Sabre 2011
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

#include "FareCalc/SoloCarnivalFCUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "FareCalc/FareCalcCollector.h"

#include <algorithm>
#include <exception>

namespace tse
{
namespace
{

log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.FareCalc.SoloCarnivalFCUtil"));

/**
 * This is comparer for std::min_element, that is used in findCheapestItinGroup()
 *
 * If a group cannot be priced, that group is considered as more expensive to
 * avoid it be considered as cheaper one group in overall result.
 *
 * If both groups cannot be prices, the function returns false.
 *
 * If both groups are of equal price, than shorter one is considered as cheaper.
 *
 * @throws exception if money compare fails, the exception is to be caught within the class scope
 */
bool
lessIfCheaper(const SOLItinGroups::ItinGroup* lhs, const SOLItinGroups::ItinGroup* rhs)
{
  boost::optional<Money> calc_lhs;
  if (!lhs || !(calc_lhs = lhs->getTotalPrice()))
  {
    return false; // exclude lhs arg from min_element() result
  }

  boost::optional<Money> calc_rhs;
  if (!rhs || !(calc_rhs = rhs->getTotalPrice()))
  {
    return true; // exclude rhs arg from min_element() result
  }

  Money money_lhs = *calc_lhs, money_rhs = *calc_rhs;

  bool leftIsCheaper = (money_lhs < money_rhs);
  bool priceIsEqual = (money_lhs == money_rhs);
  bool leftIsShorter = (lhs->size() < rhs->size());

  return (leftIsCheaper || (priceIsEqual && leftIsShorter));
}

} // anonymous namespace

boost::optional<Money>
SoloCarnivalFCUtil::calcTotalAmount(const Itin& itin) try
{
  MoneyAmount totalAmount = 0;

  const uint16_t brandIndex = INVALID_BRAND_INDEX;

  FareCalcCollector* fareCalcCollector = FareCalcUtil::getFareCalcCollectorForItin(_trx, &itin);
  if (!fareCalcCollector)
  {
    LOG4CXX_DEBUG(_logger, "Cannot get fareCalcCollector for Itin");

    return boost::none;
  }

  CurrencyCode totalEquivalentCurrency = INVALID_CURRENCYCODE;
  CurrencyNoDec totalEquivalentNoDec = INVALID_CURRENCY_NO_DEC;

  MoneyAmount totalEquivalent = fareCalcCollector->getEquivFareAmountTotal(_trx,
      totalEquivalentCurrency, totalEquivalentNoDec, false, brandIndex);
  CurrencyCode totalTaxCurrency = INVALID_CURRENCYCODE;
  CurrencyNoDec totalTaxNoDec = INVALID_CURRENCY_NO_DEC;
  MoneyAmount totalTax = fareCalcCollector->getTaxTotal(_trx,
                                                        totalTaxCurrency,
                                                        totalTaxNoDec,
                                                        /*forNetRemit*/ false,
                                                        brandIndex);

  totalAmount =
      totalEquivalent + getAmountInCurrency(totalTax, totalTaxCurrency, totalEquivalentCurrency);

  Money result(totalAmount, totalEquivalentCurrency);
  LOG4CXX_DEBUG(_logger, "Itin total amount calculated as " << result);

  return result;
}
catch (const Money::InvalidOperation& e) // can be thrown by getAmountInCurrency()
{
  LOG4CXX_ERROR(_logger, "Money::InvalidOperation: " << e.what());

  return boost::none;
}

boost::optional<Money>
SoloCarnivalFCUtil::calcTotalAmount(const SOLItinGroups::ItinGroup& itinGroup) const
{
  boost::optional<Money> result;

  for (std::size_t itinIndex = 0; itinIndex < itinGroup.size(); ++itinIndex)
  {
    boost::optional<Money> itinAmount = itinGroup.getItinPrice(itinIndex);
    if (!itinAmount)
    {
      LOG4CXX_DEBUG(_logger, "Cannot calculate ItinGroup total amount");

      return boost::none;
    }
    else
    {
      if (!result)
        result = *itinAmount;
      else
        *result = *result + *itinAmount;
    }
  }

  return result;
}

boost::optional<SOLItinGroups::GroupType>
SoloCarnivalFCUtil::findCheapestItinGroup(const SOLItinGroups::ItinGroupVec& itinGroups) const
{
  SOLItinGroups::ItinGroupVec::const_iterator minElement =
      std::min_element(itinGroups.begin(), itinGroups.end(), lessIfCheaper);
  //                                                    !! ^^^^^^^^^^^^^ !!
  // actually, we are not going to catch Money::InvalidOperation from lessIfCheaper() comparer,
  // because we do not expect this exception to happen in MIP Solo Carnival

  SOLItinGroups::GroupType cheapestGroupType =
      static_cast<SOLItinGroups::GroupType>(minElement - itinGroups.begin());

  bool isValid = itinGroups[cheapestGroupType] && itinGroups[cheapestGroupType]->getTotalPrice();
  if (isValid)
    return cheapestGroupType;
  else
    return boost::none;
}

std::string
SoloCarnivalFCUtil::getFareCalcLine(const Itin& itin)
{
  FareCalcCollector* fareCalcCollector = FareCalcUtil::getFareCalcCollectorForItin(_trx, &itin);

  if (!fareCalcCollector)
  {
    LOG4CXX_DEBUG(_logger, "Cannot get fareCalcCollector for Itin");

    return "";
  }

  if (itin.farePath().empty())
  {
    LOG4CXX_DEBUG(_logger, "Empty FarePath vector for sub-Itin in SolItinGroupsMap");

    return "";
  }

  if (itin.farePath().size() != 1)
  {
    LOG4CXX_ERROR(_logger,
                  "Solo Carnival expects only one FarePath per sub-Itin in SolItinGroupsMap");

    return "";
  }

  FarePath* fpath = itin.farePath().at(0);
  if (!fpath)
  {
    LOG4CXX_WARN(_logger, "FarePath in NULL for sub-Itin in SolItinGroupsMap");

    return "";
  }

  CalcTotals* calcTotals = fareCalcCollector->findCalcTotals(fpath);
  if (!calcTotals)
  {
    LOG4CXX_DEBUG(_logger, "CalcTotals is not found for sub-Itin in SolItinGroupsMap");

    return "";
  }

  return calcTotals->fareCalculationLine;
}

void
SoloCarnivalFCUtil::assignPriceToSolItinGroupsMapItems()
{
  bool validItineraryFound = false;

  for (PricingTrx::SOLItinGroupsMap::const_iterator itinGroupMapIter =
           _trx.solItinGroupsMap().begin();
       itinGroupMapIter != _trx.solItinGroupsMap().end();
       ++itinGroupMapIter)
  {
    SOLItinGroups* groups = itinGroupMapIter->second;
    if (!groups)
    {
      LOG4CXX_WARN(_logger, "SOLItinGroups* is NULL in PricingTrx::SOLItinGroupsMap");
      continue;
    }

    for (auto itinGroup : groups->itinGroups())
    {
      if (!itinGroup)
        continue;

      for (std::size_t itinIndex = 0; itinIndex < itinGroup->size(); ++itinIndex)
      {
        Itin* itin = itinGroup->at(itinIndex);
        if (!itin)
          continue;

        itinGroup->setItinPrice(itinIndex, calcTotalAmount(*itin));
        itinGroup->setItinFarecalcLine(itinIndex, getFareCalcLine(*itin));

      } // ++itinIndex

      itinGroup->setTotalPrice(calcTotalAmount(*itinGroup));

    } // ++groupVecIter

    groups->markCheapestItinGroup(findCheapestItinGroup(groups->itinGroups()));

    if (groups->getCheapestItinGroup() != boost::none)
    {
      validItineraryFound = true;
    }
  } // ++itinGroupMapIter

  if (!validItineraryFound)
  {
    _trx.diagnostic().deActivate();
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }
}

MoneyAmount
SoloCarnivalFCUtil::getAmountInCurrency(const MoneyAmount sourceAmount,
                                        const CurrencyCode& sourceCurrency,
                                        const CurrencyCode& targetCurrency)
{
  CurrencyConversionFacade currencyConverter;

  const Money source(sourceAmount, sourceCurrency);
  Money target(targetCurrency);

  bool ok = currencyConverter.convert(target, source, _trx);
  if (!ok)
  {
    throw Money::InvalidOperation("CurrencyConversionFacade::convert() error converting from '" +
                                  sourceCurrency + "' to '" + targetCurrency + "'");
  }

  return target.value();
}
}
