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
#include "Diagnostic/Diag620Collector.h"

#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>
#include <vector>

namespace tse
{
Diag620Collector::Diag620Collector(Diagnostic& root)
  : DiagCollector(root),
    _farePathNumber(0),
    _puNumber(0),
    _fcNumber(0),
    _currentFarePathNumber(0),
    _currentPricingUnitNumber(0),
    _currentFareUsageNumber(0)
{
  initParam(root);
}

void
Diag620Collector::initParam(Diagnostic& root)
{
  _rootDiag = &root;
  _farePathNumber = 0;
  _puNumber = 0;
  _fcNumber = 0;
  _currentFarePathNumber = 0;

  DiagParamMap::iterator itEnd = root.diagParamMap().end();
  DiagParamMap::iterator it = root.diagParamMap().find("FP");
  if (it != itEnd)
    _farePathNumber = atoi(it->second.c_str());

  it = root.diagParamMap().find("PU");
  if (it != itEnd)
    _puNumber = atoi(it->second.c_str());

  it = root.diagParamMap().find("FN");
  if (it != itEnd)
    _fcNumber = atoi(it->second.c_str());
}

bool
Diag620Collector::enableFilter(DiagnosticTypes diagType, int dtCount, size_t currentFarePathNumber)
{
  if (!_rootDiag->isActive())
    return false;

  if (_diagnosticType != diagType)
    return false;

  if (_farePathNumber == 0)
    return true;

  _currentFarePathNumber = currentFarePathNumber;

  return true;
}

bool
Diag620Collector::enableFilter(DiagnosticTypes diagType,
                               uint32_t farePathNumber,
                               uint32_t pricingUnitNumber,
                               uint32_t fareUsageNumber)
{
  if (_rootDiag->isActive() && _diagnosticType == diagType)
  {
    if (_farePathNumber != 0)
    {
      _currentFarePathNumber = farePathNumber;
    }
    if (_puNumber != 0)
    {
      _currentPricingUnitNumber = pricingUnitNumber;
    }
    if (_fcNumber != 0)
    {
      _currentFareUsageNumber = fareUsageNumber;
    }
    return ((_currentFarePathNumber == farePathNumber) &&
            (_currentPricingUnitNumber == pricingUnitNumber) &&
            (_currentFareUsageNumber == fareUsageNumber));
  }
  return false;
}

Diag620Collector& Diag620Collector::operator<< (std::ostream& ( *pf )(std::ostream&))
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << pf;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (char ch)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << ch;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (int i)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << i;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (long l)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << l;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (float f)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << f;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (double d)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << d;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (uint16_t u)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << u;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (uint32_t u)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << u;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (uint64_t u)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << u;

  return *this;
}

Diag620Collector& Diag620Collector::operator<< (const char *msg)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    if (msg)
      ((std::ostringstream&)*this) << msg;

  return *this;
}

Diag620Collector & Diag620Collector::operator<< (const std::string &msg)
{
  if (!_active)
    return *this;

  if (_farePathNumber == _currentFarePathNumber)
    ((std::ostringstream&)*this) << msg;

  return *this;
}

void
Diag620Collector::printHeader()
{
  if (!_active)
    return;

  if (_farePathNumber == 0)
  {
    ((std::ostringstream&)*this)
        << "************** SCOREBOARD COMBINATION ANALYSIS ****************\n";
  }
  else
  {
    ((std::ostringstream&)*this)
        << "*********** COMBINABILITY CATEGORY CONTROL RECORD *************\n";
  }
}

void
Diag620Collector::printLine()
{
  if (!_active)
    return;

  if (_farePathNumber == _currentFarePathNumber)
  {
    ((std::ostringstream&)*this)
        << "***************************************************************\n";
  }
}

Diag620Collector&
Diag620Collector::operator << ( const FarePath& farePath )
{
  if (!_active)
    return *this;

  if (_farePathNumber == 0)
    collect620(farePath);
  else if (_farePathNumber == _currentFarePathNumber)
    collect620MX(farePath);

  PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);
  if (trx && trx->getTrxType() == PricingTrx::MIP_TRX && trx->isFlexFare())
    (*this) << "\nFLEX FARES GROUP ID: " << farePath.getFlexFaresGroupId() << "\n";

  return *this;
}

