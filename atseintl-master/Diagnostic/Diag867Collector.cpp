//----------------------------------------------------------------------------
//  File:        Diag867Collector.cpp
//  Authors:
//  Created:     2015
//
//  Description: Diagnostic 867 - Commissions Management.
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2015
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

#include "Diagnostic/Diag867Collector.h"

#include "Common/CommissionKeys.h"
#include "Common/FallbackUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionLocSegInfo.h"
#include "DBAccess/CommissionMarketSegInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/CommissionTravelDatesSegInfo.h"
#include "DBAccess/Currency.h"
#include "Rules/RuleUtil.h"

#include <iomanip>
#include <iostream>

using namespace std;
namespace
{
//                                       10        20        30        40        50        60
//                              012345678901234567890123456789012345678901234567890123456789012
const std::string separateLn = "*-------------------------------------------------------------*\n";
const std::string separateStarLn = "***************************************************************\n";
}
namespace tse
{
FALLBACK_DECL(fallbackDiag867ItinNum);
FALLBACK_DECL(fallbackFixDisplayValidatingCarrierDiag867);


void
Diag867Collector::printCommissionFC(const FareUsage& fu)
{
  printCommissionFC(*(fu.paxTypeFare()));
}

void
Diag867Collector::printFarePathHeader(const FarePath& fp)
{
    displayFarePathHeader(fp);
    if(!isDetailedDiagnostic())
      printFpCurrencies(fp);
}

void
Diag867Collector::printCommissionNotFound(const FareUsage& fu)
{
  if(!isDetailedDiagnostic())
  {
    printNoCommissionFound(*(fu.paxTypeFare()), _detailedDiag.nextFu);
    _detailedDiag.nextFu = true;
  }
}

void
Diag867Collector::printFinalAgencyCommission(MoneyAmount amt)
{
  if(!isDetailedDiagnostic())
    printFinalAgencyCommission(amt, _detailedDiag.anyComm);
}

void
Diag867Collector::initTrx(PricingTrx& trx)
{
  _trx = &trx;
  _detailedDiag.initialize(trx);
}

bool
Diag867Collector::diagFcFmMatch(const FareUsage& fu) const
{
  return (isDiagFareClassMatch(fu) &&
          isDiagFareMarketMatch(*fu.paxTypeFare()->fareMarket()));
}


bool
Diag867Collector::isDiagFareMarketMatch(const FareMarket& fareMarket) const
{
  return _detailedDiag.diagFareMarket.empty() ? true :
    (((fareMarket.origin()->loc() != _detailedDiag.diagBoardCity) &&
      (fareMarket.boardMultiCity() != _detailedDiag.diagBoardCity)) ||
     ((fareMarket.destination()->loc() != _detailedDiag.diagOffCity) &&
      (fareMarket.offMultiCity() != _detailedDiag.diagOffCity)));
}

bool
Diag867Collector::isDiagPaxTypeMatch(const FarePath& fp)
{
  return (!_detailedDiag.paxType.empty() && fp.paxType()) ?
    fp.paxType()->paxType() == _detailedDiag.paxType :
    true;
}

bool
Diag867Collector::isDiagFareClassMatch(const FareUsage& fu) const
{
  return _detailedDiag.diagFareClass.empty() ||
      fu.paxTypeFare()->fareClass() != _detailedDiag.diagFareClass;
}

void
Diag867Collector::displayFarePathHeader(const FarePath& farePath)
{
  if (_active)
  {
    Diag867Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
//         012345678901234567890123456789012345678901234567890123456789012
    dc << separateStarLn;
    dc << "*          AGENCY MANAGED COMMISSION DIAGNOSTIC 867           *\n";
    dc << separateStarLn;
    dc.setf(std::ios::left, std::ios::adjustfield);

    // sg952572 -  Including init number code
    if(!fallback::fallbackDiag867ItinNum(_trx))
    {
        if((farePath.itin() != nullptr) && (_trx->getTrxType() == PricingTrx::MIP_TRX))
        {
            dc << "ITIN-NO: " << farePath.itin()->itinNum() << "\n";
        }
    }

    displayPaxTypeLine(farePath.paxType()->paxType());
  }
}

void
Diag867Collector::displayPaxTypeLine(const PaxTypeCode& paxTypeCode)
{
  Diag867Collector& dc = *this;

  dc << "                   ** REQUESTED PTC: " << paxTypeCode << " **\n";
}

void
Diag867Collector::printAMCNotApplicable()
{
  if (!_active)
  return;

  Diag867Collector& dc = *this;
  dc << "AMC IS NOT APPLICABLE\n";
}

void
Diag867Collector::printAMCforCat35NotApplicable(const FarePath& farePath)
{
  if (!_active || !paxTypeMatched(farePath))
    return;
  displayFarePathHeader(farePath);
  printAMCNotApplicable();
  Diag867Collector& dc = *this;
  dc << "REASON: CAT 35 NET TICKETING OR CAT 35 NET REMIT\n";
}

void
Diag867Collector::printAMCforASNotApplicable(const FarePath& farePath)
{
  if (!_active || !paxTypeMatched(farePath))
    return;
  displayFarePathHeader(farePath);
  printAMCNotApplicable();
  Diag867Collector& dc = *this;
  dc << "REASON: AIRLINE SOLUTION CUSTOMER\n";
}

void
Diag867Collector::printAMCCommandNotSupported(const FarePath& farePath)
{
  if (!_active || !paxTypeMatched(farePath))
    return;
  displayFarePathHeader(farePath);
  printAMCNotApplicable();
  Diag867Collector& dc = *this;
  dc << "REASON: COMMAND NOT SUPPORTED\n";
}

void
Diag867Collector::printNoSecurityHandShakeForPCC(const FarePath& farePath,
                                                 const PseudoCityCode& pcc)
{
  if (!_active || !paxTypeMatched(farePath))
    return;
  displayFarePathHeader(farePath);
  printAMCNotApplicable();
  Diag867Collector& dc = *this;
  dc << "REASON:  PCC " << pcc << " DOES NOT HAVE SECURITY HANDSHAKE\n";
}

void
Diag867Collector::printAMCforKPandEPRNotApplicable(const FarePath& farePath)
{
  if (!_active || !paxTypeMatched(farePath))
    return;
  displayFarePathHeader(farePath);
  printAMCNotApplicable();
  Diag867Collector& dc = *this;
  dc << "REASON: AGENT OVERRIDE INPUT WITH KEYWORD COMOVR\n";
}

void
Diag867Collector::printCommissionNotProcessed()
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " \n  AGENCY MANAGED COMMISSION NOT PROCESSED OR NOT FOUND\n";
  }
}

