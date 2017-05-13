//-------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "RexPricing/RexPaxTypeFareValidator.h"

#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag602Collector.h"
#include "Diagnostic/DiagCollector.h"

using namespace std;

namespace tse
{

struct PaxTypeFareSegmentOrder
{
  bool operator()(const PaxTypeFare* paxTypeFare1, const PaxTypeFare* paxTypeFare2) const
  {
    return paxTypeFare1->fareMarket()->travelSeg().front()->segmentOrder() <
           paxTypeFare2->fareMarket()->travelSeg().front()->segmentOrder();
  }
};

RexPaxTypeFareValidator::RexPaxTypeFareValidator(RexPricingTrx& rexTrx, DiagCollector* dc)
  : _rexTrx(rexTrx), _excItin(*_rexTrx.exchangeItin().front()), _dc(dc)
{
}

RexPaxTypeFareValidator::~RexPaxTypeFareValidator() {}

bool
RexPaxTypeFareValidator::collectFullyFlownFares(std::vector<const PaxTypeFare*>& fares,
                                                const FarePath& farePath)
{
  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
  for (; puIter != farePath.pricingUnit().end(); ++puIter)
  {
    const PricingUnit& pu = **puIter;
    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();

    for (; fuIter != pu.fareUsage().end(); ++fuIter)
      if ((*fuIter)->paxTypeFare()->fareMarket()->changeStatus() == FL)
        fares.push_back((*fuIter)->paxTypeFare());
      else
        break;
  }

  return !fares.empty();
}

bool
RexPaxTypeFareValidator::collectFullyFlownFaresAfterBoardPoint(
    std::vector<const PaxTypeFare*>& fares, const FarePath& farePath, int16_t lowestSegmentOrder)
{
  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
  for (; puIter != farePath.pricingUnit().end(); ++puIter)
  {
    const PricingUnit& pu = **puIter;
    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();

    for (; fuIter != pu.fareUsage().end(); ++fuIter)
      if ((*fuIter)->paxTypeFare()->fareMarket()->changeStatus() == FL)
      {
        if ((*fuIter)->paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder() >=
            lowestSegmentOrder)
          fares.push_back((*fuIter)->paxTypeFare());
      }
      else
        break;
  }

  return !fares.empty();
}

void
RexPaxTypeFareValidator::displayDiagnostic(const PaxTypeFare& paxTypeFare,
                                           const MoneyAmount& newAmount,
                                           const MoneyAmount& excAmount)
{
  Diag602Collector* dc602 = nullptr;
  DCFactory* factory = setupDiagnostic(dc602);
  if (dc602)
  {
    dc602->initializeFilter(_rexTrx, paxTypeFare);
    *dc602 << paxTypeFare;
    dc602->displayErrorMessage(excAmount, newAmount);
  }
  releaseDiagnostic(factory, dc602);
}

bool
RexPaxTypeFareValidator::validate(const PaxTypeFare& paxTypeFare)
{
  if (_excFares.empty() && !collectFullyFlownFaresAfterBoardPoint(
                               _excFares,
                               *_excItin.farePath().front(),
                               paxTypeFare.fareMarket()->travelSeg().front()->segmentOrder()))
    return true;

  sort(_excFares.begin(), _excFares.end(), PaxTypeFareSegmentOrder());
  _newFares.clear();
  _newFares.push_back(&paxTypeFare);

  if (_excFares.front()->fareMarket()->travelSeg().front()->segmentOrder() !=
      paxTypeFare.fareMarket()->travelSeg().front()->segmentOrder())
    return true;

  vector<const PaxTypeFare*>::const_iterator excFaresIterFrom = _excFares.begin();
  vector<const PaxTypeFare*>::const_iterator excFaresIterTo = excFaresIterFrom;
  vector<const PaxTypeFare*>::const_iterator newFaresIterFrom = _newFares.begin();
  vector<const PaxTypeFare*>::const_iterator newFaresIterTo = newFaresIterFrom;

  if (findFCsCombinationWithSameFareBreaks(excFaresIterTo, newFaresIterTo))
  {
    MoneyAmount newAmount = countAmountForTheSameCurrency(newFaresIterFrom, newFaresIterTo);
    MoneyAmount excAmount = countAmountForTheSameCurrency(excFaresIterFrom, excFaresIterTo);

    if (excAmount > (newAmount + EPSILON))
    {
      displayDiagnostic(paxTypeFare, newAmount, excAmount);
      return false;
    }
  }

  return true;
}

bool
RexPaxTypeFareValidator::findFCsCombinationWithSameFareBreaks(
    std::vector<const PaxTypeFare*>::const_iterator& excFaresIter,
    std::vector<const PaxTypeFare*>::const_iterator& newFaresIter) const
{
  while (excFaresIter != _excFares.end() && newFaresIter != _newFares.end())
  {
    const PaxTypeFare& excPaxTypeFare = **excFaresIter;
    const PaxTypeFare& newPaxTypeFare = **newFaresIter;

    if (excPaxTypeFare.fareMarket()->travelSeg().back()->segmentOrder() ==
        newPaxTypeFare.fareMarket()->travelSeg().back()->segmentOrder())
    {
      ++excFaresIter;
      ++newFaresIter;
      return true;
    }
    if (excPaxTypeFare.fareMarket()->travelSeg().back()->segmentOrder() >
        newPaxTypeFare.fareMarket()->travelSeg().back()->segmentOrder())
      ++newFaresIter;
    else
      ++excFaresIter;
  }
  return false;
}

MoneyAmount
RexPaxTypeFareValidator::countAmountForTheSameCurrency(
    vector<const PaxTypeFare*>::const_iterator faresIterFrom,
    const vector<const PaxTypeFare*>::const_iterator& faresIterTo) const
{
  MoneyAmount result = 0;
  for (; faresIterFrom != faresIterTo; ++faresIterFrom)
    result += (*faresIterFrom)->nucFareAmount();
  return result;
}

bool
RexPaxTypeFareValidator::validate(const FarePath& farePath)
{
  if ((_excFares.empty() && !collectFullyFlownFares(_excFares, *_excItin.farePath().front())) ||
      (_newFares.empty() && !collectFullyFlownFares(_newFares, farePath)))
    return true;
  sort(_excFares.begin(), _excFares.end(), PaxTypeFareSegmentOrder());
  sort(_newFares.begin(), _newFares.end(), PaxTypeFareSegmentOrder());

  vector<const PaxTypeFare*>::const_iterator excFaresIterFrom = _excFares.begin();
  vector<const PaxTypeFare*>::const_iterator excFaresIterTo = excFaresIterFrom;
  vector<const PaxTypeFare*>::const_iterator newFaresIterFrom = _newFares.begin();
  vector<const PaxTypeFare*>::const_iterator newFaresIterTo = newFaresIterFrom;

  while (excFaresIterTo != _excFares.end() && newFaresIterTo != _newFares.end())
  {
    if (findFCsCombinationWithSameFareBreaks(excFaresIterTo, newFaresIterTo) &&
        (*excFaresIterFrom)->fareMarket()->travelSeg().back()->segmentOrder() !=
            (*newFaresIterFrom)->fareMarket()->travelSeg().back()->segmentOrder())
    {
      MoneyAmount newAmount = countAmountForTheSameCurrency(newFaresIterFrom, newFaresIterTo);
      MoneyAmount excAmount = countAmountForTheSameCurrency(excFaresIterFrom, excFaresIterTo);

      if (excAmount > (newAmount + EPSILON))
      {
        if (_dc)
        {
          *_dc << " FAILED: EXCHANGE " << (*excFaresIterFrom)->fareMarket()->boardMultiCity() << "-"
               << (*(excFaresIterTo - 1))->fareMarket()->offMultiCity()
               << " FARE AMOUNT: " << excAmount << " NUC IS GREATER THAN\n";
          *_dc << " REPRICE SOLUTION FARE AMOUNT: " << newAmount << " NUC\n";
        }
        return false;
      }
    }
    excFaresIterFrom = excFaresIterTo;
    newFaresIterFrom = newFaresIterTo;
  }

  return true;
}

DCFactory*
RexPaxTypeFareValidator::setupDiagnostic(Diag602Collector*& dc602)
{
  if (_rexTrx.diagnostic().diagnosticType() != Diagnostic602)
  {
    dc602 = nullptr;
    return nullptr;
  }

  DCFactory* factory = DCFactory::instance();
  dc602 = dynamic_cast<Diag602Collector*>(factory->create(_rexTrx));
  if (dc602)
    dc602->enable(Diagnostic602);

  return factory;
}

void
RexPaxTypeFareValidator::releaseDiagnostic(DCFactory* factory, Diag602Collector*& dc602)
{
  if (dc602 && factory)
  {
    dc602->flushMsg();
    dc602 = nullptr;
  }
}
}
