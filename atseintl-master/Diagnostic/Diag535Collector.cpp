//----------------------------------------------------------------------------
//  File:        Diag535Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 535
//               NegotiatedFareRule data processing for IT/BT ticketing.
//
//  Updates:
//          10/25/2004  VK - create.
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

#include "Diagnostic/Diag535Collector.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PrintOption.h"
#include "DBAccess/ValueCodeAlgorithm.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/TicketingEndorsement.h"
#include "Rules/ValueCodeUtil.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

FALLBACK_DECL(fallbackProcessEmptyTktEndorsements)
FALLBACK_DECL(fallbackBSPMt3For1S)
FALLBACK_DECL(fallbackEndorsementsRefactoring)

void
Diag535Collector::diag535Request(PricingTrx& trx, const FarePath& farePath)
{
  if (!_active)
    return;

  Diag535Collector& dc = *this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "************************************************************\n";
  dc << "*     ATSE REQUEST DATA FOR NEGOTIATED FARE RULE PROCESS   *\n";
  dc << "************************************************************\n";

  char ind;
  std::string str;

  if (trx.getRequest()->ticketingAgent()->abacusUser())
    str = "ABACUS";
  else if (trx.getRequest()->ticketingAgent()->axessUser())
    str = "AXESS";
  else if (trx.getRequest()->ticketingAgent()->infiniUser())
    str = "INFINI";
  else if (trx.getRequest()->ticketingAgent()->cwtUser())
    str = "CWT";

  if (str.empty())
  {
    if ((trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()))
    { // for hosted carriers we display carrier code
      if (trx.billing() && (!trx.billing()->partitionID().empty()))
        str = trx.billing()->partitionID();
    }
    else
      str = "SABRE";
  }
  dc << " USER TYPE                       : " << str << '\n';

  if (trx.getRequest()->isTicketEntry())
    ind = 'Y';
  else
    ind = 'N';
  dc << " TICKETING ENTRY                 : " << ind << '\n';

  if (trx.getRequest()->isFormOfPaymentCard())
    ind = 'Y';
  else
    ind = 'N';
  dc << " CREDIT CARD FOP                 : " << ind << '\n';

  if (trx.getOptions()->isCat35Sell())
    ind = 'Y';
  else
    ind = 'N';
  dc << " PRINT SELLING CATEGORY 35 FARE  : " << ind << '\n';

  if (trx.getOptions()->isCat35Net())
    ind = 'Y';
  else
    ind = 'N';
  dc << " PRINT NET CATEGORY 35 FARE      : " << ind << '\n';

  CurrencyCode paymentCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    paymentCurrency = trx.getOptions()->currencyOverride();
  }
  dc << " PAYMENT CURRENCY                : " << paymentCurrency << '\n';

  ind = 'N';
  bool gnrEnabledUser = false;
  bool jointVenGDSCus = false;
  if ((trx.getRequest()->ticketingAgent()->abacusUser()) ||
      (trx.getRequest()->ticketingAgent()->axessUser()) ||
      (trx.getRequest()->ticketingAgent()->infiniUser()))
    jointVenGDSCus = true;
  else
  {
    if (!trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    {
      gnrEnabledUser = trx.isGNRAllowed();
    }
  }
  if (!fallback::fallbackBSPMt3For1S(&trx) ? TrxUtil::isBspUser(trx) :
       ((jointVenGDSCus || gnrEnabledUser) && TrxUtil::isBspUser(trx)))
  {
    NegPaxTypeFareRuleData* negPaxTypeFare = nullptr;

    for (PricingUnit* pu : farePath.pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        if (!fu->paxTypeFare())
          continue;
        const NegFareRest* negFareRest =
            NegotiatedFareRuleUtil::getCat35Record3(fu->paxTypeFare(), negPaxTypeFare);
        if (negFareRest)
        {
          // for 1B/1J/1F users, it is 'Y' when MT2,MT3 and MT1  fares in the combination.
          if (jointVenGDSCus)
          {
            if ((negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_1) ||
                (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2) ||
                (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3))
              ind = 'Y';
          }
          // for 1S GNR users it is 'Y'  when MT2 fares in the  combination
          else if (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2)
          {
             ind = 'Y';
          }
          else
          {
             // for 1S Users  it is 'Y'  when MT3 fares in the combination
             if (!fallback::fallbackBSPMt3For1S(&trx) &&
                 negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3)
               ind = 'Y';
          }
        }
        if (ind == 'Y')
          break;
      }
      if (ind == 'Y')
        break;
    }
  }
  dc << " NET REMIT PROCESS               : " << ind << '\n';
}