//@todo remove it
bool
Diag867Collector::paxTypeMatched(const FarePath& farePath)
{
  if (_active)
  {
    const std::string& paxType = _trx->diagnostic().diagParamMapItem(Diagnostic::PAX_TYPE);
    if (!paxType.empty() && farePath.paxType() &&
        farePath.paxType()->paxType() != paxType)
      return false;
  }
  return true;
}

void
Diag867Collector::printContractNotFound(const CarrierCode& valCxr,
                                        const PseudoCityCode& pcc)
{
  if (!_active)
    return;

  Diag867Collector& dc = *this;
  dc << "AMC IS NOT APPLICABLE FOR CARRIER CODE " << valCxr
     << " / PCC " << pcc << "\n";
  dc << "REASON: NO CONTRACT FOUND\n";
  printFuClosingLine();
}

void
Diag867Collector::printCommissionContractInfoShort(const CommissionContractInfo& cci)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << separateLn;
  printCommonCommissionContractInfo(cci);
  dc << " \n";
}

void
Diag867Collector::printCommonCommissionContractInfo(const CommissionContractInfo& cci)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
//  dc << separateLn;
  dc << "CONTRACT CARRIER CODE: " << cci.carrier() << "\n"
     << "CONTRACT ID          : " << cci.contractId() << "\n"
     << "CONTRACT DESCRIPTION : ";
  std::string tmp = cci.description();
  boost::to_upper(tmp);
  dc << std::setw(41) << tmp << "\n";
}

void
Diag867Collector::printCommissionContractInfo(const CommissionContractInfo& cci)
{
  if (!_active) return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << separateLn;
  printCommonCommissionContractInfo(cci);

  dc << "CONTRACT VENDOR      : " << cci.vendor() << "\n"
     << "EFFECTIVE DATE TIME  : ";
  printDate(cci.effDateTime());
  dc << "EXPIRED DATE TIME    : ";
  printDate(cci.expireDate());
}

void
Diag867Collector::printFpCurrencies(const FarePath& fp)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  CurrencyCode paymentCurrency = _trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx->getOptions()->currencyOverride().empty())
    paymentCurrency = _trx->getOptions()->currencyOverride();

  dc << "BASE FARE CURRENCY   : " << fp.baseFareCurrency() << "\n"
     << "PAYMENT CURRENCY     : " << paymentCurrency << "\n";
}

void
Diag867Collector::printValidatingCarrierFCHeader(const CarrierCode& cc)
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "                   ** VALIDATING CXR: "<< cc <<" **\n";
  }
}

void
Diag867Collector::printCommissionFCHeader()
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << separateStarLn;
    dc << "*      AGENCY COMMISSION ANALYSIS - FARE COMPONENT LEVEL      *\n";
    dc << separateStarLn;
  }
}

void
Diag867Collector::printCommissionFC(const PaxTypeFare& paxTypeFare)
{
  if (_active)
  {
    Diag867Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << paxTypeFare.fareMarket()->origin()->loc() << " - ";
    dc << paxTypeFare.fare()->carrier() << " - ";
    dc << paxTypeFare.fareMarket()->destination()->loc() << "  ";
    dc << paxTypeFare.fare()->fareClass() << '\n';
    dc << separateStarLn;
  }
}

void
Diag867Collector::printFuClosingLine()
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " \n" << separateStarLn;
  }
}

void
Diag867Collector::printNoCommissionFound()
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "  NO AGENCY COMMISSION FOUND\n";
  }
}

void
Diag867Collector::printFuNoCommissionFound()
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << separateStarLn;
    printNoCommissionFound();
    dc << separateStarLn;
  }
}

void
Diag867Collector::printNoCommissionFound(const PaxTypeFare& paxTypeFare, bool nextFu)
{
  if (_active)
  {
    Diag867Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    if(_detailedDiag.nextFu)
      printFuClosingLine();
    printCommissionFC(paxTypeFare);
    printNoCommissionFound();
  }
}

void
Diag867Collector::printCommissionRuleShortHeader()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  COMMISSION ID  TYPE  VALUE        STATUS\n";
}

