//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "Diagnostic/Diag660Collector.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"

#include <iomanip>

namespace tse
{
void
Diag660Collector::printHeader()
{
  if (_active)
   ((DiagCollector&)*this) << "********************* PRICING UNIT ANALYSIS *******************\n";
}

Diag660Collector& Diag660Collector::operator<<(const FarePath& fPath)
{
  if (!_active)
    return *this;

  DiagCollector& dc = *this;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);

  uint16_t count = 0;
  for (const PricingUnit* pu : fPath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pu->fareUsage())
    {
      const FareMarket* fm = fareUsage->paxTypeFare()->fareMarket();
      dc << " "
         << fPath.itin()->segmentOrder(fm->travelSeg().front()) << "--"
         << fPath.itin()->segmentOrder(fm->travelSeg().back()) << ":" << fm->boardMultiCity() << " "
         << fm->offMultiCity() << " ";

      if (++count % 4 == 0)
        dc << "\n";
    }
  }
  dc << "\n";

  uint16_t puCount = 0;
  for (const PricingUnit* pu : fPath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pu->fareUsage())
    {
      const PaxTypeFare* fare = fareUsage->paxTypeFare();
      const FareMarket* fm = fare->fareMarket();

      std::string gd;
      globalDirectionToStr(gd, fare->globalDirection());

      dc.setf(std::ios::right, std::ios::adjustfield);
      dc << std::setw(4) << fm->boardMultiCity() << "-" << std::setw(3) << fm->offMultiCity()
         << std::setw(3) << fare->carrier() << std::setw(3) << gd << std::setw(2)
         << (fare->directionality() == FROM ? "O" : (fare->directionality() == TO ? "I" : " "));

      dc.setf(std::ios::left, std::ios::adjustfield);
      dc << std::setw(1) << " ";

      dc << std::setw(13) << DiagnosticUtil::getFareBasis(*fare);

      dc.setf(std::ios::right, std::ios::adjustfield);
      MoneyAmount nucFareAmount = fare->nucFareAmount();
      PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
      if (fPath.itin()->calculationCurrency() != "NUC")
      {
        CurrencyConversionFacade converter;
        Money source(fare->nucFareAmount(), fare->currency());
        Money target("NUC");
        converter.convert(target, source, *pricingTrx);
        nucFareAmount = target.value();
      }
      dc << "NUC" << std::setw(8) << nucFareAmount;
      dc << " PU" << std::setfill('0') << std::setw(2) << ++puCount << std::setfill(' ');

      if (pu->puFareType() == PricingUnit::NL)
        dc << "/NORM/";
      else
        dc << "/SPCL/";

      if (fare->owrt() == ONE_WAY_MAY_BE_DOUBLED) // Tag 1
        dc << "TAG1/";
      else if (fare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) // Tag 2
        dc << "TAG2/";
      else if (fare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) // Tag 3
        dc << "TAG3/";
      else
        dc << "    /";

      if (fareUsage->isPaxTypeFareNormal())
        dc << "N/";
      else if (fareUsage->paxTypeFare()->isSpecial())
        dc << "S/";
      else
        dc << " /";

      if (fare->isInternational())
        dc << "I";
      else if (fare->isDomestic())
        dc << "D";
      else if (fare->isTransborder())
        dc << "T";
      else if (fare->isForeignDomestic())
        dc << "F";
      else
        dc << " ";

      dc << " ";
      dc << std::setw(2) << DiagnosticUtil::pricingUnitTypeToShortString(pu->puType())
         << "\n";
    }

    if (pu != fPath.pricingUnit().back())
      dc << " -------\n";
  }
  return *this;
}
}