void
Diag535Collector::displayCoupons(PricingTrx& trx, const NegFareRest& negFareRest)
{
  if (_active)
  {
    Diag535Collector& dc = *this;
    bool axessCat35Request = false;
    if (trx.getRequest()->ticketingAgent()->axessUser() &&
        (trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()))
    {
      axessCat35Request = true;
    }

    if (negFareRest.couponInd1() == NegotiatedFareRuleUtil::AUDIT_COUPON)
    {
      dc << " AUDIT COUPON                    : " << '\n';
    }
    if (negFareRest.couponInd1() == NegotiatedFareRuleUtil::PSG_COUPON)
    {
      dc << " PASSENGER COUPON                : " << '\n';
    }
    dc << "     TOUR BOX TYPE    - " << negFareRest.tourBoxCodeType1() << '\n';
    dc << "     TOUR BOX CODE    - " << negFareRest.tourBoxCode1() << '\n';
    dc << "     TKT DESIGNATOR   - " << negFareRest.tktDesignator1() << '\n';
    dc << "     FARE BOX TEXT    - " << negFareRest.fareBoxText1() << '\n';
    dc << "     FARE BOX CUR/AMT - ";
    if (!negFareRest.cur11().empty())
    {
      dc << Money(negFareRest.fareAmt1(), negFareRest.cur11()).toString(negFareRest.noDec11());
    }
    dc << '\n';

    if (negFareRest.netRemitMethod() == RuleConst::NRR_METHOD_2 ||
        negFareRest.netRemitMethod() == RuleConst::NRR_METHOD_3 || axessCat35Request)
    {
      dc << "     TICKETED FARE DATA:" << '\n';
      dc << "         INDICATOR      - " << negFareRest.tktFareDataInd1() << '\n';
      dc << "         OW/RT          - " << negFareRest.owrt1() << '\n';
      std::string gd1;
      globalDirectionToStr(gd1, negFareRest.globalDir1());
      dc << "         GLOBAL IND     - " << gd1 << '\n';

      if (negFareRest.ruleTariff1() == 0)
        dc << "         RULE TARIFF    - " << '\n';
      else
        dc << "         RULE TARIFF    - " << negFareRest.ruleTariff1() << '\n';

      dc << "         CARRIER        - " << negFareRest.carrier11() << '\n';
      dc << "         RULE NUMBER    - " << negFareRest.rule1() << '\n';
      dc << "         FARE CLASS     - " << negFareRest.fareClass1() << '\n';
      dc << "         FARE TYPE      - " << negFareRest.fareType1() << '\n';
      dc << "         SEASON TYPE    - " << negFareRest.seasonType1() << '\n';
      dc << "         DOW TYPE       - " << negFareRest.dowType1() << '\n';
      dc << "         BETWEEN CITY   - " << negFareRest.betwCity1() << '\n';
      dc << "         AND CITY       - " << negFareRest.andCity1() << '\n';
    }

    if (negFareRest.noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS)
    {
      if (negFareRest.couponInd2() == NegotiatedFareRuleUtil::AUDIT_COUPON)
      {
        dc << " AUDIT COUPON                    : " << '\n';
      }
      if (negFareRest.couponInd2() == NegotiatedFareRuleUtil::PSG_COUPON)
      {
        dc << " PASSENGER COUPON                : " << '\n';
      }
      dc << "     TOUR BOX TYPE    - " << negFareRest.tourBoxCodeType2() << '\n';
      dc << "     TOUR BOX CODE    - " << negFareRest.tourBoxCode2() << '\n';
      dc << "     TKT DESIGNATOR   - " << negFareRest.tktDesignator2() << '\n';
      dc << "     FARE BOX TEXT    - " << negFareRest.fareBoxText2() << '\n';
      dc << "     FARE BOX CUR/AMT - ";
      if (!negFareRest.cur21().empty())
      {
        dc << Money(negFareRest.fareAmt2(), negFareRest.cur21()).toString(negFareRest.noDec21());
      }
      dc << '\n';

      if (negFareRest.netRemitMethod() == RuleConst::NRR_METHOD_2 ||
          negFareRest.netRemitMethod() == RuleConst::NRR_METHOD_3 || axessCat35Request)
      {
        dc << "     TICKETED FARE DATA:" << '\n';
        dc << "         INDICATOR      - " << negFareRest.tktFareDataInd2() << '\n';
        dc << "         OW/RT          - " << negFareRest.owrt2() << '\n';
        std::string gd2;
        globalDirectionToStr(gd2, negFareRest.globalDir2());
        dc << "         GLOBAL IND     - " << gd2 << '\n';

        if (TrxUtil::cat35LtypeEnabled(trx) && negFareRest.ruleTariff2() == 0)
          dc << "         RULE TARIFF    - " << '\n';
        else
          dc << "         RULE TARIFF    - " << negFareRest.ruleTariff2() << '\n';

        dc << "         CARRIER        - " << negFareRest.carrier21() << '\n';
        dc << "         RULE NUMBER    - " << negFareRest.rule2() << '\n';
        dc << "         FARE CLASS     - " << negFareRest.fareClass2() << '\n';
        dc << "         FARE TYPE      - " << negFareRest.fareType2() << '\n';
        dc << "         SEASON TYPE    - " << negFareRest.seasonType2() << '\n';
        dc << "         DOW TYPE       - " << negFareRest.dowType2() << '\n';
        dc << "         BETWEEN CITY   - " << negFareRest.betwCity2() << '\n';
        dc << "         AND CITY       - " << negFareRest.andCity2() << '\n';
      }
    }
  }
}

