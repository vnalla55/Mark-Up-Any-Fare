//----------------------------------------------------------------------------
//  File:        Diag316Collector.C
//
//  Description: Diagnostic 316 (Penalties)
//
//  Updates:
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

#include "Diagnostic/Diag316Collector.h"

#include "Common/DateTime.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "Rules/RuleValidationContext.h"

#include <iomanip>

using namespace std;

namespace tse
{
void
Diag316Collector::writeHeader(PaxTypeFare& paxTypeFare,
                              const CategoryRuleInfo& cri,
                              const PenaltyInfo& penaltyInfo,
                              PricingTrx& trx,
                              bool farePhase)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);

    dc << "******************************************************" << endl;
    dc << "*   CATEGORY 16 PENALTIES APPLICATION DIAGNOSTICS    *" << endl;
    dc << "******************************************************" << endl;

    if (farePhase)
      dc << "PHASE: FARE VALIDATOR    R3 ITEM NUMBER: ";
    else
      dc << "PHASE: PRICING UNIT     R3 ITEM NUMBER: ";

    dc << penaltyInfo.itemNo() << endl;
    dc << paxTypeFare.fareMarket()->origin()->loc() << " "
       << paxTypeFare.fareMarket()->destination()->loc() << " " << paxTypeFare.fareClass()
       << "     ";
    dc << "R2:FARERULE    :  " << cri.vendorCode() << " " << cri.tariffNumber() << " "
       << cri.carrierCode() << " " << cri.ruleNumber() << endl;

    dc.flushMsg();

    PricingOptions* options = trx.getOptions();

    if (options->isNoPenalties())
      dc << "EXCLUDE PENALTIES: YES";
    else
      dc << "EXCLUDE PENALTIES: NO";

    dc << endl;

    dc << "CATEGORY 16 RULE DATA";
    dc << endl;
    dc << "TICKET/RESERVATIONS RESTRICTIONS - ";

    if (penaltyInfo.noRefundInd() == Diag316Collector::NO_APPLICATION)
      dc << "N/A";
    else if (penaltyInfo.noRefundInd() == 'B')
      dc << "NO-REFUND/NO CHANGE";
    else if (penaltyInfo.noRefundInd() == 'N')
      dc << "NO CHANGE";
    else if (penaltyInfo.noRefundInd() == 'X')
      dc << "NO-REFUND";

    dc << endl;

    dc.precision(penaltyInfo.penaltyNoDec1());

    dc << "PENALTIES: " << endl;
    dc << "CHANGE OF ITINERARY REQUIRING REISSUE OF TICKET: - ";

    if (penaltyInfo.penaltyReissue() == Diag316Collector::NO_APPLICATION)
      dc << "N/A";
    else if (penaltyInfo.penaltyReissue() == 'X')
      dc << "APPLIES";

    dc << endl;

    dc << "CHANGE NOT REQUIRING REISSUE OF TICKET: - ";
    if (penaltyInfo.penaltyNoReissue() == Diag316Collector::NO_APPLICATION)
      dc << "N/A";
    else if (penaltyInfo.penaltyNoReissue() == 'X')
      dc << "APPLIES";

    dc << endl;

    dc << "PENALTY VOLUNTARY APPL: - ";

    if (penaltyInfo.volAppl() == Diag316Collector::NO_APPLICATION)
      dc << "N/A";
    else if (penaltyInfo.volAppl() == 'X')
      dc << "APPLIES";

    dc << endl;

    dc << "PENALTY CANCEL REFUND APPL: - ";

    if (penaltyInfo.cancelRefundAppl()  == Diag316Collector::NO_APPLICATION)
      dc << "N/A";
    else if (penaltyInfo.cancelRefundAppl() == 'X')
      dc << "APPLIES";

    dc << endl;

    dc << "PENALTY AMT1 - ";
    dc << penaltyInfo.penaltyAmt1();
    dc << endl;

    dc.precision(penaltyInfo.penaltyNoDec2());

    dc << "PENALTY CUR1 - ";
    dc << penaltyInfo.penaltyCur1();
    dc << endl;

    dc << "PENALTY AMT2 - ";
    dc << penaltyInfo.penaltyAmt2();
    dc << endl;

    dc.precision(penaltyInfo.penaltyPercentNoDec());

    dc << "PENALTY CUR2 - ";
    dc << penaltyInfo.penaltyCur2();
    dc << endl;

    dc << "PENALTY PERCENT - ";
    dc << penaltyInfo.penaltyPercent();
    dc << endl;

    dc.flushMsg();
  }
}

void
Diag316Collector::printFlexFareNoPenalties(const flexFares::ValidationStatusPtr& validationStatus,
                                           const Record3ReturnTypes& validationResult,
                                           const RuleValidationContext& context)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);

    dc << "FLEX FARE VALIDATION: ";
    dc << endl;

    dc << "  GROUP ID: ";
    dc << context._groupId;
    dc << endl;

    dc << "  NO PENALTY: ";

    Record3ReturnTypes result = validationResult;
    if (RuleValidationContext::FARE_MARKET == context._contextType)
    {
      result = validationStatus->getStatusForAttribute<flexFares::NO_PENALTIES>();
    }
    dc << ShoppingUtil::getFlexFaresValidationStatus(result);

    dc << endl;
    dc.flushMsg();
  }
}
}