void
Diag867Collector::printCommissionsRuleInfoShort(const CommissionRuleInfo& cri,
                                                CommissionValidationStatus rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "  " << std::setw(13) << cri.commissionId() << "  "
     << std::setw(4) << cri.commissionTypeId() << "  ";
  uint8_t length = 0;
  if(cri.commissionValue() >= 0)
  {
    if(cri.commissionTypeId() == 12)
    {
      dc << formatAmount(cri.commissionValue(), cri.currency());
      length = (formatAmount(cri.commissionValue(), cri.currency())).size();
    }
    else
    {
      dc << formatPercent(cri.commissionValue());
      length = (formatPercent(cri.commissionValue())).size();
      if (_trx && _trx->billing() != nullptr && (_trx->billing()->requestPath() != PSS_PO_ATSE_PATH))
      {
        dc << "%";
        ++length;
      }
    }
 }
  length = 13 - length;
  dc << std::setw(length) << " ";
  displayShortFailStatus(rc);
}

void
Diag867Collector::printRuleIdName(const CommissionRuleInfo& cri)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "COMMISSION ID: " << cri.commissionId() << "\n";
  dc << "DESCRIPTION  : ";
  std::string tmp = cri.description();
  boost::to_upper(tmp);
  dc << std::setw(48) << tmp << "\n";
}

void
Diag867Collector::printCommissionsRuleInfo(const CommissionRuleInfo& cri,
                                           CommissionValidationStatus rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << separateLn;
  printRuleIdName(cri);
  dc << "VALIDATION RESULT: ";
  displayStatus(rc);

  dc << separateLn;
  dc << "---COMMISSION RULE DATA---\n";
  dc << "VENDOR: " << std::setw(4) << cri.vendor() << "   ";
  dc << "PROGRAM ID: " << std::setw(10) << cri.programId() << "\n";
  dc << "COMMISSION TYPE          : " << cri.commissionTypeId();
  printCommissionTypeIdVerbiage(cri.commissionTypeId());

  if(cri.commissionTypeId() == 12)
    dc << "COMMISSION AMOUNT        : ";
  else
    dc << "COMMISSION PERCENT       : ";

  if(cri.commissionValue() >= 0)
  {
    if(cri.commissionTypeId() == 12)
      dc << formatAmount(cri.commissionValue(), cri.currency());
    else
    {
      dc << formatPercent(cri.commissionValue());
      if (_trx && _trx->billing() != nullptr && (_trx->billing()->requestPath() != PSS_PO_ATSE_PATH))
        dc << "%";
    }
  }
  dc << "\n";

  dc << "EFFECTIVE DATE TIME      : ";
  printDate(cri.effDate());
  dc << "EXPIRED DATE TIME        : ";
  printDate(cri.expireDate());
  dc << "PASSENGER TYPE REQUIRED  : ";
  displayAnyString(cri.requiredPaxType());
  dc << "FBC STARTS WITH REQUIRED : ";
  displayAnyString(cri.fareBasisCodeIncl());
  dc << "                EXCLUDED : ";
  displayAnyString(cri.fareBasisCodeExcl());
  dc << "FBC FRAGMENTS REQUIRED   : ";
  displayAnyString(cri.fbcFragmentIncl());
  dc << "              EXCLUDED   : " ;
  displayAnyString(cri.fbcFragmentExcl());
  dc << "TKT DESIGNATOR REQUIRED  : ";
  displayAnyString(cri.requiredTktDesig());
  dc << "               EXCLUDED  : ";
  displayAnyString(cri.excludedTktDesig());

  dc << "CABIN TYPE REQUIRED      : ";
  displayAnyString(cri.requiredCabinType());
  dc << "           EXCLUDED      : ";
  displayAnyString(cri.excludedCabinType());
  dc << "CLASS OF SERVICE REQUIRED: ";
  displayAnyString(cri.classOfServiceIncl());
  dc << "                 EXCLUDED: ";
  displayAnyString(cri.classOfServiceExcl());

  dc << "MARKETING CXR REQUIRED   : ";
  displayAnyString(cri.marketingCarrierIncl());
  dc << "              EXCLUDED   : ";
  displayAnyString(cri.marketingCarrierExcl());
  dc << "OPERATING CXR REQUIRED   : ";
  displayAnyString(cri.operatingCarrierIncl());
  dc << "              EXCLUDED   : ";
  displayAnyString(cri.operatingCarrierExcl());
  dc << "TICKETING CXR REQUIRED   : ";
  displayAnyString(cri.ticketingCarrierIncl());
  dc << "              EXCLUDED   : ";
  displayAnyString(cri.ticketingCarrierExcl());

  dc << "MKT GOV CXR REQUIRED     : ";
  displayAnyString(cri.requiredMktGovCxr());
  dc << "            EXCLUDED     : ";
  displayAnyString(cri.excludedMktGovCxr());
  dc << "OPER GOV CXR REQUIRED    : ";
  displayAnyString(cri.requiredOperGovCxr());
  dc << "             EXCLUDED    : ";
  displayAnyString(cri.excludedOperGovCxr());

  dc << "CONNECT POINT REQUIRED   : ";
  displayAnyString(cri.requiredCnxAirPCodes());
  dc << "              EXCLUDED   : ";
  displayAnyString(cri.excludedCnxAirPCodes());

  dc << "NON-STOP REQUIRED        : ";
  if(!cri.requiredNonStop().empty())
  {
    uint8_t length = 0;
    for(const CommissionRuleInfo::OriginDestination od: cri.requiredNonStop())
    {
      length += 7;
      if(length < 36)
        dc << od._origin << od._destination << " ";
      else
      {
        dc << "\n" << std::setw(27) << " ";
        length = 7;
        dc << od._origin << od._destination << " ";
      }
    }
  }
  dc << "\n";
  dc << "INTERLINE REQUIRED       : " << cri.interlineConnectionRequired() << "\n";

  dc << "ROUND TRIP               : ";
  if(cri.roundTrip() == 'R')
    dc << "R - REQUIRED";
  else
  if(cri.roundTrip() == 'X')
    dc << "X - EXCLUDED";
  dc << "\n";

  dc << "FARE AMOUNT MINIMUM      : ";
  if(cri.fareAmountMin() >= 0)
  {
    dc << formatAmount(cri.fareAmountMin(), cri.fareAmountCurrency());
  }
  dc << "\n";
  dc << "            MAXIMUM      : ";
  if(cri.fareAmountMax() >= 0)
  {
    dc << formatAmount(cri.fareAmountMax(), cri.fareAmountCurrency());
  }
  dc << "\n";
}