void
Diag535Collector::displayHeader()
{
  Diag535Collector& dc = *this;

  dc << "************************************************************\n";
  dc << "*         ATSE NEGOTIATED FARE RULE PROCESS                *\n";
  dc << "************************************************************\n";
}

void
Diag535Collector::displayPaxTypeFare(const PaxTypeFare& paxTypeFare)
{
  Diag535Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << setw(3) << paxTypeFare.fareMarket()->origin()->loc();
  dc << "-";
  dc << setw(6) << paxTypeFare.fareMarket()->destination()->loc();
  dc << setw(5) << paxTypeFare.fare()->carrier();

  dc << setw(11) << paxTypeFare.fare()->fareClass();

  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";
  dc << std::setw(12) << " DISP TYPE -";
  dc << setw(5) << paxTypeFare.fcaDisplayCatType();
  if (paxTypeFare.isNegotiated())
    dc << setw(3) << (paxTypeFare.isFareByRule() ? "C25" : "C35");
  dc << "\n";
}

void
Diag535Collector::displayCat27TourCode(PricingTrx& trx,
                                       const PaxTypeFare* ptf,
                                       const NegFareRest* negFareRest)
{
  if (!negFareRest)
    return;
  string tourCode;
  RuleUtil::getCat27TourCode(ptf, tourCode);
  if (!tourCode.empty())
    *this << " CATEGORY 27 TOUR CODE           : " << tourCode << "\n";
}

