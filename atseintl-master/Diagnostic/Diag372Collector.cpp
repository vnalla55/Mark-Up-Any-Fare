#include "Diagnostic/Diag372Collector.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"

using namespace std;

namespace tse
{
const string Diag372Collector::DiagStream::empty = "***";
const size_t Diag372Collector::DiagStream::lineLength = 63;

template <class T>
void
Diag372Collector::DiagStream::addSet(const set<T>& inputSet, bool except, size_t margin)
{
  string str;
  bool multiline = false;

  if (inputSet.empty())
    str = empty;
  else
  {
    typename set<T>::const_iterator iter = inputSet.begin();
    typename set<T>::const_iterator iterEnd = inputSet.end();
    size_t spaceLeft = lineLength - margin - ((except) ? 7 : 0);

    if (except)
      str += "EXCEPT ";

    this->setf(ios::right, ios::adjustfield);

    do
    {
      if (spaceLeft >= iter->size())
      {
        str += *iter;

        spaceLeft -= iter->size();
        ++iter;
      }
      else
      {
        if (!multiline)
          (*this) << setw(lineLength - margin) << str << endl;
        else
          (*this) << str << endl;

        multiline = true;
        spaceLeft = lineLength;
        this->setf(ios::left, ios::adjustfield);
        str = "";
        continue;
      }

      if (spaceLeft >= 1)
      {
        if (iter != iterEnd)
          str += "/";

        --spaceLeft;
      }
      else
      {
        if (!multiline)
          (*this) << setw(lineLength - margin) << str << endl;
        else
          (*this) << str << endl;

        multiline = true;
        spaceLeft = lineLength;
        this->setf(ios::left, ios::adjustfield);
        str = "";
        continue;
      }

    } while (iter != iterEnd);
  }

  if (multiline && str.size() > 0)
    (*this) << str << endl;
  else if (str.size() > 0)
    (*this) << setw(lineLength - margin) << str << endl;
}

void
Diag372Collector::DiagStream::addSectorExemptionInfo(int count,
                                                     const SurfaceSectorExemptionInfo& info)
{
  Diag372Collector::DiagStream& diagStr = *this;

  diagStr << "SEQUENCE NUMBER " << setw(47) << count << endl << "VALIDATING CXR " << setw(48)
          << info.validatingCarrier() << endl << "CRS " << setw(59)
          << ((info.userAppl().empty()) ? empty : info.userAppl()) << endl;

  diagStr << "POS " << setw(59)
          << ((info.posLocException() == 'Y') ? "EXCEPT " : "") + info.posLoc() << endl;

  diagStr << "LOC 1 " << setw(57) << ((info.locException() == 'Y') ? "EXCEPT " : "") + info.loc1()
          << endl;

  diagStr << "LOC 2 " << setw(57) << ((info.locException() == 'Y') ? "EXCEPT " : "") + info.loc2()
          << endl;

  diagStr << "MARKETING CARRIER ";
  addSet(info.marketingCarriers(), info.exceptMarketingCarriers() == 'Y', 18);
  diagStr << endl;

  diagStr << "OPERATING CARRIER ";
  diagStr.addSet(info.operatingCarriers(), info.exceptOperatingCarriers() == 'Y', 18);
  diagStr << endl;

  diagStr << "PTC " << setw(4);
  diagStr.addSet(info.paxTypes(), info.exceptPassengersTypes() == 'Y', 4);
  diagStr << endl;

  diagStr << "CREATE DATE - EXPIRE DATE" << setw(38);
  diagStr.addDates(info.createDate(), info.expireDate());
  diagStr << endl;

  diagStr << "FIRST - LAST SALE DATE" << setw(41);
  diagStr.addDates(info.effDate(), info.discDate());
  diagStr << endl;
}

Diag372Collector& Diag372Collector::operator<<(const FarePath& farePath)
{
  if (!_active)
    return *this;

  DiagCollector& dc = *this;

  dc.setf(ios::right, ios::adjustfield);
  dc.setf(ios::fixed, ios::floatfield);
  dc.precision(2);

  int count = 0;

  vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
  vector<PricingUnit*>::const_iterator puIterEnd = farePath.pricingUnit().end();

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  Itin* itin = (*pricingTrx->itin().begin());

  vector<TravelSeg*>::const_iterator tsIter = itin->travelSeg().begin();
  vector<TravelSeg*>::const_iterator tsIterEnd = itin->travelSeg().end();

  for (; tsIter != tsIterEnd; ++tsIter)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*tsIter);

    if (airSeg != nullptr)
    {
      ++count;
      dc << " " << airSeg->segmentOrder() << ": " << airSeg->origin()->loc() << " "
         << airSeg->destination()->loc() << " ";
    }
    else
    {
      ArunkSeg* arunkSeg = dynamic_cast<ArunkSeg*>(*tsIter);
      if (arunkSeg != nullptr)
      {
        ++count;
        dc << " " << arunkSeg->segmentOrder() << ": "
           << "ARUNK"
           << " ";
      }
    }

    if (count % 4 == 0)
      dc << endl;
  }
  dc << endl;

  int puCount = 0;
  puIter = farePath.pricingUnit().begin();
  for (; puIter != puIterEnd; ++puIter)
  {
    ++puCount;

    const vector<FareUsage*>& fareUsage = (*puIter)->fareUsage();
    vector<FareUsage*>::const_iterator fuIter = fareUsage.begin();
    vector<FareUsage*>::const_iterator fuIterEnd = fareUsage.end();
    for (; fuIter != fuIterEnd; ++fuIter)
    {
      PaxTypeFare* fare = (*fuIter)->paxTypeFare();
      FareMarket* faraMarket = (*fuIter)->paxTypeFare()->fareMarket();

      string gd;
      globalDirectionToStr(gd, fare->globalDirection());

      dc.setf(ios::right, ios::adjustfield);
      dc << setw(4) << faraMarket->boardMultiCity() << "-" << setw(3) << faraMarket->offMultiCity()
         << setw(3) << fare->carrier() << setw(3) << gd << setw(2)
         << (fare->directionality() == FROM ? "O" : (fare->directionality() == TO ? "I" : " "));

      dc.setf(ios::left, ios::adjustfield);
      dc << setw(1) << " ";

      dc << setw(13) << DiagnosticUtil::getFareBasis(*fare);

      dc.setf(ios::right, ios::adjustfield);
      MoneyAmount nucFareAmount = fare->nucFareAmount();
      if (farePath.itin()->calculationCurrency() != "NUC")
      {
        PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
        if (pricingTrx)
        {
          CurrencyConversionFacade converter;
          Money source(fare->nucFareAmount(), fare->currency());
          Money target("NUC");
          converter.convert(target, source, *pricingTrx);
          nucFareAmount = target.value();
        }
      }
      dc << "NUC" << setw(8) << nucFareAmount;

      dc << " PU" << setfill('0') << setw(2) << puCount << setfill(' ');

      if ((*puIter)->puFareType() == PricingUnit::NL)
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

      if ((*fuIter)->isPaxTypeFareNormal())
        dc << "N/";
      else if ((*fuIter)->paxTypeFare()->isSpecial())
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
      dc << setw(2) << DiagnosticUtil::pricingUnitTypeToShortString((*puIter)->puType()) << endl;
    }

    if (puIter != farePath.pricingUnit().end() - 1)
      dc << " -------" << endl;
  }
  return *this;
}
}