void
Diag867Collector::printCommissionTypeIdVerbiage(unsigned commissionTypeId)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  if(commissionTypeId == 9)
    dc << " - NO PRORATION ALLOWED";
  if(commissionTypeId == 10)
    dc << " - NON ZERO PRORATION";
  if(commissionTypeId == 11)
    dc << " - ZERO PRORATION";
  if(commissionTypeId == 12)
    dc << " - SEGMENT BONUS";
  dc << "\n";
}

template <class T>
void
Diag867Collector::displayAnyString(const std::vector<T>& inputString)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  if(inputString.empty())
  {
    dc << "\n";
    return;
  }

  std::ostringstream oss;
  std::ostringstream os;

  uint8_t length = inputString.size();
  const char* const wild = "%";

  for(uint8_t i=0; i < length; ++i )
  {
    os.str("");
    os << inputString[i];
    std::string value = os.str();
    std::size_t indexWild = 0;
    if (_trx && _trx->billing() != nullptr && (_trx->billing()->requestPath() == PSS_PO_ATSE_PATH))
    {
      while((indexWild = value.find(wild, indexWild)) != std::string::npos)
      {
        value.erase(indexWild, 1);
        value.insert(indexWild,"=");
      }
    }

    if(oss.str().size() + value.size() < 35)
      oss << value << " ";
    else
    {
      dc << oss.str() << "\n"
         << std::setw(27) << " ";
      oss.str("");
      oss << value << " ";
    }
  }
  if(!(oss.str()).empty())
    dc << oss.str() << "\n";
}

