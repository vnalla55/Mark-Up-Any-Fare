//----------------------------------------------------------------------------
//  File:        Diag866Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 866
//               Commissions processing for the ticketing entry.
//
//  Updates:
//          12/07/2004  VK - create.
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
#include "Diagnostic/Diag866Collector.h"

#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CommissionCap.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/NegFareRest.h"
#include "Rules/Commissions.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>
#include <iostream>

namespace tse
{
void
Diag866Collector::diag866Request(PricingTrx& trx, FarePath& farePath, const Commissions& comm)
{
  if (_active)
  {
    char ind;
    std::string type;

    MoneyAmount inputAmt = trx.getRequest()->ticketingAgent()->commissionAmount();
    Percent inputPrc = trx.getRequest()->ticketingAgent()->commissionPercent();

    DiagCollector& dc(*this);

    CurrencyCode pC = comm.paymentCurrency();
    CurrencyCode cC = farePath.calculationCurrency();

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "************************************************************\n";
    dc << "*     ATSE COMMISSION REQUEST DATA                         *\n";
    dc << "************************************************************\n";
    dc << '\n';
    dc << " PASSENGER TYPE                  : ";
    dc << std::setw(7) << farePath.paxType()->paxType() << '\n';

    dc << " CALCULATED CURRENCY             : " << cC << '\n';

    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc << " TOTAL SELLING AMOUNT            : ";
    dc << std::setw(8) << Money(comm.totalSellAmt(), cC) << '\n';

    if (comm.ticketingEntry())
      ind = 'Y';
    else
      ind = 'N';
    dc << " TICKETING ENTRY                 : " << ind << '\n';

    dc << " PAYMENT CURRENCY                : " << pC << '\n';

    if (trx.getRequest()->ticketingAgent()->agentCommissionType()[0] ==
        Commissions::PERCENT_COMM_TYPE)
    {
      type = "PERCENT";
      dc << " AGENT COMMISSION TYPE           : " << type << '\n';
      dc << " AGENT COMMISSION VALUE          : ";

      dc.precision(2);
      dc << std::setw(8) << inputPrc << '\n';
    }
    // temporary.....
    else if (trx.getRequest()->ticketingAgent()->agentCommissionType().empty())
    {
      type = "NONE";
      dc << " AGENT COMMISSION TYPE           : " << type << '\n';
      dc << " AGENT COMMISSION VALUE          : " << '\n';
    }
    else // if (trx.getRequest()->ticketingAgent()->agentCommissionType()[0] ==
         // Commissions::AMOUNT_COMM_TYPE ||
    //          )
    {
      type = "AMOUNT";
      dc << " AGENT COMMISSION TYPE           : " << type << '\n';
      dc << " AGENT COMMISSION VALUE          : ";
      dc << std::setw(8) << Money(inputAmt, comm.paymentCurrency()) << '\n';
    }

    dc.setf(std::ios::left, std::ios::adjustfield);

    CarrierCode vC = farePath.itin()->validatingCarrier();
    dc << " VALIDATING CARRIER              : " << vC << '\n';

    type.clear();
    if (comm.allOnValidCarrier())
      type = " ALL SEGMENTS";
    else if (comm.anyOnValidCarrier())
      type = " AT LEAST ONE SEGMENT";
    else if (comm.noValidCarrier())
      type = " NONE";
    else
      type = " UNKNOWN";
    dc << " ITINERARY ON VALIDATING CARRIER :" << type << '\n';

    type.clear();
    SmallBitSet<uint16_t, Itin::TripCharacteristics>& tripChar =
        farePath.itin()->tripCharacteristics();

    if (tripChar.isSet(Itin::USOnly))
      type = "WITHIN USA";
    else if (tripChar.isSet(Itin::CanadaOnly))
      type = "WITHIN CANADA";
    else if (farePath.itin()->geoTravelType() == GeoTravelType::Domestic)
      type = "DOMESTIC";
    else if (farePath.itin()->geoTravelType() == GeoTravelType::International)
      type = "INTERNATIONAL";
    else if (farePath.itin()->geoTravelType() == GeoTravelType::Transborder)
      type = "TRANSBORDER US/CANADA";
    else
      type = " ";
    dc << " ITINERARY TRIP TYPE             : " << type << '\n';

    type.clear();
    if (tripChar.isSet(Itin::OneWay))
      type = "ONE WAY";
    else if (tripChar.isSet(Itin::RoundTrip))
      type = "ROUND TRIP";
    else
      type = " ";
    dc << " TRIP TYPE                       : " << type << '\n';
  }

  return;
}

void
Diag866Collector::diag866DisplayComCap(PricingTrx& trx, const CommissionCap& cCap)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << '\n';
    dc << " MATCHED COMMISSION CAP DATA     : " << '\n';

