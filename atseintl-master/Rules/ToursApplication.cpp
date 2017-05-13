#include "Rules/ToursApplication.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag527Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleUtil.h"

using std::string;

namespace tse
{
namespace
{
ConfigurableValue<bool>
skipTourCodesValidation("SOLO_CARNIVAL_OPT", "SKIP_TOUR_CODES_VALIDATION", false);
}
ToursApplication::ToursApplication(PricingTrx* trx) : _trx(trx), _diag(nullptr)
{
  DCFactory* factory = DCFactory::instance();

  if (UNLIKELY(trx && _trx->diagnostic().diagnosticType() == Diagnostic527))
  {
    _diag = dynamic_cast<Diag527Collector*>(factory->create(*_trx));
    _diag->enable(Diagnostic527);
  }
}

ToursApplication::~ToursApplication()
{
  if (UNLIKELY(_diag != nullptr))
  {
    _diag->flushMsg();
  }
}

CarrierCode
ToursApplication::getCarrier(const PaxTypeFare* paxTypeFare) const
{
  CarrierCode carrier = paxTypeFare->carrier();
  if (carrier == INDUSTRY_CARRIER)
    carrier = paxTypeFare->fareMarket()->governingCarrier();
  return carrier;
}

Record3ReturnTypes
ToursApplication::validate(const vector<FareUsage*>& fareUsages)
{
  if (UNLIKELY(fareUsages.empty()))
    return PASS;

  if (UNLIKELY(_trx->getTrxType() == PricingTrx::IS_TRX &&
               _trx->getOptions()->isCarnivalSumOfLocal()))
  {
    if (skipTourCodesValidation.getValue())
      return PASS;
  }

  PaxTypeFare* firstFare = fareUsages.front()->paxTypeFare();
  bool skipValidation = isNegotiatedFareWithTourCode(firstFare);

  string tourCode;
  RuleUtil::getCat27TourCode(firstFare, tourCode);

  vector<FareUsage*>::const_iterator iterFU = fareUsages.begin() + 1;
  vector<FareUsage*>::const_iterator iterFUEnd = fareUsages.end();
  while (iterFU != iterFUEnd)
  {
    PaxTypeFare* paxTypeFare = (*iterFU)->paxTypeFare();
    if (!skipValidation)
      skipValidation = isNegotiatedFareWithTourCode(paxTypeFare);

    string nextTourCode;
    RuleUtil::getCat27TourCode(paxTypeFare, nextTourCode);

    if (!tourCode.empty())
    {
      if (!nextTourCode.empty())
      {
        if (nextTourCode != tourCode)
        {
          if (skipValidation)
            return SOFTPASS;
          return FAIL;
        }
      }
    }
    else
    {
      tourCode = nextTourCode;
    }

    ++iterFU;
  }

  return PASS;
}

Record3ReturnTypes
ToursApplication::validate(const PricingUnit* pricingUnit)
{
  if (!pricingUnit)
    return PASS;

  if (UNLIKELY(_diag))
  {
    _diag->displayPUHeader();
    _diag->displayPricingUnit(*pricingUnit);
  }

  bool cmdPricing = (const_cast<PricingUnit*>(pricingUnit))->isCmdPricing();
  string msg;

  Record3ReturnTypes result = FAIL;
  if (cmdPricing && !_diag)
    result = PASS;
  else
  {
    result = validate(pricingUnit->fareUsage());
    if (cmdPricing && result == FAIL)
      result = SOFTPASS; // need in diag
  }

  if (UNLIKELY(_diag))
  {
    _diag->printLine();
    _diag->displayStatus(result);
  }

  if (result == SOFTPASS)
    result = PASS;

  return result;
}

Record3ReturnTypes
ToursApplication::validate(const FarePath* farePath)
{
  if (UNLIKELY(!farePath))
    return PASS;

  bool isCmdPricing = false;
  if (UNLIKELY(_diag))
  {
    _diag->displayFPHeader();
    _diag->displayFarePath(*farePath);
  }

  vector<FareUsage*> allFareUsages;

  const vector<PricingUnit*>& pricingUnits = farePath->pricingUnit();
  vector<PricingUnit*>::const_iterator iterPU = pricingUnits.begin();
  vector<PricingUnit*>::const_iterator iterPUEnd = pricingUnits.end();
  while (iterPU != iterPUEnd)
  {
    if (UNLIKELY((*iterPU)->isCmdPricing()))
      isCmdPricing = true;

    const vector<FareUsage*>& fareUsages = (*iterPU)->fareUsage();
    vector<FareUsage*>::const_iterator iterFU = fareUsages.begin();
    vector<FareUsage*>::const_iterator iterFUEnd = fareUsages.end();
    while (iterFU != iterFUEnd)
    {
      allFareUsages.push_back(*iterFU);
      ++iterFU;
    }
    ++iterPU;
  }

  Record3ReturnTypes result = validate(allFareUsages);

  FarePath* fp = const_cast<FarePath*>(farePath);
  fp->setMultipleTourCodeWarning(result, isCmdPricing);

  if (UNLIKELY((_trx->getRequest()->ticketingAgent()->abacusUser() ||
                _trx->getRequest()->ticketingAgent()->infiniUser()) &&
               fp->multipleTourCodeWarning()))
    fp->tfrRestricted() = true;

  string msg;
  if (UNLIKELY(farePath->multipleTourCodeWarning()))
    result = SOFTPASS; // need in diag

  if (UNLIKELY(_diag))
  {
    _diag->displayStatus(result);
    _diag->printLine();
  }

  if (UNLIKELY(result == SOFTPASS))
    result = PASS;

  return result;
}

bool
ToursApplication::isNegotiatedFareWithTourCode(const PaxTypeFare* paxTypeFare) const
{
  if (!paxTypeFare->isNegotiated())
    return false;

  NegotiatedFareRuleUtil nfru;
  NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;

  const NegFareRest* negFareRest = nfru.getCat35Record3(paxTypeFare, negPaxTypeFare);

  if (LIKELY(negFareRest))
  {
    if (negFareRest->noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS &&
        (negFareRest->tourBoxCodeType1() != negFareRest->tourBoxCodeType2() ||
         negFareRest->tourBoxCode1() != negFareRest->tourBoxCode2()))
      return false;
    if (!negFareRest->tourBoxCode1().empty())
      return true;
  }
  return false;
}
}
