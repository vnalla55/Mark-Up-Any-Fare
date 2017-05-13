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
#include "Diagnostic/Diag614Collector.h"

#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
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
Diag614Collector::Diag614Collector(Diagnostic& root)
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
Diag614Collector::initParam(Diagnostic& root)
{
  _rootDiag = &root;
  _farePathNumber = 0;
  _puNumber = 0;
  _fcNumber = 0;
  _currentFarePathNumber = 0;
  _currentPricingUnitNumber = 0;
  _currentFareUsageNumber = 0;

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
Diag614Collector::enableFilter(DiagnosticTypes diagType, int dtCount, size_t currentFarePathNumber)
{
  if (_rootDiag->isActive())
  {
    if (_farePathNumber != 0)
      _currentFarePathNumber = currentFarePathNumber;

    return (_diagnosticType == diagType) && _currentFarePathNumber == _farePathNumber;
  }
  return false;
}

bool
Diag614Collector::enableFilter(DiagnosticTypes diagType,
                               uint32_t farePathNumber,
                               uint32_t pricingUnitNumber,
                               uint32_t fareUsageNumber)
{
  if (_rootDiag->isActive())
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
    return ((_diagnosticType == diagType) && (_currentFarePathNumber == farePathNumber) &&
            (_currentPricingUnitNumber == pricingUnitNumber) &&
            (_currentFareUsageNumber == fareUsageNumber));
  }
  return false;
}

Diag614Collector& Diag614Collector::operator<< (std::ostream& ( *pf )(std::ostream&))
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << pf;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (char ch)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << ch;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (int i)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << i;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (long l)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << l;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (float f)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << f;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (double d)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << d;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (uint16_t u)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (uint32_t u)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (uint64_t u)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

Diag614Collector& Diag614Collector::operator<< (const char *msg)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    if (msg)
    {
      ((std::ostringstream&)*this) << msg;
    }
  }
  return *this;
}

Diag614Collector & Diag614Collector::operator<< (const std::string &msg)
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this) << msg;
  }
  return *this;
}

void
Diag614Collector::printHeader()
{
  if (_active)
  {
    ((std::ostringstream&)*this)
        << "******************** END ON END LIST DISPLAY ******************\n";
  }
}

void
Diag614Collector::printLine()
{
  if (_active && (!_farePathNumber || (_farePathNumber == _currentFarePathNumber)))
  {
    ((std::ostringstream&)*this)
        << "***************************************************************\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag614Collector::operator <<
//
// Description:  override base operator << to handle diagnostic 614 display
//
// </PRE>
// ----------------------------------------------------------------------------
Diag614Collector&
Diag614Collector::operator<<(const FarePath& farePath)
{
  if (_active && (_farePathNumber == 0 || (_farePathNumber == _currentFarePathNumber)))
  {
    DiagCollector& dc = *this;
    std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();

    if(! farePath.validatingCarriers().empty())
    {
      dc << " VALIDATING CXR: ";
      for (CarrierCode valCxr : farePath.validatingCarriers())
        dc << valCxr << "  ";
      dc << std::endl;
    }

    for (; puIter != farePath.pricingUnit().end(); ++puIter)
    {
      std::vector<FareUsage*>::const_iterator fuIter = (*puIter)->fareUsage().begin();
      size_t i = 1;

      for (; fuIter != (*puIter)->fareUsage().end(); ++fuIter, ++i)
      {
        dc << std::setw(3) << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->boardMultiCity()
           << "-" << std::setw(2) << (*fuIter)->paxTypeFare()->fareMarket()->governingCarrier()
           << "-" << std::setw(3)
           << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->offMultiCity() << "  ";

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

          dc << std::setw(3)
             << ((*puIter)->puType() == PricingUnit::Type::OPENJAW
                     ? "101"
                     : ((*puIter)->puType() == PricingUnit::Type::ROUNDTRIP
                            ? "102"
                            : ((*puIter)->puType() == PricingUnit::Type::CIRCLETRIP
                                   ? "103"
                                   : ((*puIter)->puType() == PricingUnit::Type::ONEWAY ? "104" : " "))));

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
                 << (*fuIter)->rec2Cat10()->dojFareClassTypeRestInd() << "\n";
            }
            else if ((*puIter)->puType() == PricingUnit::Type::ROUNDTRIP)
            {
              dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->ct2Ind() << "      "
                 << std::setw(2) << (*fuIter)->rec2Cat10()->ct2CarrierRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->ct2TariffRuleRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->ct2FareClassTypeRestInd() << "\n";
            }
            else if ((*puIter)->puType() == PricingUnit::Type::CIRCLETRIP)
            {
              dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->ct2plusInd() << "      "
                 << std::setw(2) << (*fuIter)->rec2Cat10()->ct2plusCarrierRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->ct2plusTariffRuleRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->ct2plusFareClassTypeRestInd() << "\n";
            }
            else if ((*puIter)->puType() == PricingUnit::Type::ONEWAY)
            {
              dc << " " << std::setw(2) << (*fuIter)->rec2Cat10()->eoeInd() << "      "
                 << std::setw(2) << (*fuIter)->rec2Cat10()->eoeCarrierRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->eoeTariffRuleRestInd() << std::setw(2)
                 << (*fuIter)->rec2Cat10()->eoeFareClassTypeRestInd() << "\n";
            }
            else
            {
              dc << "\n";
            }
          } // if (*fuIter)
          else
          {
            dc << "\n";
          }
        } // if fare
      } // for fuIter
    } // for puIter
  }
  return *this;
}
}