    MoneyAmount amtMax = cCap.maxAmt();
    MoneyAmount amtMin = cCap.minAmt();
    MoneyAmount amt = cCap.amt();
    CurrencyCode cc = cCap.cur();
    //
    if (cCap.amtType() == Commissions::PERCENT_COMM_TYPE)
    {
      dc << "        COMMISSION VALUE         : " << amt << " PERCENT " << '\n';
    }
    else if (cCap.amtType() == Commissions::FLAT_AMOUNT_TYPE)
    {
      dc << "        COMMISSION VALUE         : ";
      dc << std::setw(8) << Money(amt, cc) << '\n';
    }
    else
    {
      dc << "        COMMISSION VALUE         : NONE" << '\n';
    }
    //
    if (cCap.maxAmtType() == Commissions::PERCENT_COMM_TYPE)
    {
      dc << "        MAX COMMISSION VALUE     : " << amtMax << " PERCENT " << '\n';
    }
    else if (cCap.maxAmtType() == Commissions::FLAT_AMOUNT_TYPE)
    {
      dc << "        MAX COMMISSION VALUE     : ";
      dc << std::setw(8) << Money(amtMax, cc) << '\n';
    }
    else
    {
      dc << "        MAX COMMISSION VALUE     : NONE" << '\n';
    }
    //
    if (cCap.minAmtType() == Commissions::PERCENT_COMM_TYPE)
    {
      dc << "        MIN COMMISSION VALUE     : " << amtMin << " PERCENT " << '\n';
    }
    else if (cCap.minAmtType() == Commissions::FLAT_AMOUNT_TYPE)
    {
      dc << "        MIN COMMISSION VALUE     : ";
      dc << std::setw(8) << Money(amtMin, cc) << '\n';
    }
    else
    {
      dc << "        MIN COMMISSION VALUE     : NONE" << '\n';
    }
    //
    char ind;

    if (cCap.canada() == YES)
      ind = 'Y';
    else
      ind = 'N';
    dc << "        TRAVEL IN CANADA         : " << ind << '\n';

    if (cCap.domestic() == YES)
      ind = 'Y';
    else
      ind = 'N';
    dc << "        DOMESTIC TRAVEL          : " << ind << '\n';

    if (cCap.international() == YES)
      ind = 'Y';
    else
      ind = 'N';
    dc << "        INTERNATIONAL TRAVEL     : " << ind << '\n';

    if (cCap.transBorder() == YES)
      ind = 'Y';
    else
      ind = 'N';
    dc << "        US/CANADA TRAVEL         : " << ind << '\n';

    std::string type;
    if (cCap.owRt() == Commissions::ONE_WAY)
      type = "ONE WAY";
    else if (cCap.owRt() == Commissions::ROUND_TRIP)
      type = "ROUND TRIP";
    else
      type = "ANY TRIP";
    dc << "        TRAVEL TYPE              : " << type << '\n';

    type.clear();
    if (cCap.valCarrierTvl() == Commissions::ALL_SEG_ON_VAL_CARRIER)
      type = "ALL SEGMENTS ";
    else if (cCap.valCarrierTvl() == Commissions::AT_LEAST_ONE_SEG_ON_VAL_CARRIER)
      type = "ONE SEGMENT ";
    else if (cCap.valCarrierTvl() == Commissions::NO_SEG_ON_VAL_CARRIER)
      type = "NO SEGMENTS";
    else
      type = " NONE";
    dc << "        VALIDATING CARRIER       : " << type << '\n';

    dc << "        TICKETING ISSUE NATION   : " << cCap.tktIssueNation() << '\n';
  }
}

void
Diag866Collector::diag866Commission(PricingTrx& trx,
                                    const FarePath& farePath,
                                    const Commissions& comm)
{
  if (_active)
  {
    MoneyAmount cAmt = comm.commAmount();
    CurrencyCode pC = comm.paymentCurrency();
    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << " " << '\n';
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc << " CALCULATED COMMISSION AMOUNT    :" << std::setw(8) << Money(cAmt, pC) << '\n';

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "************************************************************\n";
  }
}
}