void
Diag535Collector::diag535Collector(PricingTrx& trx, const Itin& itin, const FareUsage& fu)
{
  if (_active)
  {
    Diag535Collector& dc = *this;
    const PaxTypeFare* ptf = fu.paxTypeFare();

    dc.setf(std::ios::left, std::ios::adjustfield);

    displayHeader();
    displayPaxTypeFare(*ptf);

    if (!ptf->isNegotiated())
    {
      dc << " NOT NEGOTIATED FARE " << '\n';
      dc << "************************************************************\n";
      dc.flushMsg();
      return;
    }
    const NegPaxTypeFareRuleData* negPaxTypeFare = ptf->getNegRuleData();

    if (negPaxTypeFare == nullptr)
    {
      dc << " NOT NEGOTIATED FARE -WRONG PAX TYPE FARE RULE DATA" << '\n';
      dc << "************************************************************\n";
      return;
    }
    const NegFareRest* negFareRest =
        dynamic_cast<const NegFareRest*>(negPaxTypeFare->ruleItemInfo());

    if (negFareRest == nullptr)
    {
      dc << " NOT NEGOTIATED FARE - WRONG NEGOTIATED FARE RULE DATA " << '\n';
      dc << "************************************************************\n";
      dc.flushMsg();
      return;
    }

    dc << " NEGOTIATED FARE RULE DATA: RECORD3 - " << negFareRest->itemNo() << '\n';
    dc << '\n';

    dc << " NEG FARE SECURITY TABLE NUMBER  : " << negFareRest->negFareSecurityTblItemNo() << '\n';
    dc << " NEG FARE CALCULATE TABLE NUMBER : " << negFareRest->negFareCalcTblItemNo() << '\n';
    if (negPaxTypeFare->tktIndicator() == 'N')
    {
      dc << " TICKET PERMISSION INDICATOR     : NOT PERMITTED" << '\n';
    }

    bool axessCat35Request = false;
    if (trx.getRequest()->ticketingAgent()->axessUser() &&
        (trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()))
    {
      axessCat35Request = true;
    }

    if (axessCat35Request)
    {
      dc << " COMMISSION PERCENT              : ";
      if (negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL)
        dc << "NOT APPL" << '\n';
      else
        dc << negFareRest->commPercent() << '\n';

      if (!negFareRest->cur1().empty())
      {
        dc << " COMMISSIONS 1                 *X: ";
        dc << std::setw(8) << Money(negFareRest->commAmt1(), negFareRest->cur1()) << '\n';
      }
      if (!negFareRest->cur2().empty())
      {
        dc << " COMMISSIONS 2                 *X: ";
        dc << std::setw(8) << Money(negFareRest->commAmt2(), negFareRest->cur2()) << '\n';
      }
      if (!negFareRest->cur1().empty() || !negFareRest->cur2().empty())
        dc << "                      ** NOTE: *X IS IGNORED FOR AXESS USER"
           << "\n";
    }
    else // old format...
    {
      if (negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL)
      {
        if (!negFareRest->cur1().empty())
        {
          dc << " COMMISSIONS 1 : ";
          dc << std::setw(8) << Money(negFareRest->commAmt1(), negFareRest->cur1()) << '\n';
        }
        else if (!negFareRest->cur2().empty())
        {
          dc << " COMMISSIONS 2 : ";
          dc << std::setw(8) << Money(negFareRest->commAmt2(), negFareRest->cur2()) << '\n';
        }
        else
        {
          dc << " COMMISSIONS ARE NOT FOUND " << '\n';
        }
      }
      else
      {
        dc << " COMMISSION PERCENT              : " << negFareRest->commPercent() << '\n';
      }
    }

    dc << " NET REMIT METHOD                : " << negFareRest->netRemitMethod() << '\n';
    dc << " NET GROSS INDICATOR             : " << negFareRest->netGrossInd() << '\n';
    dc << " BAGGAGE TYPE                    : " << negFareRest->bagTypeInd() << '\n';
    dc << " BAGGAGE NUMBER                  : " << negFareRest->bagNo() << '\n';

    if (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2 ||
        negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3 || axessCat35Request)
    {
      dc << " CARRIER RESTRICTION             : ";
      if (!negFareRest->carrier().empty())
      {
        if (negFareRest->tktAppl() == ' ')
        {
          dc << "MUST BE ";
        }
        else
        {
          dc << "MUST NOT BE ";
        }
        dc << negFareRest->carrier();
      }
      dc << '\n';
    }

    dc << " NUMBER OF SEGMENTS              : " << negFareRest->noSegs() << '\n';

    if (negFareRest->noSegs() != 0)
    {
      displayCoupons(trx, *negFareRest);
    }

    displayCat27TourCode(trx, ptf, negFareRest);
    displayTourCodeCombInd(negPaxTypeFare->negFareRestExt(), false);

    if (negPaxTypeFare->fareProperties())
      displayFareProperties(negPaxTypeFare->fareProperties(), false);

    if (negPaxTypeFare->valueCodeAlgorithm())
      displayValueCodeAlgorithm(*negPaxTypeFare->valueCodeAlgorithm(), false);

    if (TrxUtil::optimusNetRemitEnabled(trx))
    {
      displayStaticValueCode(
          negPaxTypeFare->negFareRestExt(), ValueCodeUtil::getCat18ValueCode(fu), false);
      if (ptf->vendor() == SMF_ABACUS_CARRIER_VENDOR_CODE &&
          negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_BLANK)
        displayPrintOption(negPaxTypeFare, false);
    }

    if (shouldDisplayTFDSPCInd(negPaxTypeFare, negFareRest->tktFareDataInd1()))
      dc << " FARE BASIS/AMOUNT IND : " << negPaxTypeFare->negFareRestExt()->fareBasisAmtInd()
         << "\n";

    if (tktFareDataSegExists(negPaxTypeFare))
      displayTfdpsc(negPaxTypeFare);

    if (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
        negFareRest->negFareCalcTblItemNo() == 0 && negFareRest->tourBoxCode1().empty() &&
        negFareRest->tourBoxCode2().empty() && negFareRest->tktDesignator1().empty() &&
        negFareRest->tktDesignator2().empty() && negFareRest->fareBoxText1().empty() &&
        negFareRest->fareBoxText2().empty() && negFareRest->commAmt1() == 0 &&
        negFareRest->commAmt2() == 0 && negFareRest->commPercent() == RuleConst::PERCENT_NO_APPL &&
        negFareRest->bagTypeInd() == BLANK)
    {
      dc << " **CATEGORY 35 IS USED FOR SECURITY ONLY - NO TKT DATA**" << '\n';
    }

    if (!TrxUtil::optimusNetRemitEnabled(trx))
    {
      if (negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2 ||
          negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_3 || axessCat35Request)
      {
        dc << '\n';
        if (axessCat35Request)
        {
          dc << " ENDORSEMENT FROM CAT18 : " << '\n';
        }
        else
        {
          dc << " VALUE CODE FROM CAT18 : " << '\n';
        }

        if (fu.tktEndorsement().size() >= 1)
        {
          dc << " ";

          TicketingEndorsement tktEndo;
          TicketEndorseLine* line;
          if (!fallback::fallbackEndorsementsRefactoring(&trx))
          {
            line = tktEndo.sortAndGlue(trx, itin, const_cast<FareUsage&>(fu), EndorseCutter());
          }
          else
          {
            line = tktEndo.sortAndGlue(trx, itin, const_cast<FareUsage&>(fu),
                                       EndorseCutterUnlimited());
          }

          if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
          {
            if (line)
              dc << line->endorseMessage;
            dc << '\n';
          }
          else
          {
            dc << line->endorseMessage << '\n';
          }
        }
      }
    }
    if ((TrxUtil::isCat35TFSFEnabled(trx) &&
         (ptf->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          (ptf->fcaDisplayCatType() == RuleConst::SELLING_FARE &&
           negFareRest->negFareCalcTblItemNo() != 0) ||
          (ptf->fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD &&
           negFareRest->negFareCalcTblItemNo() != 0 && !negPaxTypeFare->isT979FareIndInRange()))) &&
        negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_BLANK &&
        (negFareRest->fareBoxText1() == NegotiatedFareRuleUtil::IT_TICKET ||
         negFareRest->fareBoxText1() == NegotiatedFareRuleUtil::BT_TICKET))
    {
      dc << " ** CAT 35 TFSF LOGIC APPLIED**" << '\n';
    }

    dc << "************************************************************\n";
    dc.flushMsg();
    return;
  }
}

