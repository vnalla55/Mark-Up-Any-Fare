//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "Diagnostic/Diag602Collector.h"

#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ProcessTagInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/FareTypeTablePresenter.h"
#include "Rules/RuleUtil.h"

#include <string>

namespace tse
{
Diag602Collector& Diag602Collector::operator<< (const  PaxTypeFare& ptf)
{
  if (!_active || !_filterPassed)
    return *this;

  this->printLine();

  const Fare& fare = *ptf.fare();
  const FareMarket& fm = *ptf.fareMarket();

  this->setf(std::ios::left, std::ios::adjustfield);
  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  *this << std::setw(7) << fm.boardMultiCity() + fm.offMultiCity() << std::setw(9)
        << fare.fareClass() << std::setw(5) << fare.vendor() << std::setw(4) << fare.tcrRuleTariff()
        << "-" << std::setw(8) << fare.tcrRuleTariffCode() << std::setw(3) << fare.carrier()
        << std::setw(5) << fare.ruleNumber() << std::setw(8)
        << Money(fare.fareAmount(), fare.currency()) << " " << std::setw(3) << gd << std::setw(4)
        << ptf.fcaFareType() << std::setw(2) << fare.owrt() << std::setw(2)
        << (fare.directionality() == FROM ? "O" : fare.directionality() == TO ? "I" : " ")
        << "\n       ";

  *this << DiagnosticUtil::tcrTariffCatToString(ptf.tcrTariffCat()) << " TARIFF REPRICE FARE\n \n";

  return *this;
}

void
Diag602Collector::printPtiInfo(RepriceFareValidationResult r, const ProcessTagInfo& pti)
{
  if (!_active || !_filterPassed)
    return;

  DiagCollector& dc = *(DiagCollector*)this;
  const ReissueSequenceW& tbl988(*pti.reissueSequence());

  dc << "R3 ITEM " << pti.record3()->itemNo();

  if (!tbl988.orig())
  {
    dc << " NO T988\n";
    return;
  }

  dc << " T988 ITEM " << pti.itemNo() << " SEQ " << pti.seqNo() << " ";

  switch (r)
  {
  case REPRICE_PASS:
    dc << "REPRICE PASS";
    break;
  case RULE_INDICATOR:
    dc << "RULE INDICATOR: " << tbl988.ruleInd() << "  "
       << "RULE NUMBER: " << tbl988.ruleNo();
    break;
  case CARRIER_APPLICATION_TBL:
    dc << "CARRIER APPLICATION TABLE: " << tbl988.fareCxrApplTblItemNo();
    break;
  case TARIFF_NUMBER:
    dc << "TARIFF NUMBER: " << tbl988.ruleTariffNo();
    break;
  case RULE_NUMBER:
    dc << "RULE NUMBER: " << tbl988.ruleNo();
    break;
  case EXCLUDE_PRIVATE:
  {
    dc << "EXCLUDE PRIVATE: " << tbl988.excludePrivate();
    *this << "\n       " << DiagnosticUtil::tcrTariffCatToString(pti.paxTypeFare()->tcrTariffCat())
          << " TARIFF EXCHANGE FARE";
    break;
  }
  case FARE_CLASS_CODE:
    dc << "FARE CLASS INDICATOR: " << tbl988.fareTypeInd() << " CLASS CODE: " << tbl988.fareClass();
    break;
  case FARE_TYPE_CODE:
    dc << "FARE TYPE INDICATOR: " << tbl988.fareTypeInd() << " FARE TYPE: " << tbl988.fareType();
    break;
  case SAME_INDICATOR:
    dc << "SAME INDICATOR: " << tbl988.sameInd();
    break;
  case FARE_TYPE_TABLE:
  {
    FareTypeTablePresenter fttp(*this, static_cast<RexBaseTrx&>(*_trx));
    fttp.printFareType(pti.paxTypeFare()->vendor(),
                       pti.reissueSequence()->fareTypeTblItemNo(),
                       pti.fareMarket()->ruleApplicationDate());
    break;
  }
  case FARE_AMOUNT:
    dc << "FARE AMOUNT: " << tbl988.fareAmtInd();
    break;
  case NORMAL_SPECIAL:
    dc << "NORMAL/SPECIAL: " << tbl988.normalspecialInd();
    break;
  case OWRT_INDICATOR:
    dc << "OWRT: " << tbl988.owrt();
    break;
  }

  dc << std::endl;
}

Diag602Collector& Diag602Collector::operator<<(RepriceFareValidationResult r)
{
  if (!_active || !_filterPassed)
    return *this;

  *this << " \n";

  switch (r)
  {
  case REPRICE_PASS:
    *this << "REPRICE PASS\n";
    break;
  case RULE_INDICATOR:
    *this << "FARES CHECK FAILED ON RULE NUMBER\n";
    break;
  case CARRIER_APPLICATION_TBL:
    *this << "FARES CHECK FAILED ON CARRIER APPLICATION TABLE\n";
    break;
  case TARIFF_NUMBER:
    *this << "FARES CHECK FAILED ON TARIFF NUMBER\n";
    break;
  case RULE_NUMBER:
    *this << "FARES CHECK FAILED ON RULE NUMBER\n";
    break;
  case EXCLUDE_PRIVATE:
    *this << "FARES CHECK FAILED ON EXCLUDE PRIVATE\n";
    break;
  case FARE_CLASS_CODE:
    *this << "FARES CHECK FAILED ON FARE CLASS CODE\n";
    break;
  case FARE_TYPE_CODE:
    *this << "FARES CHECK FAILED ON FARE TYPE\n";
    break;
  case SAME_INDICATOR:
    *this << "FARES CHECK FAILED ON SAME\n";
    break;
  case FARE_TYPE_TABLE:
    *this << "FARES CHECK FAILED ON FARE TYPE TABLE\n";
    break;
  case FARE_AMOUNT:
    *this << "FARES CHECK FAILED ON FARE AMOUNT\n";
    break;
  case NORMAL_SPECIAL:
    *this << "FARES CHECK FAILED ON NORMAL/SPECIAL\n";
    break;
  case OWRT_INDICATOR:
    *this << "FARES CHECK FAILED ON OWRT\n";
    break;
  }

  return *this;
}

void
Diag602Collector::initializeFilter(PricingTrx& trx, const PaxTypeFare& ptf)
{
  _filterPassed = true;
  std::map<std::string, std::string>::const_iterator i = trx.diagnostic().diagParamMap().find(
                                                         Diagnostic::FARE_MARKET),
                                                     e = trx.diagnostic().diagParamMap().end();
  if (i != e)
  {
    LocCode boardCity = i->second.substr(0, 3);
    LocCode offCity = i->second.substr(3, 3);
    const FareMarket& fm = *ptf.fareMarket();

    if (((fm.origin()->loc() != boardCity) && (fm.boardMultiCity() != boardCity)) ||
        ((fm.destination()->loc() != offCity) && (fm.offMultiCity() != offCity)))
    {
      _filterPassed = false;
    }
  }

  if (!_filterPassed)
    return;

  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  if (i != e && !i->second.empty())
    _filterPassed = RuleUtil::matchFareClass(i->second.c_str(), ptf.fareClass().c_str());
}

void
Diag602Collector::displayErrorMessage(const MoneyAmount& excAmount, const MoneyAmount& newAmount)
{
  if (!_active || !_filterPassed)
    return;

  *this << " FAILED: EXCHANGE FARE AMOUNT: " << excAmount << " NUC IS GREATER THAN\n";
  *this << " REPRICE SOLUTION FARE AMOUNT: " << newAmount << " NUC\n";
}
}