void
Diag867Collector::displayStatus(CommissionValidationStatus status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_CR:
    dc << "PASS";
    break;
  case SKIP_CR:
    dc << "SKIP";
    break;
  case FAIL_CR_FARE_BASIS_INCL:
    dc << "FAIL - NOT MATCH FARE BASIS";
    break;
  case FAIL_CR_FARE_BASIS_EXCL:
    dc << "FAIL - MATCH EXCL FARE BASIS CODE";
    break;
  case FAIL_CR_CLASS_OF_SERVICE_INCL:
    dc << "FAIL - NOT MATCH CLASS OF SERVICE";
    break;
  case FAIL_CR_CLASS_OF_SERVICE_EXCL:
    dc << "FAIL - MATCH EXCL CLASS OF SERVICE";
    break;
  case FAIL_CR_OPER_CARRIER_INCL:
    dc << "FAIL - NOT MATCH OPERATING CARRIER";
    break;
  case FAIL_CR_OPER_CARRIER_EXCL:
    dc << "FAIL - MATCH EXCL OPERATING CARRIER";
    break;
  case FAIL_CR_MARKET_CARRIER_INCL:
    dc << "FAIL - NOT MATCH MARKETING CARRIER";
    break;
  case FAIL_CR_MARKET_CARRIER_EXCL:
    dc << "FAIL - MATCH EXCL MARKETING CARRIER";
    break;
  case FAIL_CR_TICKET_CARRIER_INCL:
    dc << "FAIL - NOT MATCH TICKETING CARRIER";
    break;
  case FAIL_CR_TICKET_CARRIER_EXCL:
    dc << "FAIL - MATCH EXCL TICKETING CARRIER";
    break;
  case FAIL_CR_INTERLINE_CONNECTION:
    dc << "FAIL - NOT INTERLINE CONNECTION";
    break;
  case FAIL_CR_ROUNDTRIP:
    dc << "FAIL - NOT MATCH ROUND TRIP";
    break;
  case FAIL_CR_FARE_AMOUNT_MIN:
    dc << "FAIL - NOT MATCH MIN FARE AMOUNT";
    break;
  case FAIL_CR_FARE_AMOUNT_MIN_NODEC:
    dc << "FAIL - NOT MATCH NODEC MIN FARE AMOUNT";
    break;
  case FAIL_CR_FARE_AMOUNT_MAX:
    dc << "FAIL - NOT MATCH MAX FARE AMOUNT";
    break;
  case FAIL_CR_FARE_AMOUNT_MAX_NODEC:
    dc << "FAIL - NOT MATCH NODEC MAX FARE AMOUNT";
    break;
  case FAIL_CR_FARE_CURRENCY:
    dc << "FAIL - NOT MATCH FARE CURRENCY";
    break;
  case FAIL_CR_FBC_FRAGMENT_INCL:
    dc << "FAIL - NOT MATCH FARE BASIS FRAGMENT";
    break;
  case FAIL_CR_FBC_FRAGMENT_EXCL:
    dc << "FAIL - MATCH EXCL FARE BASIS FRAGMENT";
    break;
  case FAIL_CR_REQ_TKT_DESIGNATOR:
    dc << "FAIL - NOT MATCH TKT DESIGNATOR";
    break;
  case FAIL_CR_EXCL_TKT_DESIGNATOR:
    dc << "FAIL - MATCH EXCL TKT DESIGNATOR";
    break;
  case FAIL_CR_REQ_CONN_AIRPORT:
    dc << "FAIL - NOT MATCH CONNECTION AIRPORT";
    break;
  case FAIL_CR_EXCL_CONN_AIRPORT:
    dc << "FAIL - MATCH EXCL CONNECTION AIRPORT";
    break;
  case FAIL_CR_REQ_MKT_GOV_CXR:
    dc << "FAIL - NOT MATCH REQ MKT GOV CARRIER";
    break;
  case FAIL_CR_EXCL_MKT_GOV_CXR:
    dc << "FAIL - MATCH EXCL MKT GOV CARRIER";
    break;
  case FAIL_CR_PSGR_TYPE:
    dc << "FAIL - NOT MATCH PSGR TYPE";
    break;
  case FAIL_CR_REQ_OPER_GOV_CXR:
    dc << "FAIL - NOT MATCH REQ OPER GOV CARRIER";
    break;
  case FAIL_CR_EXCL_OPER_GOV_CXR:
    dc << "FAIL - MATCH EXCL OPER GOV CARRIER";
    break;
  case FAIL_CR_REQ_NON_STOP:
    dc << "FAIL - NOT MATCH NON STOP";
    break;
  case FAIL_CR_REQ_CABIN:
    dc << "FAIL - NOT MATCH CABIN";
    break;
  case FAIL_CR_EXCL_CABIN:
    dc << "FAIL - MATCH EXCL CABIN";
    break;
  case FAIL_CR_EXCL_TOUR_CODE:
    dc << "FAIL - MATCH EXCL TOUR CODE";
    break;
  case FAIL_CP_POINT_OF_SALE:
    dc << "FAIL - NOT MATCH POINT OF SALE";
    break;
  case FAIL_CP_POINT_OF_ORIGIN:
    dc << "FAIL - NOT MATCH POINT OF ORIGIN";
    break;
  case FAIL_CP_TRAVEL_DATE:
    dc << "FAIL - NOT MATCH TRAVEL DATE";
    break;
  case FAIL_CP_TICKET_DATE:
    dc << "FAIL - NOT MATCH TICKETING DATE";
    break;
  case FAIL_CP_MAX_CONNECTION_TIME:
    dc << "FAIL - NOT MATCH CONNECTION TIME";
    break;
  case FAIL_CP_MARKET:
    dc << "FAIL - NOT MATCH MARKET";
    break;
  case FAIL_CP_NOT_VALID:
    dc << "FAIL - PROGRAM NOT VALIDATED";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

void
Diag867Collector::displayShortFailStatus(CommissionValidationStatus status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_CR:
    dc << "PASS";
    break;
  case SKIP_CR:
    dc << "SKIP";
    break;
  case FAIL_CR_FARE_BASIS_INCL:
  case FAIL_CR_FARE_BASIS_EXCL:
    dc << "FAIL - FARE BASIS";
    break;
  case FAIL_CR_CLASS_OF_SERVICE_INCL:
  case FAIL_CR_CLASS_OF_SERVICE_EXCL:
    dc << "FAIL - CLASS OF SERVICE";
    break;
  case FAIL_CR_OPER_CARRIER_INCL:
  case FAIL_CR_OPER_CARRIER_EXCL:
    dc << "FAIL - OPERATING CARRIER";
    break;
  case FAIL_CR_MARKET_CARRIER_INCL:
  case FAIL_CR_MARKET_CARRIER_EXCL:
    dc << "FAIL - MARKETING CARRIER";
    break;
  case FAIL_CR_TICKET_CARRIER_INCL:
  case FAIL_CR_TICKET_CARRIER_EXCL:
    dc << "FAIL - TICKETING CARRIER";
    break;
  case FAIL_CR_INTERLINE_CONNECTION:
    dc << "FAIL - INTERLINE CONNECTION";
    break;
  case FAIL_CR_ROUNDTRIP:
    dc << "FAIL - ROUND TRIP";
    break;
  case FAIL_CR_FBC_FRAGMENT_INCL:
  case FAIL_CR_FBC_FRAGMENT_EXCL:
    dc << "FAIL - FARE BASIS FRAGMENT";
    break;
  case FAIL_CR_REQ_TKT_DESIGNATOR:
  case FAIL_CR_EXCL_TKT_DESIGNATOR:
    dc << "FAIL - TKT DESIGNATOR";
    break;
  case FAIL_CR_REQ_CONN_AIRPORT:
  case FAIL_CR_EXCL_CONN_AIRPORT:
    dc << "FAIL - CONNECTION AIRPORT";
    break;
  case FAIL_CR_REQ_MKT_GOV_CXR:
  case FAIL_CR_EXCL_MKT_GOV_CXR:
    dc << "FAIL - MARKET GOV CARRIER";
    break;
  case FAIL_CR_PSGR_TYPE:
    dc << "FAIL - PSGR TYPE";
    break;
  case FAIL_CR_REQ_OPER_GOV_CXR:
  case FAIL_CR_EXCL_OPER_GOV_CXR:
    dc << "FAIL - OPER GOV CARRIER";
    break;
  case FAIL_CR_REQ_NON_STOP:
    dc << "FAIL - NON STOP";
    break;
  case FAIL_CR_REQ_CABIN:
  case FAIL_CR_EXCL_CABIN:
    dc << "FAIL - CABIN";
    break;
  case FAIL_CR_EXCL_TOUR_CODE:
    dc << "FAIL - TOUR CODE";
    break;
  case FAIL_CR_FARE_AMOUNT_MIN:
  case FAIL_CR_FARE_AMOUNT_MIN_NODEC:
    dc << "FAIL - MIN FARE AMOUNT";
    break;
  case FAIL_CR_FARE_AMOUNT_MAX:
  case FAIL_CR_FARE_AMOUNT_MAX_NODEC:
    dc << "FAIL - MAX FARE AMOUNT";
    break;
  case FAIL_CR_FARE_CURRENCY:
    dc << "FAIL - FARE CURRENCY";
    break;
  case FAIL_CP_POINT_OF_SALE:
    dc << "FAIL - POINT OF SALE";
    break;
  case FAIL_CP_POINT_OF_ORIGIN:
    dc << "FAIL - POINT OF ORIGIN";
    break;
  case FAIL_CP_TRAVEL_DATE:
    dc << "FAIL - TRAVEL DATE";
    break;
  case FAIL_CP_TICKET_DATE:
    dc << "FAIL - TICKETING DATE";
    break;
  case FAIL_CP_MAX_CONNECTION_TIME:
    dc << "FAIL - CONNECTION TIME";
    break;
  case FAIL_CP_MARKET:
    dc << "FAIL - MARKET";
    break;
  case FAIL_CP_NOT_VALID:
    dc << "FAIL - NOT VALIDATED";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";}

void
Diag867Collector::printProgramIdName(const CommissionProgramInfo& cpi)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "PROGRAM ID: " << cpi.programId() << "\n";
  dc << "PROGRAM NAME: ";
  std::string tmp = cpi.programName();
  boost::to_upper(tmp);
  dc << std::setw(49) << tmp << "\n";
}

void
Diag867Collector::printCommissionsProgramInfo(const CommissionProgramInfo& cpi,
                                              CommissionValidationStatus rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << separateLn;
  printProgramIdName(cpi);
  dc << "VALIDATION RESULT: ";
  displayStatus(rc);

  dc << separateLn;
  dc << "---COMMISSION PROGRAM DATA---\n";
  dc << "VENDOR: " << std::setw(4) << cpi.vendor() << " ";
  dc << "CONTRACT ID: " << cpi.contractId() << "\n";

  dc << "EFFECTIVE DATE TIME     : ";
  printDate(cpi.effDate());
  dc << "EXPIRED DATE TIME       : ";
  printDate(cpi.expireDate());

  dc << "VALID POINTS OF SALE ITEM NO  : " << cpi.pointOfSaleItemNo() << "\n";
  if(cpi.pointOfSaleItemNo() != 0 && !cpi.pointOfSale().empty())
  {
    printCommissionsLocSegInfo(cpi.pointOfSale());
  }

  dc << "VALID POINTS OF ORIGIN ITEM NO: " << cpi.pointOfOriginItemNo() << "\n";
  if(cpi.pointOfOriginItemNo() != 0 && !cpi.pointOfOrigin().empty())
  {
    printCommissionsLocSegInfo(cpi.pointOfOrigin());
  }

  dc << "VALID MARKET ITEM NO : " << cpi.marketItemNo() << "\n";
  if(cpi.marketItemNo() != 0 && !cpi.markets().empty())
  {
    dc << "  ORDER NO    ORIG      DEST      BI-DIRECTIONAL INCL/EXCL\n";

    for (const CommissionMarketSegInfo* market : cpi.markets())
    {
      dc << "  " << std::setw(12) << market->orderNo()
         << market->origin().locType() << "-"
         << std::setw(8) << market->origin().loc()
         << market->destination().locType() << "-"
         << std::setw(14) << market->destination().loc();
      if(market->bidirectional() == 'Y')
        dc << std::setw(11) << "YES";
      else
        dc << std::setw(11) << "NO";

      if(market->inclExclInd() == 'I')
        dc << "INCL\n";
      else
        dc << "EXCL\n";
    }
  }

  dc << "TRAVEL DATES ITEM NO    : " << cpi.travelDatesItemNo() << "\n";
  if(cpi.travelDatesItemNo() != 0 && !cpi.travelDates().empty())
  {
    for (const CommissionTravelDatesSegInfo* travelDate : cpi.travelDates())
    {
      dc << "  START TRAVEL DATE     : ";
      printDate(travelDate->firstTravelDate());

      dc << "  END TRAVEL DATE       : ";
      printDate(travelDate->endTravelDate());
    }
  }

  dc << "START TICKETING DATE    : ";
  printDate(cpi.startTktDate());
  dc << "END TICKETING DATE      : ";
  printDate(cpi.endTktDate());

  dc << "SURCHARGE INDICATOR     : " << cpi.qSurchargeInd() << "\n";
  dc << "THROUGH FARE INDICATOR  : " << cpi.throughFareInd() << "\n";
  dc << "MAX CONNECTION TIME     : " << cpi.maxConnectionTime() << "\n";
  dc << "LAND AGREEMENT INDICATOR: " << cpi.landAgreementInd() << "\n \n";
}

void
Diag867Collector::printDate(DateTime date)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << date.dateToString(DDMMMYYYY, "-");
  if( date.isValid())
    dc << "  "  << date.timeToString(HHMMSS, ".");
  dc << "\n";
}