void
Diag620Collector::collect620(const FarePath& farePath)
{
  // DiagCollector& dc( *this );
  DiagCollector& dc = *this;
  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();

  for (; puIter != farePath.pricingUnit().end(); ++puIter)
  {
    std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
    size_t i = 1;

    for (; fuIter != (*puIter)->fareUsage().end(); ++fuIter, ++i)
    {
      dc << std::setw(3) << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->boardMultiCity()
         << "-" << std::setw(2) << (*fuIter)->paxTypeFare()->fareMarket()->governingCarrier() << "-"
         << std::setw(3) << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->offMultiCity()
         << "  ";

      if (i % 4 == 0)
      {
        dc << "\n";
      }
    } // for fuIter

    if (i % 4 != 0)
    {
      dc << "\n";
    }

    fuIter = (*puIter)->fareUsage().begin();

    for (; fuIter != (*puIter)->fareUsage().end(); ++fuIter)
    {
      PaxTypeFare* fare = (*fuIter)->paxTypeFare();

      if (fare)
      {
        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << " " << std::setw(13) << DiagnosticUtil::getFareBasis(*fare) << " " << std::setw(4) << fare->vendor()
           << std::setw(4) << fare->tcrRuleTariff() << std::setw(3) << fare->carrier()
           << std::setw(5) << fare->ruleNumber() << std::setw(4)
           << (*fuIter)->paxTypeFare()->fcaFareType() << std::setw(2) << fare->owrt()
           << std::setw(1)
           << (fare->directionality() == FROM ? "O" : fare->directionality() == TO ? "I" : " ");

        dc.setf(std::ios::right, std::ios::adjustfield);
        if ((*fuIter) && (*fuIter)->rec2Cat10())
          dc << std::setw(8) << (*fuIter)->rec2Cat10()->sequenceNumber();
        else
          dc << "        ";

        if ((*fuIter) && (*fuIter)->rec2Cat10())
        {
          dc.setf(std::ios::left, std::ios::adjustfield);

          if ((*puIter)->puType() == PricingUnit::Type::OPENJAW)
          {
            dc << "   " << std::setw(2) << (*fuIter)->rec2Cat10()->sojInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->sojorigIndestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->dojInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->dojCarrierRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->dojTariffRuleRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->dojFareClassTypeRestInd();
          }
          else if ((*puIter)->puType() == PricingUnit::Type::ROUNDTRIP)
          {
            dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->ct2Ind() << "      "
               << std::setw(2) << (*fuIter)->rec2Cat10()->ct2CarrierRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->ct2TariffRuleRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->ct2FareClassTypeRestInd();
          }
          else if ((*puIter)->puType() == PricingUnit::Type::CIRCLETRIP)
          {
            dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->ct2plusInd() << "      "
               << std::setw(2) << (*fuIter)->rec2Cat10()->ct2plusCarrierRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->ct2plusTariffRuleRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->ct2plusFareClassTypeRestInd();
          }
          else if ((*puIter)->puType() == PricingUnit::Type::ONEWAY)
          {
            dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->eoeInd() << "      "
               << std::setw(2) << (*fuIter)->rec2Cat10()->eoeCarrierRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->eoeTariffRuleRestInd() << std::setw(2)
               << (*fuIter)->rec2Cat10()->eoeFareClassTypeRestInd();
          }
        } // if (*fuIter)
        dc << std::setw(3)
           << ((*puIter)->puType() == PricingUnit::Type::OPENJAW
                   ? "101"
                   : ((*puIter)->puType() == PricingUnit::Type::ROUNDTRIP
                          ? "102"
                          : ((*puIter)->puType() == PricingUnit::Type::CIRCLETRIP
                                 ? "103"
                                 : ((*puIter)->puType() == PricingUnit::Type::ONEWAY ? "104" : " "))));

        dc << "\n";
      } // if fare
    } // for fuIter
  } // for puIter
}

void
Diag620Collector::collect620MX(const FarePath& farePath)
{
  DiagCollector& dc(*this);
  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();

  for (size_t i = 1; puIter != farePath.pricingUnit().end(); ++puIter, ++i)
  {
    if (i != _puNumber)
      continue;

    std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
    for (size_t j = 1; fuIter != (*puIter)->fareUsage().end(); ++fuIter, ++j)
    {

      if (j != _fcNumber)
        continue;

      PaxTypeFare* fare = (*fuIter)->paxTypeFare();
      CombinabilityRuleInfo* pCat10 = (*fuIter)->rec2Cat10();

      if (fare == 0)
      {
        dc << "******** NO FARE TO DISPLAY ********\n";
        break;
      }

      dc.setf(std::ios::left, std::ios::adjustfield);
      dc << "FROM-" << std::setw(4) << fare->origin() << "TO-" << std::setw(4)
         << fare->destination() << "CARRIER-" << std::setw(4) << fare->carrier() << "RULE "
         << std::setw(4) << fare->ruleNumber() << "-" << std::setw(7) << fare->tcrRuleTariffCode()
         << "\n"
         << "FARE BASIS-" << std::setw(13) << fare->createFareBasis(0) << std::setw(2)
         << (fare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "RT" : "OW") << "-" << std::setw(5)
         << (*fuIter)->paxTypeFare()->fcaFareType() << std::setw(5)
         << (((*fuIter)->isPaxTypeFareNormal()) ? "NORMAL FARE" : "SPECIAL FARE") << "\n\n";

      if ((*fuIter)->rec2Cat10() == 0)
      {
        dc << "            ******** NO RECORD 2 CATEGORY 10 ********\n";
        break;
      }

      tools::printCat10Info(dc, pCat10);

      break;

    } // for fuIter

    break;

  } // for puIter
}
}