void
Diag535Collector::diag535Collector(const PaxTypeFare& paxTypeFare)
{
  if (_active)
  {
    Diag535Collector& dc = *this;
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " NET REMIT PUBLISHED FARE:" << '\n';
    dc << " ";
    dc << setw(3) << paxTypeFare.fareMarket()->origin()->loc();
    dc << "-";
    dc << setw(6) << paxTypeFare.fareMarket()->destination()->loc();
    dc << setw(5) << paxTypeFare.fare()->carrier();
    dc << setw(11) << paxTypeFare.fare()->fareClass();
    dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << '\n';

    dc << " RULE-" << setw(5) << paxTypeFare.ruleNumber();
    dc << "RULE TARIFF-" << setw(4) << paxTypeFare.tcrRuleTariff();
    dc << "OWRT-" << setw(3) << paxTypeFare.owrt();
    std::string gd;
    globalDirectionToStr(gd, paxTypeFare.globalDirection());
    dc << "GI-" << setw(3) << gd;
    dc << "FT-" << setw(4) << paxTypeFare.fcaFareType();
    dc << "ST-" << setw(3) << paxTypeFare.fcaSeasonType();
    dc << "DT-" << setw(3) << paxTypeFare.fcaDowType() << '\n';

    dc << "************************************************************\n";
    dc.flushMsg();
    return;
  }
}