void
Diag867Collector::printCommissionsLocSegInfo(const std::vector<CommissionLocSegInfo*>& logSegs)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "  ORDER NO          LOC       INCL/EXCL\n";

  for (const CommissionLocSegInfo* pos : logSegs)
  {
   dc << "  " << std::setw(18) << pos->orderNo()
       << pos->loc().locType() << "-"
       << std::setw(10) << pos->loc().loc();
   if(pos->inclExclInd() == 'I')
     dc << "INCL\n";
   else
     dc << "EXCL\n";
  }
}

void
Diag867Collector::printCommissionsProgramInfoShort(const CommissionProgramInfo& cpi,
                                                   CommissionValidationStatus rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "PROGRAM ID: " << std::setw(12) << cpi.programId();
  if(rc == PASS_CR)
  {
    dc << "PASS      SURCHARGE ALLOWED-" << cpi.qSurchargeInd() << "\n";
  }
  else
    displayShortFailStatus(rc);
}

void
Diag867Collector::printFinalFcCommission(const FarePath& fp, const FareUsage& fu,
                                         MoneyAmount amt, const amc::CommissionRuleData& crd)
{
  if (!_active || !crd.commContInfo() || !crd.commProgInfo() || !crd.commRuleInfo())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  printCommissionFCHeader();
  printCommissionFC(*(fu.paxTypeFare()));
  printCommonCommissionContractInfo(*(crd.commContInfo()));
  dc << " \n";
  printProgramIdName(*(crd.commProgInfo()));
  dc << "SURCHARGE COMMISSION ALLOWED: ";
  if(crd.isQSurchargeApplicable())
    dc << "YES\n";
  else
    dc << "NO\n";
  dc << " \n";

  printRuleIdName(*(crd.commRuleInfo()));
  dc << "AGENCY COMMISSION TYPE             : " << crd.commRuleInfo()->commissionTypeId();
  printCommissionTypeIdVerbiage(crd.commRuleInfo()->commissionTypeId());
  dc << " \n";

  if(crd.commRuleInfo()->commissionTypeId() == 12)
    dc << "AGENCY COMMISSION AMOUNT           : "
      << setw(10) << formatAmount(crd.commRuleInfo()->commissionValue(), crd.commRuleInfo()->currency(), true)
      << crd.commRuleInfo()->currency();
  else
    dc << "AGENCY COMMISSION PERCENT          : " << formatPercent(crd.commRuleInfo()->commissionValue());
  dc << " \n \n";

  CurrencyCode calcCur = fp.calculationCurrency();
  dc << "TOTAL FARE AMOUNT                  : "
    << setw(10) << formatAmount(fu.totalFareAmount(), calcCur, true) << calcCur << "\n";

  dc << "CAT12 SURCHARGE                    : "
    << setw(10) << formatAmount((fp.isRollBackSurcharges()?
          fu.surchargeAmtUnconverted() : fu.surchargeAmt()), calcCur, true) << calcCur << "\n \n";

  MoneyAmount commAmt = fu.totalFareAmount();
  if(crd.commProgInfo()->qSurchargeInd() != 'Y')
    commAmt = fu.totalFareAmount() - (fp.isRollBackSurcharges()?
        fu.surchargeAmtUnconverted() : fu.surchargeAmt());
  dc << "FARE AMOUNT FOR COMMISSION CALCULATION\n";

  CurrencyCode baseFareCurrency = fp.baseFareCurrency();
  CurrencyCode paymentCurrency = _trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx->getOptions()->currencyOverride().empty())
    paymentCurrency = _trx->getOptions()->currencyOverride();

  if (calcCur != baseFareCurrency)
  {
    dc << "           IN CALCULATION CURRENCY : "
      << setw(10) << formatAmount(commAmt, calcCur, true) << calcCur << "\n";

    commAmt = convertCurrency(fp, commAmt, calcCur,
        baseFareCurrency, fp.applyNonIATARounding(*_trx));
  }

  if (baseFareCurrency != paymentCurrency)
  {
    dc << "           IN BASE FARE CURRENCY   : "
      << setw(10) << formatAmount(commAmt, baseFareCurrency, true) << baseFareCurrency << "\n";
    commAmt = convertCurrency(fp, commAmt, baseFareCurrency, paymentCurrency);
  }

  dc << "           IN PAYMENT CURRENCY     : "
    << setw(10) << formatAmount( commAmt, paymentCurrency, true)
    << paymentCurrency << "\n \n";

  dc << "AGENCY COMMISSION APPLICATION\n";

  if(crd.commRuleInfo()->commissionTypeId() == 9 &&
      (fp.pricingUnit().size() > 1 || fp.pricingUnit()[0]->fareUsage().size() > 1))
  {
    dc << "LOWEST AGENCY COMMISSION PERCENT   : "
      << formatPercent(crd.commRuleInfo()->commissionValue()) << "\n";
  }

  dc << "FARE AMOUNT * COMM PERCENT         : ";

  CurrencyConverter cc;
  Money target(amt, paymentCurrency);
  cc.roundNone(target, 0);
  dc << setw(10) << formatAmount( target.value(), paymentCurrency, true)
    << paymentCurrency << "\n";
}

