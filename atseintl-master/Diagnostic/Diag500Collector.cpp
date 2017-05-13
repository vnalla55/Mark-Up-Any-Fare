//----------------------------------------------------------------------------
//  File:        Diag500Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 500
//
//  Updates:
//          04/23/2004  VK - create.
//
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

#include "Diagnostic/Diag500Collector.h"

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"

#include <iomanip>
#include <string>

using namespace std;

namespace tse
{
const string Diag500Collector::SHORT_OUTPUT = "PH";
const string Diag500Collector::PREVALIDATION = "PREV";
const string Diag500Collector::NORMAL_OR_FARECOMP_VALIDATION = "FCV";
const string Diag500Collector::REVALIDATION = "REV";
const string Diag500Collector::DYNAMIC_VALIDATION = "DV";
const string Diag500Collector::SPECIFIC_CATEGORY = "RL";

Diag500Collector::ValidationPhase Diag500Collector::_givenValidationPhase = Diag500Collector::ANY;
Diag500Collector::ValidationPhase Diag500Collector::_validationPhase = Diag500Collector::ANY;

uint16_t Diag500Collector::_categoryNumber = 0;

void
Diag500Collector::printHeader()
{
  if (_active)
  {
    ((DiagCollector&)*this)

        //       123456789012345678901234567890123456789012345678901234567890123
        << "---------------------------------------------------------------\n"
        << "PAX MARKET FARE     FARE     O VND CXR TRF RULE  SEQ    SEG RUL\n"
        << "           CLASS    BASIS    I         NBR NBR   NBR    CNT CAT\n"
        << "---------------------------------------------------------------\n";
  }
}

void
Diag500Collector::printRulePhase(const CategoryPhase phase)
{
  if (_active)
  {
    (*this) << "RULE PHASE: ";

    switch(phase)
    {
    case NormalValidation:
      (*this) << "FARE COMPONENT VALIDATION\n";
      break;

    case PreValidation:
      (*this) << "PRE-VALIDATION\n";
      break;

    case PreCombinabilityValidation:
      (*this) << "PRE-COMBINABILITY VALIDATION\n";
      break;

    case PURuleValidation:
      (*this) << "PU FACTORY RULE VALIDATION\n";
      break;

    case FPRuleValidation:
      (*this) << "FAREPATH FACTORY RULE VALIDATION\n";
      break;

    case DynamicValidation:
      (*this) << "DYNAMIC-VALIDATION\n";
      break;

    case ShoppingComponentValidation:
      (*this) << "SHOPPING-COMPONENT-VALIDATION\n";
      break;

    case ShoppingComponentWithFlightsValidation:
      (*this) << "SHOPPING-COMPONENT-WITH-FLIGHTS-VALIDATION\n";
      break;

    case FootNotePrevalidation:
    case FootNotePrevalidationALT:
      (*this) << "FOOTNOTE PREVALIDATION\n";
      break;

    case LoadRecords:
      (*this) << "LOAD RECORDS\n";
      break;

    case FBRBaseFarePrevalidation:
      (*this) << "FBR BASE FARE PREVALIDATION\n";
      break;

    default:
      (*this) << "UNKNOWN RULE CONTROLLER PHASE\n";
    }
  }
}

void
Diag500Collector::diag500Collector(const PaxTypeFare& paxFare,
                                   const GeneralFareRuleInfo* rule,
                                   const bool fareRule)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isValid())
    {
      dc << setw(3) << paxFare.paxType()->paxType();
      dc << ":";
    }
    else
    {
      if (paxFare.isFareClassAppMissing())
      {
        dc << "-R1:";
      }
      else if (paxFare.isFareClassAppSegMissing())
      {
        dc << "-1B:";
      }
      else if (paxFare.isRoutingProcessed() && !(paxFare.isRoutingValid()))
      {
        dc << "-RT:";
      }
      else if (paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        dc << "-BK:";
      }
      else if (!paxFare.areAllCategoryValid())
      {
        if (paxFare.paxType()) // to fix core dump
        {
          dc << setw(3) << paxFare.paxType()->paxType();
        }
        else
        {
          dc << setw(3) << "   ";
        }
        dc << "*";
      }
      else
      {
        dc << "-XX:";
      }
    }

    dc << setw(3) << paxFare.fareMarket()->origin()->loc();
    dc << setw(4) << paxFare.fareMarket()->destination()->loc();
    FareClassCode fareClass = paxFare.fareClass();

    if (fareClass.size() > 7)
      fareClass = fareClass.substr(0, 7) + "*"; // Cross-of-lorraine?
    dc << std::setw(8) << fareClass << " ";

    std::string fareBasis = paxFare.createFareBasis(nullptr);

    if (fareBasis.size() > 7)
      fareBasis = fareBasis.substr(0, 7) + "*"; // Cross-of-lorraine?
    dc << std::setw(8) << fareBasis << " ";

    if (paxFare.directionality() == FROM)
      dc << setw(2) << "O";
    else if (paxFare.directionality() == TO)
      dc << setw(2) << "I";
    else
      dc << setw(2) << "-";

    dc << setw(5) << rule->vendorCode();
    dc << setw(3) << rule->carrierCode();
    dc << setw(5) << rule->tariffNumber();
    dc << setw(5) << rule->ruleNumber();

    dc << setw(8) << rule->sequenceNumber();
    dc << setw(2) << rule->segcount();
    if (fareRule)
    {
      dc << " R";
    }
    else
      dc << " G";
    dc << setw(2) << rule->categoryNumber();
    dc << '\n';

    displayDates(paxFare);
  }
}