void
Diag535Collector::diag535Message(PricingTrx& trx,
                                 const NegotiatedFareRuleUtil::WARNING_MSG warningMsg)
{
  if (_active)
  {
    Diag535Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " WARNING MESSAGE CODE:";
    if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_TOUR_CODE)
    {
      dc << " MULTIPLE TOUR CODES" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::TOUR_CODE_NOT_FOUND)
    {
      dc << " TOUR CODE NOT FOUND" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_VALUE_CODE)
    {
      dc << " MULTIPLE VALUE CODES" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_PRINT_OPTION)
    {
      dc << " MULTIPLE PRINT OPTIONS" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::INVALID_ITBT_PSG_COUPON)
    {
      dc << " INVALID ITBT PSG COUPON" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::EMPTY_ITBT_PSG_COUPON)
    {
      dc << " EMPTY ITBT PSG COUPON" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::ITBT_BSP_NOT_BLANK)
    {
      dc << " ITBT BSP NOT BLANK" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::NOT_ITBT)
    {
      dc << " NOT ITBT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_FARE_BOX)
    {
      dc << " MULTIPLE FARE BOX" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_BSP)
    {
      dc << " MULTIPLE BSP" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_NET_GROSS)
    {
      dc << " MULTIPLE NET GROSS" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MIX_FARES)
    {
      dc << " MIX FARES" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE)
    {
      dc << " FARES NOT COMBINABLE" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::NET_SELLING_CONFLICT)
    {
      dc << " NET SELLING CONFLICT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MULTIPLE_COMMISSION)
    {
      dc << " MULTIPLE COMMISSION" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MIX_COMMISSION)
    {
      dc << " MIX COMMISSION" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::ISSUE_SEPARATE_TKT)
    {
      dc << " ISSUE SEPARATE TKT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::INVALID_NET_REMIT_FARE)
    {
      dc << " INVALID NET REMIT FARE" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::NO_NET_FARE_AMOUNT)
    {
      dc << " NO NET FARE AMOUNT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::FARES_NOT_COMBINABLE_NO_NET)
    {
      dc << " UNABLE TO AUTO TICKET - INVALID COMMISSION AMOUNT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::INVALID_NET_REMIT_COMM)
    {
      dc << " INVALID NET REMIT COMMISSION" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::AUTO_TKT_NOT_PERMITTED)
    {
      dc << " AUTO TICKETING NOT PERMITTED" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::NET_FARE_AMOUNT_EXCEEDS_FARE)
    {
      dc << " NET AMOUNT EXCEEDS FARE" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MIXED_FARES_COMBINATION)
    {
      dc << " MIXED FARES INCL NET REMIT" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::CONFLICTING_TFD_BYTE101)
    {
      dc << "NET REMIT/CONFLICT IN FARE IND BYTE 101" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::TFD_RETRIEVE_FAIL)
    {
      dc << "NET REMIT/UNA TO VAL TKT FARE DATA" << '\n';
    }
    else if (warningMsg == NegotiatedFareRuleUtil::MIXED_FARE_BOX_AMT)
    {
      dc << " MIXED FARE BOX CUR/AMT" << '\n';
    }
    else
    {
      dc << " SYSTEM ERROR" << '\n';
    }
    dc << "************************************************************\n";
    dc.flushMsg();
    return;
  }
}

void
Diag535Collector::displayHeader(bool farePathScope)
{
  if (_active)
  {
    displayHeader();
    *this << (farePathScope ? "FARE PATH" : "PRICING UNIT") << " SCOPE\n \n";
  }
}

void
Diag535Collector::displayPaxTypeFare(const PaxTypeFare& paxTypeFare,
                                     const NegFareRest* negFareRest,
                                     bool shouldDisplayTfdpsc,
                                     bool farePathScope,
                                     const PricingTrx& trx,
                                     const FarePath* fp)
{
  if (_active)
  {
    Diag535Collector& dc = *this;

    displayPaxTypeFare(paxTypeFare);

    if (paxTypeFare.isNegotiated())
    {
      if (negFareRest)
      {
        dc << "  NEGOTIATED FARE RULE DATA: RECORD3 - " << negFareRest->itemNo() << "\n";
        dc << "  NET REMIT METHOD : " << negFareRest->netRemitMethod() << "\n";

        displayCommission(negFareRest);
        displayTourCode(&paxTypeFare, negFareRest);

        const NegPaxTypeFareRuleData* negPaxTypeFare = paxTypeFare.getNegRuleData();

        if (negPaxTypeFare)
          displayTourCodeCombInd(negPaxTypeFare->negFareRestExt(), true);

        if (farePathScope && negPaxTypeFare && TrxUtil::optimusNetRemitEnabled(trx))
        {
          if (negPaxTypeFare->fareProperties())
            displayFareProperties(negPaxTypeFare->fareProperties(), true);
          if (negPaxTypeFare->valueCodeAlgorithm())
            displayValueCodeAlgorithm(*negPaxTypeFare->valueCodeAlgorithm(), true);
          displayStaticValueCode(
              negPaxTypeFare->negFareRestExt(), getCat18ValueCode(fp, paxTypeFare), true);
        }

        if (negPaxTypeFare && TrxUtil::optimusNetRemitEnabled(trx) &&
            paxTypeFare.vendor() == SMF_ABACUS_CARRIER_VENDOR_CODE &&
            negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_BLANK)
          displayPrintOption(negPaxTypeFare, true);

        if (shouldDisplayTFDSPCInd(negPaxTypeFare, negFareRest->tktFareDataInd1()))
          displayFareBasisAmountInd(negPaxTypeFare->negFareRestExt());
        else
          displayTFDByte101(negFareRest);

        if (shouldDisplayTfdpsc && tktFareDataSegExists(negPaxTypeFare))
        {
          displayTfdpsc(negPaxTypeFare);
          displayMatchedSequencesResult(fp, paxTypeFare);
        }
      }
    }
    else
    {
      dc << "  NOT NEGOTIATED FARE"
         << "\n";
    }
    dc << " \n";
  }
}

void
Diag535Collector::displayValidationResult(bool result, const char* warningMsg)
{
  if (_active)
  {
    Diag535Collector& dc = *this;

    dc << "************************************************************\n";
    dc << " VALIDATION RESULT: ";

    if (result)
      dc << "PASS";
    else
      dc << "FAIL - " << warningMsg;

    dc << "\n************************************************************\n";
  }
}

void
Diag535Collector::displayCommission(const NegFareRest* negFareRest)
{
  bool needSlash = false;

  Diag535Collector& dc = *this;

  dc << "  COMMISSION : ";

  if (negFareRest->netGrossInd() != ' ')
  {
    dc << negFareRest->netGrossInd();
    needSlash = true;
  }

  if (negFareRest->commPercent() != RuleConst::PERCENT_NO_APPL)
  {
    if (needSlash)
      dc << " / ";
    dc << negFareRest->commPercent() << "PCT";
    needSlash = true;
  }

  if (!negFareRest->cur1().empty())
  {
    if (needSlash)
      dc << " / ";
    dc << std::setw(8) << Money(negFareRest->commAmt1(), negFareRest->cur1());
    needSlash = true;
  }

  if (!negFareRest->cur2().empty())
  {
    if (needSlash)
      dc << " / ";
    dc << std::setw(8) << Money(negFareRest->commAmt2(), negFareRest->cur2());
  }
  dc << '\n';
}

void
Diag535Collector::displayTourCode(const PaxTypeFare* ptf, const NegFareRest* negFareRest)
{
  if (!negFareRest)
    return;

  Diag535Collector& dc = *this;
  std::string tourCode, tourCode27;

  if (!negFareRest->tourBoxCode1().empty())
  {
    tourCode = negFareRest->tourBoxCode1();
  }
  else if ((negFareRest->noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS) &&
           (!negFareRest->tourBoxCode2().empty()))
  {
    tourCode = negFareRest->tourBoxCode2();
  }
  RuleUtil::getCat27TourCode(ptf, tourCode27);

  if (!tourCode.empty() && !tourCode27.empty())
  {
    dc << "  TOUR CODE  : "
       << "*C35* " << negFareRest->tourBoxCodeType1() << "/" << tourCode << " / *C27* "
       << tourCode27 << "\n";
  }
  else if (!tourCode.empty())
  {
    dc << "  TOUR CODE  : "
       << "*C35* " << negFareRest->tourBoxCodeType1() << "/" << tourCode << "\n";
  }
  else if (!tourCode27.empty())
  {
    dc << "  TOUR CODE  : "
       << "*C27* " << tourCode27 << "\n";
  }
}

bool
Diag535Collector::shouldDisplayTFDSPCInd(const NegPaxTypeFareRuleData*& negPaxTypeFare,
                                         Indicator tktFareDataInd1) const
{
  return tktFareDataInd1 == RuleConst::BLANK && negPaxTypeFare &&
         negPaxTypeFare->negFareRestExt() &&
         (negPaxTypeFare->negFareRestExt()->fareBasisAmtInd() != RuleConst::BLANK ||
          negPaxTypeFare->negFareRestExt()->tktFareDataSegExistInd() == 'Y');
}

bool
Diag535Collector::tktFareDataSegExists(const NegPaxTypeFareRuleData*& negPaxTypeFare) const
{
  return negPaxTypeFare && negPaxTypeFare->negFareRestExt() &&
         negPaxTypeFare->negFareRestExt()->tktFareDataSegExistInd() == 'Y';
}

void
Diag535Collector::displayTFDByte101(const NegFareRest* negFareRest)
{
  if (!negFareRest)
    return;

  Diag535Collector& dc = *this;
  dc << "  FARE IND BYTE 101 : " << negFareRest->tktFareDataInd1() << "\n";
}

void
Diag535Collector::displayFareBasisAmountInd(const NegFareRestExt* negFareRestExt)
{
  Diag535Collector& dc = *this;
  dc << "  FARE BASIS/AMOUNT IND : " << negFareRestExt->fareBasisAmtInd() << "\n";
}

void
Diag535Collector::displayTfdpsc(const NegPaxTypeFareRuleData* negPaxTypeFare)
{
  if (negPaxTypeFare->negFareRestExtSeq().empty())
    return;
  this->displayTFDPSC(*this,
                      reinterpret_cast<const std::vector<const NegFareRestExtSeq*>&>(
                          negPaxTypeFare->negFareRestExtSeq()),
                      false);
}

//----------------------------------------------------------------------------
// display Matched Sequences for TFDPSC/UNFBC
//----------------------------------------------------------------------------
void
Diag535Collector::displayMatchedSequencesResult(const FarePath* fp, const PaxTypeFare& ptf)
{
  if (fp)
  {
    for (const PricingUnit* pu : fp->pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        if (fu->paxTypeFare() == &ptf)
        {
          displayMatchedSequencesResult(*fu);
          return;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
// display Matched Sequences for TFDPSC/UNFBC
//----------------------------------------------------------------------------
void
Diag535Collector::displayMatchedSequencesResult(const FareUsage& fu)
{
  Diag535Collector& dc = *this;
  if (fu.netRemitPscResults().empty())
  {
    dc << "  **NO MATCHED SEQUENCES FOR TFDPSC/UNFBC**"
       << "\n";
    return;
  }

  if (fu.netRemitPscResults().front()._tfdpscSeqNumber &&
      fu.netRemitPscResults().front()._tfdpscSeqNumber->uniqueFareBasis().empty())
    return;

  dc << "  MATCHED SEQUENCES FOR UNFBC: "
     << "\n";
  for (const FareUsage::TktNetRemitPscResult& pscResult : fu.netRemitPscResults())
    dc << "  SEQNO " << std::setw(7) << pscResult._tfdpscSeqNumber->seqNo() << " \n";
}

//----------------------------------------------------------------------------
// display PrintOption for Abacus/NetRemit
//----------------------------------------------------------------------------
void
Diag535Collector::displayPrintOption(const NegPaxTypeFareRuleData* negPaxTypeFare, bool addSpace)
{
  Diag535Collector& dc = *this;
  dc.setf(ios::left, ios::adjustfield);
  if (addSpace)
    dc << " ";
  dc << " PRINT OPTION      : ";
  if (negPaxTypeFare->printOption())
    dc << negPaxTypeFare->printOption()->printOption() << "\n";
  else
    dc << "3  DEFAULT"
       << "\n";
}

void
Diag535Collector::displayFareProperties(const FareProperties* fareProperties, bool addSpace)
{
  Diag535Collector& dc = *this;
  dc.setf(ios::left, ios::adjustfield);
  if (addSpace)
    dc << " ";
  dc << " VENDOR : " << fareProperties->vendor()
     << "      FARE SOURCE : " << fareProperties->fareSource() << "\n";
  if (!fareProperties->valueCodeAlgorithmName().empty())
  {
    if (addSpace)
      dc << " ";
    dc << " EXCLUDE Q SURCHARGE IND : " << fareProperties->excludeQSurchargeInd() << "\n";
  }
}

//----------------------------------------------------------------------------
// display Dynamic Value Code
//----------------------------------------------------------------------------
void
Diag535Collector::displayValueCodeAlgorithm(const ValueCodeAlgorithm& valCodeAlg, bool addSpace)
{
  Diag535Collector& dc = *this;
  dc.setf(ios::left, ios::adjustfield);

  if (addSpace)
    dc << " ";
  dc << " VALUE CODE ALGORITHM NAME : " << valCodeAlg.algorithmName() << "\n";
  if (addSpace)
    dc << " ";
  dc << " PREFIX  SUFFIX  1  2  3  4  5  6  7  8  9  0  DECIMAL \n";
  if (addSpace)
    dc << " ";
  dc << " " << setw(8) << valCodeAlg.prefix() << setw(8) << valCodeAlg.suffix() << setw(3)
     << valCodeAlg.digitToChar()[1] << setw(3) << valCodeAlg.digitToChar()[2] << setw(3)
     << valCodeAlg.digitToChar()[3] << setw(3) << valCodeAlg.digitToChar()[4] << setw(3)
     << valCodeAlg.digitToChar()[5] << setw(3) << valCodeAlg.digitToChar()[6] << setw(3)
     << valCodeAlg.digitToChar()[7] << setw(3) << valCodeAlg.digitToChar()[8] << setw(3)
     << valCodeAlg.digitToChar()[9] << setw(3) << valCodeAlg.digitToChar()[0] << setw(3)
     << valCodeAlg.decimalChar() << '\n';
}

//----------------------------------------------------------------------------
// display Static Value Code
//----------------------------------------------------------------------------
void
Diag535Collector::displayStaticValueCode(const NegFareRestExt* negFareRestExt,
                                         const std::string& cat18ValueCode,
                                         bool addSpace)
{
  Diag535Collector& dc = *this;
  std::string cat35ValueCode;
  Indicator cat35CombInd = RuleConst::DISPLAY_OPTION_BLANK;
  if (negFareRestExt)
  {
    cat35ValueCode = negFareRestExt->staticValueCode();
    cat35CombInd = negFareRestExt->staticValueCodeCombInd();
  }

  if (addSpace)
    dc << " ";
  dc << " STATIC VALUE CODE :";

  if (!cat35ValueCode.empty())
  {
    dc << " *C35* " << cat35ValueCode;
    if (!cat18ValueCode.empty())
      dc << " /";
  }

  if (!cat18ValueCode.empty())
  {
    dc << " *C18* " << cat18ValueCode;
  }

  dc << '\n';

  if (addSpace)
    dc << " ";
  dc << " STATIC VALUE CODE DISPLAY OPTION : ";

  switch (cat35CombInd)
  {
  case RuleConst::DISPLAY_OPTION_1ST:
    dc << "1ST";
    break;
  case RuleConst::DISPLAY_OPTION_2ND:
    dc << "2ND";
    break;
  case RuleConst::DISPLAY_OPTION_ALL:
    dc << "ALL";
    break;
  }
  dc << "\n";
}

std::string
Diag535Collector::getCat18ValueCode(const FarePath* fp, const PaxTypeFare& ptf)
{
  if (fp)
  {
    for (const PricingUnit* pu : fp->pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        if (fu->paxTypeFare() == &ptf)
          return ValueCodeUtil::getCat18ValueCode(*fu);
      }
    }
  }

  return "";
}

void
Diag535Collector::displayTourCodeCombInd(const NegFareRestExt* negFareRestExt, bool addSpace)
{
  Diag535Collector& dc = *this;
  Indicator tourCodeCombInd =
      negFareRestExt ? negFareRestExt->tourCodeCombInd() : RuleConst::DISPLAY_OPTION_BLANK;
  if (addSpace)
    dc << " ";
  dc << " TOUR CODE DISPLAY OPTION : ";

  switch (tourCodeCombInd)
  {
  case RuleConst::DISPLAY_OPTION_1ST:
    dc << "1ST";
    break;
  case RuleConst::DISPLAY_OPTION_2ND:
    dc << "2ND";
    break;
  case RuleConst::DISPLAY_OPTION_ALL:
    dc << "ALL";
    break;
  }
  dc << "\n";
}

} // namespace
