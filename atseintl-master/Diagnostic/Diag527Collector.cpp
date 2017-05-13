#include "Diagnostic/Diag527Collector.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Rules/RuleUtil.h"

#include <iomanip>
#include <iostream>
#include <vector>

namespace tse
{
void
Diag527Collector::displayFarePath(const FarePath& farePath)
{
  if (!_active)
    return;

  const std::vector<PricingUnit*>& pricingUnits = farePath.pricingUnit();
  std::vector<PricingUnit*>::const_iterator puIter = pricingUnits.begin();
  std::vector<PricingUnit*>::const_iterator puIterEnd = pricingUnits.end();
  while (puIter != puIterEnd)
  {
    PricingUnit* pricingUnit = *puIter;
    displayPricingUnit(*pricingUnit);
    ++puIter;
  }
}

void
Diag527Collector::displayPricingUnit(const PricingUnit& pricingUnit)
{
  if (!_active)
    return;

  const std::vector<FareUsage*>& fareUsages = pricingUnit.fareUsage();
  std::vector<FareUsage*>::const_iterator fuIter = fareUsages.begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = fareUsages.end();
  while (fuIter != fuIterEnd)
  {
    FareUsage* fareUsage = *fuIter;
    displayPaxTypeFare(*(fareUsage->paxTypeFare()), pricingUnit.puType());
    ++fuIter;
  }
}

void
Diag527Collector::getTourCode(const PaxTypeFare& paxTypeFare, string& tourCode)
{
  RuleUtil::getCat27TourCode(&paxTypeFare, tourCode);
}

void
Diag527Collector::getMoney(const PaxTypeFare& paxTypeFare, string& money)
{
  Money mn(paxTypeFare.fareAmount(), paxTypeFare.currency());
  money = mn.toString();
}

void
Diag527Collector::displayPaxTypeFare(const PaxTypeFare& paxTypeFare,
                                     PricingUnit::Type puType)
{
  if (!_active)
    return;

  string tourCode;
  getTourCode(paxTypeFare, tourCode);

  const FareMarket* fareMarket = paxTypeFare.fareMarket();

  *this << std::setw(3) << fareMarket->boardMultiCity() << " " << std::setw(3)
        << fareMarket->offMultiCity() << " " << std::setw(13)
        << DiagnosticUtil::getFareBasis(paxTypeFare) << " ";

  setf(std::ios::right, std::ios::adjustfield);
  setf(std::ios::fixed, std::ios::floatfield);

  string money;
  getMoney(paxTypeFare, money);

  *this << std::setw(8) << money << " " << std::setw(4) << paxTypeFare.vendor() << " "
        << std::setw(5) << paxTypeFare.tcrRuleTariff() << " " << std::setw(3)
        << paxTypeFare.carrier() << " " << std::setw(5) << paxTypeFare.ruleNumber() << " ";

  *this << std::setw(3);
  switch (puType)
  {
  case PricingUnit::Type::ROUNDTRIP:
    *this << " RT";
    break;
  case PricingUnit::Type::CIRCLETRIP:
    *this << " CT";
    break;
  case PricingUnit::Type::ONEWAY:
    *this << " OW";
    break;
  case PricingUnit::Type::OPENJAW:
    *this << " OJ";
    break;
  default:
    *this << "   ";
  }
  *this << std::endl << "TOUR CODE: " << tourCode << std::endl << std::endl;
}

void
Diag527Collector::displayHeader()
{
  if (!_active)
    return;

  *this << "***************************************************************" << std::endl
        << "*        ATSE CATEGORY 27 TOURS FARE PATH PROCESSING          *" << std::endl
        << "***************************************************************" << std::endl;
}

void
Diag527Collector::displayPUHeader()
{
  if (!_active)
    return;

  *this << "PRICING UNIT:" << std::endl;
}

void
Diag527Collector::displayFPHeader()
{
  if (!_active)
    return;

  *this << "FARE PATH:" << std::endl;
}

void
Diag527Collector::displayStatus(Record3ReturnTypes status)
{
  if (!_active)
    return;

  string state;
  switch (status)
  {
  case PASS:
    state = "PASS";
    break;
  case SOFTPASS:
    state = "SOFTPASS";
    break;
  case FAIL:
    state = "FAIL";
    break;
  default:
    state = "UNK";
    break;
  }
  *this << "TOUR CODE COMBINATION: " << state << std::endl;
}
}