void
Diag867Collector::printFinalAgencyCommission(MoneyAmount amt, bool comm)
{
  if (!_active)
    return;

  if( !comm )
  {
    printFuNoCommissionFound();
    return;
  }

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  CurrencyCode paymentCurrency = _trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  if (!_trx->getOptions()->currencyOverride().empty())
    paymentCurrency = _trx->getOptions()->currencyOverride();

  dc << separateStarLn
     << "*        FINAL AGENCY FARE COMMISSION - FARE PATH LEVEL       *\n"
     << separateStarLn
     << "   CALCULATED AGENCY COMMISSION AMOUNT: "
     << formatAmount(amt, paymentCurrency) << "\n"
     << separateStarLn;
}

std::string
Diag867Collector::formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode, bool noCurr) const
{
  CurrencyNoDec noDec = 2;

  if (currencyCode != NUC)
  {
    DataHandle dataHandle;
    const Currency* currency = nullptr;
    currency = dataHandle.getCurrency( currencyCode );

    if (currency)
    {
      noDec = currency->noDec();
    }
  }
  std::ostringstream os;

  os.setf(std::ios::left | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);
  os.precision(noDec);
  if(noCurr)
    os << amount;
  else
    os << amount << " " << currencyCode;

  return os.str();
}

std::string
Diag867Collector::formatPercent(Percent percent) const
{
  std::ostringstream os;

  os.setf(std::ios::left | std::ios::fixed, std::ios::adjustfield | std::ios::floatfield);
  os.precision(2);
  os << percent;

  return os.str();
}

