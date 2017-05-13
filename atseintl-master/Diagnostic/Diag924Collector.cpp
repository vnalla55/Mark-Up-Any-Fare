#include "Diagnostic/Diag924Collector.h"

#include "Common/Money.h"
#include "Common/ShpqTypes.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/CxrFareMarkets.h"
#include "DataModel/DirFMPath.h"
#include "DataModel/DirFMPathList.h"
#include "DataModel/DirFMPathListCollector.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/SoloFmPath.h"

#include <string>

namespace tse
{
namespace
{
std::string
strSolutionType(shpq::SolutionType sType)
{
  if (sType == shpq::OW)
    return std::string("OW");
  if (sType == shpq::HRT)
    return std::string("HRT");
  if (sType == shpq::OW_OW)
    return std::string("OW-OW");
  if (sType == shpq::OW_HRT)
    return std::string("OW-HRT");
  if (sType == shpq::HRT_HRT)
    return std::string("HRT-HRT");
  if (sType == shpq::HRT_OW)
    return std::string("HRT-OW");
  return std::string("");
}
}

Diag924Collector& Diag924Collector::operator<<(const shpq::SoloFmPath& soloFmPath)
{
  Diag924Collector& dc = *this;
  if (_active)
  {
    dc << "***************************************************" << std::endl;
    dc << " 924: LOWEST FARE AMOUNT PER SOLUTION PATTERN" << std::endl << std::endl;
    dc << " Solution patterns: " << std::endl;
    dc << " OW       : one way" << std::endl;
    dc << " HRT      : half round trip" << std::endl;
    dc << " OW-OW    : one way - one way" << std::endl;
    dc << " OW-HRT   : one way - half round trip" << std::endl;
    dc << " HRT-OW   : half round trip - one way" << std::endl;
    dc << " HRT-HRT  : half round trip - half round trip" << std::endl;
    dc << "***************************************************" << std::endl;
    for (shpq::SoloFmPath::size_type ind = 0, size = soloFmPath.size(); ind < size; ++ind)
    {
      dc << " LEG " << ind + 1 << " OF " << size << std::endl;

      shpq::DirFMPathListCollectorPtr leg = soloFmPath[ind];
      shpq::DirFMPathListCollector::const_iterator it = leg->begin(), endIt = leg->end();

      uint16_t solutionNo = 1;
      uint16_t totalSolutionNo = leg->size();
      for (; it != endIt; ++it)
      {
        shpq::SolutionType sType = it->first;
        dc << "************* SOLUTION PATTERN: " << std::setw(7) << strSolutionType(sType) << "  ("
           << solutionNo++ << " of " << totalSolutionNo << ")"
           << " *************" << std::endl;

        dc << "TOTAL AMOUNT: " << Money(it->second->lowerBound(), NUC) << std::endl;
        dc << *it->second;
      }
    }
  }
  return dc;
}

Diag924Collector& Diag924Collector::operator<<(const shpq::DirFMPathList& dirFmPathList)
{
  Diag924Collector& dc = *this;
  if (_active)
  {
    shpq::DirFMPathPtr dirFmPath = *dirFmPathList.begin();

    for (auto cxrFareMarkets : *dirFmPath)
    {
      shpq::OwrtFareMarketPtr owrtFareMarket = *cxrFareMarkets->begin();
      PaxTypeFare* paxFare = *owrtFareMarket->begin();
      FareMarket* fm = owrtFareMarket->getFareMarket();
      dc << fm->origin()->loc() << " - " << fm->destination()->loc() << std::endl;
      dc << "Travel date: " << fm->travelDate().dateToString(DDMMMYY, "") << std::endl;
      dc << "Marketing carrier: " << fm->governingCarrier() << std::endl;
      dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX\n";
      dc << "                            NUM R I              TYP TYP\n";
      dc << "- -- - ---- --------------- --- - - -------- --- --- ---\n";
      dc << *paxFare;
      dc << '\n';
    }
  }
  return dc;
}

Diag924Collector& Diag924Collector::operator<<(const PaxTypeFare& paxFare)
{
  Diag924Collector& dc = *this;
  if (_active)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << cnvFlags(paxFare);

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor())
       << std::setw(5) << paxFare.ruleNumber();

    std::string fareBasis("");
    if (_shoppingTrx)
      fareBasis = paxFare.createFareBasis(*_shoppingTrx, false);
    if (fareBasis.size() > 15)
      fareBasis = fareBasis.substr(0, 15) + "*";
    dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";
    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";
    else
      dc << "  ";

    dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

    if (!paxFare.isFareClassAppMissing())
      dc << std::setw(4) << paxFare.fcaFareType();
    else
      dc << "UNK ";

    if (!paxFare.isFareClassAppSegMissing())
    {
      if (paxFare.fcasPaxType().empty())
        dc << "*** ";
      else
        dc << std::setw(4) << paxFare.fcasPaxType();
    }
    else
      dc << "UNK ";

    if (paxFare.isKeepForRoutingValidation())
      dc << " SR";
    dc << '\n';
    dc << std::setw(36) << " ";
    dc << std::setw(8) << Money(paxFare.nucFareAmount(), NUC);
    dc << '\n';
  }
  return dc;
}

} // namespace tse::shpq