void
Diag500Collector::displayDates(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx());
    if (pricingTrx != nullptr)
    {
      std::map<std::string, std::string>::const_iterator i =
          pricingTrx->diagnostic().diagParamMap().find("DD");
      if ((i != pricingTrx->diagnostic().diagParamMap().end()) && (i->second == "DATE"))
      {
        displayFareDates(paxFare);
      }
    }

    RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(trx());
    if ((rexTrx != nullptr) && (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE))
    {
      displayRetrievalDate(paxFare);
      dc << "\n";
    }
  }
}

void
Diag500Collector::diag500Collector(Record3ReturnTypes statusRule)
{
  if (_active)
  {

    DiagCollector& dc(*this);

    if (statusRule == PASS)
    {
      dc << "                                              ---PASS--- ";
    }
    else if (statusRule == SKIP)
    {
      dc << "                                              ---SKIP--- ";
    }
    else if (statusRule == FAIL)
    {
      dc << "                                              ---FAIL--- ";
    }
    else if (statusRule == SOFTPASS)
    {
      dc << "                                              ---SOFT--- ";
    }
    else if (statusRule == STOP)
    {
      dc << "                                              ---FAIL--- ";
    }
    else if (statusRule == STOP_SOFT)
    {
      dc << "                                              ---SOFT--- ";
    }
    else
    {
      dc << "                                              ---XXXX--- ";
    }
    dc << "\n---------------------------------------------------------\n";
  }
}

void
Diag500Collector::diag500Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo* rule)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isValid())
    {
      dc << setw(3) << paxFare.paxType()->paxType();
      dc << ":";
    }
    else
    {
      if (paxFare.isFareClassAppMissing())
      {
        dc << "-R1:";
      }
      else if (paxFare.isFareClassAppSegMissing())
      {
        dc << "-1B:";
      }
      else if (paxFare.isRoutingProcessed() && !(paxFare.isRoutingValid()))
      {
        dc << "-RT:";
      }
      else if (paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        dc << "-BK:";
      }
      else if (!paxFare.areAllCategoryValid())
      {
        if (paxFare.paxType()) // to fix core dump
        {
          dc << setw(3) << paxFare.paxType()->paxType();
        }
        else
        {
          dc << setw(3) << "   ";
        }
        dc << "*";
      }
      else
      {
        dc << "-XX:";
      }
    }

    dc << setw(3) << paxFare.fareMarket()->origin()->loc();
    dc << setw(4) << paxFare.fareMarket()->destination()->loc();
    dc << setw(9) << paxFare.fare()->fareClass();

    if (paxFare.directionality() == FROM)
      dc << setw(2) << "O";
    else if (paxFare.directionality() == TO)
      dc << setw(2) << "I";
    else
      dc << setw(2) << "-";

    dc << "R2:";
    dc << setw(5) << paxFare.vendor();
    dc << setw(3) << paxFare.fare()->carrier();
    dc << setw(5) << rule->fareTariff();
    dc << setw(5) << rule->footNote();
    dc << setw(8) << rule->sequenceNumber();
    dc << setw(3) << rule->segcount();
    dc << " F";
    dc << setw(2) << rule->categoryNumber();
    dc << '\n';

    displayDates(paxFare);
  }
}