MoneyAmount
Diag867Collector::convertCurrency(const FarePath& fp,
                                  MoneyAmount sourceAmount,
                                  const CurrencyCode& sourceCurrency,
                                  const CurrencyCode& targetCurrency,
                                  bool doNonIataRounding)
{
  return RuleUtil::convertCurrency(*_trx, fp, sourceAmount, sourceCurrency, targetCurrency, doNonIataRounding);
}

void
Diag867Collector::DetailedDiag867::initialize(PricingTrx& trx)
{
  const Diagnostic& diag = trx.diagnostic();
  isDDAMC = diag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "AMC";
  isDDINFO = diag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
  isDDPASSED = diag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED";

  if (isDDAMC || isDDINFO || isDDPASSED)
  {
    isDDCC = !diag.diagParamMapItem(Diagnostic::COMMISSION_CONTRACT).empty();
    isDDID = !diag.diagParamMapItem(Diagnostic::IDENTIFICATION).empty();
    isDDRU = !diag.diagParamMapItem(Diagnostic::RULE_NUMBER).empty();
    if (isDDCC)
      contractId = (uint32_t)atoi(diag.diagParamMapItem(Diagnostic::COMMISSION_CONTRACT).c_str());
    if (isDDID)
      programId = (uint32_t)atoi(diag.diagParamMapItem(Diagnostic::IDENTIFICATION).c_str());
    if (isDDRU)
      commRuleId = (uint32_t)atoi(diag.diagParamMapItem(Diagnostic::RULE_NUMBER).c_str());

    if(fallback::fallbackFixDisplayValidatingCarrierDiag867(&trx))
    {
      isDDCX = !diag.diagParamMapItem(Diagnostic::DIAG_CARRIER).empty();
      if (isDDCX)
        valCxr = diag.diagParamMapItem(Diagnostic::DIAG_CARRIER);
    }
  }

  if(!fallback::fallbackFixDisplayValidatingCarrierDiag867(&trx))
  {
    isDDCX = !diag.diagParamMapItem(Diagnostic::DIAG_CARRIER).empty();
    if (isDDCX)
      valCxr = diag.diagParamMapItem(Diagnostic::DIAG_CARRIER);
  }

  if (!diag.diagParamMapItem(Diagnostic::PAX_TYPE).empty())
    paxType = diag.diagParamMapItem(Diagnostic::PAX_TYPE);

  if (!diag.diagParamMapItem(Diagnostic::FARE_CLASS_CODE).empty())
    diagFareClass = diag.diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  if (!diag.diagParamMapItem(Diagnostic::FARE_MARKET).empty())
    diagFareMarket = diag.diagParamMapItem(Diagnostic::FARE_MARKET);

  if (!diagFareMarket.empty())
  {
    diagBoardCity = diagFareMarket.substr(0, 3);
    diagOffCity = diagFareMarket.substr(3, 3);
  }
}

void
Diag867Collector::printCommissionRuleProcess(
        const CommissionRuleInfo& cri,
        CommissionValidationStatus rc)
{
  if (!_active)
    return;

  if (isDiagRuleMatch(cri.commissionId()))
  {
    if ((_detailedDiag.isDDPASSED && rc == PASS_CR) || _detailedDiag.isDDAMC)
      printCommissionsRuleInfoShort(cri, rc);
    else if (_detailedDiag.isDDINFO && isDiagRuleMatch(cri.commissionId()))
      printCommissionsRuleInfo(cri, rc);
  }
}

void
Diag867Collector::printCommissionProgramProcess(
    const CommissionProgramInfo& cpi,
    CommissionValidationStatus rc)
{
  if (!_active)
    return;

  if(isDiagProgramMatch(cpi.programId()))
  {
    if ((_detailedDiag.isDDPASSED && rc == PASS_CR) ||
        _detailedDiag.isDDAMC)
      printCommissionsProgramInfoShort(cpi, rc);
    else if (_detailedDiag.isDDINFO)
      printCommissionsProgramInfo(cpi, rc);
  }
}

void
Diag867Collector::printReuseMessage()
{
  if (!_active) return;
  if(isDetailedDiagnostic())
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "REUSING PREVALIDATED COMMISSION RULE INFO\n";
  }
}

void
Diag867Collector::printDetailedCommissionContractInfo(const CommissionContractInfo& cci)
{
  if (_detailedDiag.isDDINFO)
    printCommissionContractInfo(cci);
  else if(_detailedDiag.isDDAMC || _detailedDiag.isDDPASSED)
    printCommissionContractInfoShort(cci);
}

void
Diag867Collector::printSingleFcCommission(
    const FarePath& fpath,
    const FareUsage* fu,
    MoneyAmount amt,
    const amc::CommissionRuleData& crd)
{
  if (fu && !isDetailedDiagnostic())
  {
    printFinalFcCommission(fpath, *fu, amt, crd);
    _detailedDiag.anyComm = true;
  }
}

}