void
Diag500Collector::diag500Collector(const PaxTypeFare& paxFare, const FareByRuleCtrlInfo* rule)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (paxFare.isValid())
    {
      dc << setw(3) << paxFare.paxType()->paxType();
      dc << ":";
    }
    else
    {
      if (paxFare.isFareClassAppMissing())
      {
        dc << "-R1:";
      }
      else if (paxFare.isFareClassAppSegMissing())
      {
        dc << "-1B:";
      }
      else if (paxFare.isRoutingProcessed() && !(paxFare.isRoutingValid()))
      {
        dc << "-RT:";
      }
      else if (paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
      {
        dc << "-BK:";
      }
      else if (!paxFare.areAllCategoryValid())
      {
        if (paxFare.paxType()) // to fix core dump
        {
          dc << setw(3) << paxFare.paxType()->paxType();
        }
        else
        {
          dc << setw(3) << "   ";
        }
        dc << "*";
      }
      else
      {
        dc << "-XX:";
      }
    }

    dc << setw(3) << paxFare.fareMarket()->origin()->loc();
    dc << setw(4) << paxFare.fareMarket()->destination()->loc();
    dc << setw(9) << paxFare.fare()->fareClass();

    if (paxFare.directionality() == FROM)
      dc << setw(2) << "O";
    else if (paxFare.directionality() == TO)
      dc << setw(2) << "I";
    else
      dc << setw(2) << "-";

    dc << "R2:";
    dc << setw(5) << paxFare.vendor();
    dc << setw(3) << paxFare.fare()->carrier();
    dc << setw(5) << paxFare.fare()->tcrRuleTariff();
    dc << setw(5) << paxFare.fare()->ruleNumber();

    dc << setw(8) << rule->sequenceNumber();
    dc << " R";
    dc << setw(2) << rule->categoryNumber();
    dc << '\n';

    displayDates(paxFare);
  }
}

void
Diag500Collector::displayRelation(const PaxTypeFare& paxFare,
                                  const CategoryRuleItemInfo* rule,
                                  Record3ReturnTypes statusRule)
{
  if (_active && shouldDisplay(paxFare, rule))
  {
    string status;
    if (statusRule == PASS)
    {
      status = "PASS";
    }
    else if (statusRule == FAIL)
    {
      status = "FAIL";
    }
    else if (statusRule == SKIP)
    {
      status = "SKIP";
    }
    else if (statusRule == SOFTPASS)
    {
      status = "SOFT";
    }
    else if (statusRule == STOP)
    {
      status = "FAIL";
    }
    else if (statusRule == STOP_SOFT)
    {
      status = "SOFT";
    }
    else
    {
      status = "XXXX";
    }

    string relation;

    switch (rule->relationalInd())
    {
    case CategoryRuleItemInfo::IF:
    {
      relation = "IF";
      break;
    }
    case CategoryRuleItemInfo::THEN:
    {
      relation = "THEN";
      break;
    }
    case CategoryRuleItemInfo::OR:
    {
      relation = "OR";
      break;
    }
    case CategoryRuleItemInfo::AND:
    {
      relation = "AND";
      break;
    }
    case CategoryRuleItemInfo::ELSE:
    {
      relation = "ELSE";
      break;
    }
    default:
    {
      relation = "UNKN";
    }
    }

    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    // Directionality values are as follows:
    // 1 = Travel From Loc 1 to Loc 2
    // 2 = Travel From Loc 2 to Loc 1
    // 3 = Fares Originating Loc 1
    // 4 = Fares Originating Loc 2

    dc << "            ";
    dc << std::setw(8) << relation;
    dc << setw(2) << rule->itemcat();
    dc << " - ";
    dc << setw(8) << rule->itemNo();

    dc << " DIR-";
    if (rule->directionality() == ' ')
      dc << setw(2) << "N ";
    else
      dc << setw(2) << rule->directionality();

    dc << " O/I-";
    if (rule->inOutInd() == ' ')
      dc << setw(2) << "N ";
    else
      dc << setw(2) << rule->inOutInd();
    dc << "  ";
    dc << std::setw(6) << status;
    dc << '\n';
  }
}

bool
Diag500Collector::shouldDisplay(const PaxTypeFare& paxFare, const CategoryRuleItemInfo* rule)
{
  if ((_categoryNumber != 0) && (_categoryNumber != rule->itemcat()))
  {
    return false;
  }

  if (!_trx->diagnostic().shouldDisplay(*paxFare.fareMarket()))
  {
    return false;
  }

  // below code should be changed using DiagnosticUtil::filter after this functionality be available
  std::string fareClass;
  std::map<std::string, std::string>::const_iterator itEnd =
      _trx->diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator it =
      _trx->diagnostic().diagParamMap().find("FC");

  if (it != itEnd)
    fareClass = it->second;

  if (fareClass.empty())
  {
    return true;
  }

  if (paxFare.fareClass() == fareClass || paxFare.createFareBasis(nullptr) == fareClass)
  {
    // FareClass found in this PricingUnit
    return true;
  }

  return false;
}
}
