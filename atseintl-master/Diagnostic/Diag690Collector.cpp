//----------------------------------------------------------------------------
//  File:         Diag690Collector.C
//  Description:  Diagnostic Collector base class: Defines all the virtual methods
//                derived class may orverride these methods.
//
//  Authors:      Mohammad Hossan
//  Created:      April 2004
//
//  Updates:
//          date - initials - description.
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
#include "Diagnostic/Diag690Collector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/BookingCodeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tse
{
bool
tse::Diag690Collector::_filterPassed(true);
const std::string Diag690Collector::DISPLAY_SOLUTION = "DS";

FIXEDFALLBACK_DECL(validateAllCat16Records);

void
Diag690Collector::printHeader()
{
  if (_active)
  {
    *this << "******************** FARE PATH - ANALYSIS *********************\n";

    RexBaseTrx* rexTrx = dynamic_cast<RexBaseTrx*>(trx());
    if (rexTrx && rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
      Diag690Collector::_filterPassed =
          rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "LOWEST";
  }
}

void
Diag690Collector::printSummaryForLowestFarePaths()
{
  RexPricingTrx* rexTrx = dynamic_cast<RexPricingTrx*>(trx());
  if (!rexTrx)
    return;

  bool printAdditionalLine = !Diag690Collector::_filterPassed;
  Diag690Collector::_filterPassed = true;

  if (printAdditionalLine)
    printLine();

  if (rexTrx->lowestRebookedFarePath())
  {

    *this << "*************** THE LOWEST REBOOKED FARE PATH *****************\n" << std::endl;
    printLine();
    *this << *rexTrx->lowestRebookedFarePath();
    printLine();
  }

  if (rexTrx->lowestBookedFarePath())
  {
    *this << "*************** THE LOWEST BOOKED FARE PATH *******************\n" << std::endl;
    printLine();
    *this << *rexTrx->lowestBookedFarePath();
    printLine();
  }
}

void
Diag690Collector::printRefundFarePath()
{
  RefundPricingTrx* rexTrx = dynamic_cast<RefundPricingTrx*>(trx());
  if (!rexTrx)
    return;

  if (!Diag690Collector::_filterPassed)
    printLine();

  Diag690Collector::_filterPassed = true;

  *this << "******************* THE LOWEST FARE PATH **********************\n" << std::endl;
  printLine();

  if (rexTrx->fullRefund())
    printCommonRefundPermutationInfo(*rexTrx->fullRefundWinningPermutation());
  else
    *this << *rexTrx->itin().front()->farePath().front();

  printLine();
}

Diag690Collector & Diag690Collector::operator<< (const  Fare& fare)
{
  if (!_active)
  {
    return *this;
  }

  Diag690Collector& dc = *this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  // std::string fareBasis = paxFare.createFareBasis(*_trx, false);
  // if (fareBasis.size() > 8)
  //  fareBasis = fareBasis.substr(0,8) + "$";
  //
  // dc << std::setw(9) << fareBasis

  dc << std::setw(9) << fare.fareClass() << std::setw(5) << fare.vendor() << std::setw(4)
     << fare.tcrRuleTariff() << "-" << std::setw(8) << fare.tcrRuleTariffCode() << std::setw(3)
     << fare.carrier() << std::setw(5) << fare.ruleNumber() << std::setw(8)
     << Money(fare.fareAmount(), fare.currency()) << " " << std::setw(3) << gd << std::setw(2)
     << fare.owrt() << std::setw(2)
     << (fare.directionality() == FROM ? "O" : fare.directionality() == TO ? "I" : " ")
     << std::endl;

  return *this;
}

Diag690Collector & Diag690Collector::operator<< (const  std::vector<FarePath*>& fpVect)
{
  if (!_active || !Diag690Collector::_filterPassed)
  {
    return *this;
  }

  if (fpVect.empty())
  {
    return *this;
  }

  std::vector<FarePath*>::const_iterator fpathIter = fpVect.begin();
  Itin* itin = (*fpathIter)->itin();
  std::vector<TravelSeg*>::iterator start = itin->travelSeg().begin();
  std::vector<TravelSeg*>::reverse_iterator end = itin->travelSeg().rbegin();
  ((DiagCollector&)*this) << (*start)->origin()->loc() << "-- " << (*end)->destination()->loc()
                          << std::endl;

  setf(std::ios::fixed, std::ios::floatfield);
  precision(2);
  ((DiagCollector&)*this) << (*fpathIter)->getTotalNUCAmount() << std::endl;
  for (; fpathIter != fpVect.end(); ++fpathIter)
  {
    *this << *(*fpathIter);
    ((DiagCollector&)*this) << std::endl;
  }

  return *this;
}

namespace
{
inline std::string
toString(Indicator ind)
{
  return ind == ' ' ? "BLANK" : std::string() += ind;
}
}

std::string
Diag690Collector::residualPenaltyIndicator(Indicator orginal, Indicator actual) const
{
  std::ostringstream os;
  os << "RESIDUAL/PENALTY INDICATOR: " << toString(orginal);
  if (actual != orginal)
    os << '/' << toString(actual);
  os << '\n';

  return os.str();
}

Diag690Collector & Diag690Collector::operator<< (const FarePath& fPath)
{
  if (!_active || !Diag690Collector::_filterPassed)
  {
    return *this;
  }

  setf(std::ios::fixed, std::ios::floatfield);
  precision(2);
  //((DiagCollector &) *this).precision(2);
  // Satya added new line for Requested PAXTYPE
  DiagCollector& dc(*this);
  dc << " AMOUNT: " << fPath.getTotalNUCAmount() << " " << fPath.getCalculationCurrency()
     << " REQUESTED PAXTYPE: " << fPath.paxType()->paxType();
  if (fPath.isExchange())
    dc << "    EXCHANGE";
  else if (fPath.isReissue())
    dc << "    REISSUE";

  PricingTrx* pricingTrx(dynamic_cast<PricingTrx*>(_trx));
  if (pricingTrx->getRequest()->getBrandedFareSize() > 1)
    dc << " BRAND ID: " << AirlineShoppingUtils::getBrandIndexes(*pricingTrx, fPath);

  dc << "\n";

  const YQYRCalculator* yqyrCalc = fPath.yqyrCalculator();
  if (yqyrCalc)
  {
    if(yqyrCalc->preCalcFailed())
      dc << " YQYR PRECALCULATION TOO EXPENSIVE, PROCESSING WITHOUT TP" << std::endl;
    else
      dc << " AMOUNT WITH YQ/YR: " << fPath.getTotalNUCAmount() + fPath.yqyrNUCAmount()
         << std::endl;
  }
  RexPricingTrx* rexTrx(dynamic_cast<RexPricingTrx*>(_trx));
  if (rexTrx)
    printRexFarePathHeader(fPath);

  else if (dynamic_cast<RefundPricingTrx*>(_trx))
  {
    printRefundPlusUp();
    printRefundFarePathHeader(fPath);
  }

  else if (pricingTrx->getOptions() && pricingTrx->getOptions()->isRtw() &&
           pricingTrx->getRequest()->isLowFareRequested())
    dc << (BookingCodeUtil::isRebookedSolution(fPath) ? " REBOOKED" : " AS BOOKED") << "\n";

  if(! fPath.validatingCarriers().empty())
  {
    (*this) << "VALIDATING CXR: ";
    for (CarrierCode valCxr : fPath.validatingCarriers())
      (*this) << valCxr.c_str() << "  ";
    (*this) << std::endl;
  }

  for (PricingUnit* pu : fPath.pricingUnit())
  {
    printPricingUnit(
        *pu, fPath.getCalculationCurrency(), fPath.itin()->useInternationalRounding());
  }

  // Minimum Fare Plus up in FarePath

  // For COM and DMC across PU plus ups.
  const std::vector<FarePath::PlusUpInfo*>& fpPlusUps = fPath.plusUpInfoList();
  std::vector<FarePath::PlusUpInfo*>::const_iterator fpPlusUpIter = fpPlusUps.begin();
  for (; fpPlusUpIter != fpPlusUps.end(); fpPlusUpIter++)
  {
    if ((*fpPlusUpIter)->module() == COM)
      dc << "COM        ";
    else if ((*fpPlusUpIter)->module() == DMC)
      dc << "DMC        ";
    dc << std::setw(7) << (*fpPlusUpIter)->minFarePlusUp()->plusUpAmount << std::endl;
  }

  // For OSC
  const std::vector<FarePath::OscPlusUp*>& oscPlusUps = fPath.oscPlusUp();
  std::vector<FarePath::OscPlusUp*>::const_iterator oscIter = oscPlusUps.begin();
  for (; oscIter != oscPlusUps.end(); oscIter++)
  {
    dc << "OSC        " << std::setw(7) << (*oscIter)->plusUpAmount << std::endl;
  }

  // For RSC
  const std::vector<FarePath::RscPlusUp*>& rscPlusUps = fPath.rscPlusUp();
  std::vector<FarePath::RscPlusUp*>::const_iterator rscIter = rscPlusUps.begin();
  for (; rscIter != rscPlusUps.end(); rscIter++)
  {
    dc << "RSC        " << std::setw(7) << (*rscIter)->plusUpAmount << std::endl;
  }

  dc << std::endl;

  if (rexTrx &&
      (rexTrx->lowestRebookedFarePath() == &fPath || rexTrx->lowestBookedFarePath() == &fPath))
  {
    std::string msg = fPath.getNonrefundableMessage2();
    if (!msg.empty())
      dc << msg << '\n';
  }
  if (yqyrCalc && !yqyrCalc->preCalcFailed())
  {
    dc << "YQ/YR AMOUNT: " << fPath.yqyrNUCAmount();
    dc << "  LB: " << yqyrCalc->lowerBound();
  }
  dc << std::endl;

  return *this;
}

void
Diag690Collector::printRexFarePathHeader(const FarePath& fPath)
{
  RexPricingTrx* rexTrx(static_cast<RexPricingTrx*>(_trx));
  DiagCollector& dc(*this);

  if (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    dc << (fPath.rebookClassesExists() ? " AS REBOOKED" : " AS BOOKED");
    RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(trx());
    if (rexExcTrx)
    {
      size_t itinIndex = rexExcTrx->getItinPos(fPath.itin());
      *this << " FOR ITIN ";
      ((std::ostringstream&)*this) << itinIndex;
    }
    dc << std::endl;
  }

  if (fPath.lowestFee31Perm())
  {
    MoneyAmount changeFee = fPath.rexChangeFee();
    const ProcessTagPermutation* leastExpensivePermutation = fPath.lowestFee31Perm();

    dc << " PERMUTATION ID: " << leastExpensivePermutation->number() << std::endl;
    dc << " CHANGE FEE: ";

    const std::vector<ProcessTagInfo*>& processTagInfo(leastExpensivePermutation->processTags());
    std::vector<ProcessTagInfo*>::const_iterator pti(processTagInfo.begin());
    std::vector<ProcessTagInfo*>::const_iterator pte(processTagInfo.end());

    RexPricingTrx::WaivedChangeRecord3Set& waived(rexTrx->waivedChangeFeeRecord3());
    const RexPricingTrx::WaivedChangeRecord3Set::const_iterator NOT_WAIVED(waived.end());

    bool waiverCode(false);

    if (changeFee < EPSILON)
    {
      for (; pti != pte; pti++)
      {
        ProcessTagInfo* info(*pti);

        if (waived.find(info->record3()->orig()) != NOT_WAIVED)
        {
          waiverCode = true;
          break;
        }
      }
      if (waiverCode)
        dc << "WAIVED";
      else
        dc << "NOT APPLICABLE";
      dc << std::endl;
    }
    else
    {
      dc << rexTrx->convertCurrency(Money(changeFee, NUC),
                                    fPath.getCalculationCurrency(),
                                    rexTrx->itin().front()->useInternationalRounding())
         << std::endl;
    }

    dc << "ENDORSEMENT ";
    if (leastExpensivePermutation->isOverriden())
      dc << "(OVERIDN) ";
    dc << leastExpensivePermutation->getEndorsementByte() << std::endl;

    if (rexTrx->lowestBookedFarePath() && rexTrx->lowestRebookedFarePath())
    {
      dc << "FORM OF REFUND: REBOOKED: "
         << formOfRefundInd(rexTrx->lowestRebookedFarePath()->lowestFee31Perm())
         << " BOOKED: " << formOfRefundInd(rexTrx->lowestBookedFarePath()->lowestFee31Perm())
         << std::endl;
    }
    else if (rexTrx->lowestBookedFarePath())
    {
      dc << "FORM OF REFUND: BOOKED: "
         << formOfRefundInd(rexTrx->lowestBookedFarePath()->lowestFee31Perm()) << std::endl;
    }
    else if (rexTrx->lowestRebookedFarePath())
    {
      dc << "FORM OF REFUND: REBOOKED: "
         << formOfRefundInd(rexTrx->lowestRebookedFarePath()->lowestFee31Perm()) << std::endl;
    }

    if (rexTrx->lowestRebookedFarePath() == &fPath || rexTrx->lowestBookedFarePath() == &fPath)
      dc << residualPenaltyIndicator(leastExpensivePermutation->getResidualPenaltyByte(),
                                     fPath.residualPenaltyIndicator(*rexTrx));

    dc << " BASE FARE INCLUDING CHANGE FEE: " << fPath.getTotalNUCAmount() + changeFee << " "
       << fPath.getCalculationCurrency() << std::endl;
  }
}

void
Diag690Collector::printRefundPlusUp()
{
  RefundPricingTrx& refundTrx = static_cast<RefundPricingTrx&>(*_trx);
  if (refundTrx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    *this << " PLUS-UPS RECREATED: ";
    refundTrx.arePenaltiesAndFCsEqualToSumFromFareCalc() ? *this << 'N' : *this << 'Y';
    *this << "\n";
  }
}

void
Diag690Collector::printRefundFarePathHeader(const FarePath& fPath)
{
  if (fPath.lowestFee33Perm())
  {
    DiagCollector& dc(*this);

    printCommonRefundPermutationInfo(*fPath.lowestFee33Perm());

    dc << " BASE FARE INCLUDING PENALTY FEE: " << fPath.getTotalNUCAmount() + fPath.rexChangeFee()
       << " " << fPath.getCalculationCurrency() << "\n";
  }
}

void
Diag690Collector::printCommonRefundPermutationInfo(const RefundPermutation& winner)
{
  RefundPricingTrx& trx = static_cast<RefundPricingTrx&>(*_trx);
  DiagCollector& dc(*this);

  dc << " PERMUTATION ID: " << winner.number();
  dc << "\n PENALTY FEE: " << winner.overallPenalty(trx) << " "
     << trx.exchangeItinCalculationCurrency();
  dc << "\n FORM OF REFUND: ";
  decodeFormOfRefund(winner.formOfRefundInd());
  dc << "\n";
}

void
Diag690Collector::decodeFormOfRefund(const Indicator& formOfRefund)
{
  switch (formOfRefund)
  {
  case RefundPermutation::ORIGINAL_FOP:
    *this << "ORIGINAL FOP";
    break;
  case RefundPermutation::VOUCHER:
    *this << "VOUCHER";
    break;
  case RefundPermutation::MCO:
    *this << "MCO";
    break;
  case RefundPermutation::ANY_FORM_OF_PAYMENT:
    *this << "ANY FORM OF PAYMENT";
    break;
  case RefundPermutation::SCRIPT:
    *this << "SCRIPT";
    break;
  }
}

Diag690Collector& Diag690Collector::operator<<(const FareUsage& fu)
{
  DiagCollector& dc = *this;

  dc << " " << fu.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
     << fu.paxTypeFare()->fareMarket()->offMultiCity() << std::endl;

  printAmounts(fu);

  // Minimum Fare Plus up in Fare Usage
  const MinFarePlusUp& fuPlusUps = fu.minFarePlusUp();
  MoneyAmount fuPlusUp = fuPlusUps.getSum(HIP);
  if (fuPlusUp > 0)
    dc << "  HIP        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(BHC);
  if (fuPlusUp > 0)
    dc << "  BHC        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(COM);
  if (fuPlusUp > 0)
    dc << "  COM        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(DMC);
  if (fuPlusUp > 0)
    dc << "  DMC        " << std::setw(7) << fuPlusUp << std::endl;

  const PaxTypeFare* fare = fu.paxTypeFare();
  dc << "  " << *fare;
  if (fu.adjustedPaxTypeFare() != nullptr)
  {
    dc << "\n\n";
    dc << "  CURRENCY ADJUSTMENT APPLIED FROM: " << std::endl;
    dc << "  " << *(fu.adjustedPaxTypeFare());
  }
  dc << "\n\n";

  return *this;
}

Diag690Collector & Diag690Collector::operator<< (const  PricingUnit& pu)
{
  if (!_active)
  {
    return *this;
  }

  DiagCollector& dc = *this;
  printPuType(pu.puType());

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  for (; fuIter != pu.fareUsage().end(); ++fuIter)
  {
    *this << **fuIter;
  }

  // Minimum Fare Plus up in PU
  const MinFarePlusUp& puPlusUps = pu.minFarePlusUp();
  MoneyAmount puPlusUp = puPlusUps.getSum(CTM);
  if (puPlusUp > 0)
    dc << " CTM        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(CPM);
  if (puPlusUp > 0)
    dc << " CPM        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(COP);
  if (puPlusUp > 0)
    dc << " COP        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(OJM);
  if (puPlusUp > 0)
    dc << " HRTOJ      " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(HRT);
  if (puPlusUp > 0)
    dc << " HRTC      " << std::setw(7) << puPlusUp << std::endl;

  return *this;
}

void
Diag690Collector::printFareUsage(const FareUsage& fu,
                                 CurrencyCode calculationCurrency,
                                 bool useInternationalRounding)
{
  DiagCollector& dc = *this;

  dc << " " << fu.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
     << fu.paxTypeFare()->fareMarket()->offMultiCity() << std::endl;

  const PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  if (pricingTrx)
  {
    if(fallback::fixed::validateAllCat16Records())
    {
      Money nonRefAmt = fu.isNonRefundable()
               ? fu.getNonRefundableAmt(calculationCurrency, *pricingTrx, useInternationalRounding)
               : Money(calculationCurrency);
      dc << "  NONREFUNDABLE: " << nonRefAmt << std::endl;
    }
    else
    {
      if (fu.isNonRefundable())
      {
        dc << "  NONREFUNDABLE: YES" << std::endl;
      }
      else
      {
        dc << "  NONREFUNDABLE AMOUNT: " << fu.getNonRefundableAmount() << std::endl;
      }
    }


  }

  printAmounts(fu);

  // Minimum Fare Plus up in Fare Usage
  const MinFarePlusUp& fuPlusUps = fu.minFarePlusUp();
  MoneyAmount fuPlusUp = fuPlusUps.getSum(HIP);
  if (fuPlusUp > 0)
    dc << "  HIP        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(BHC);
  if (fuPlusUp > 0)
    dc << "  BHC        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(COM);
  if (fuPlusUp > 0)
    dc << "  COM        " << std::setw(7) << fuPlusUp << std::endl;
  fuPlusUp = fuPlusUps.getSum(DMC);
  if (fuPlusUp > 0)
    dc << "  DMC        " << std::setw(7) << fuPlusUp << std::endl;

  const PaxTypeFare* fare = fu.paxTypeFare();
  dc << "  " << *fare;
  if (fu.adjustedPaxTypeFare() != nullptr)
  {
    dc << "\n\n";
    dc << "  CURRENCY ADJUSTMENT APPLIED FROM: " << std::endl;
    dc << "  " << *(fu.adjustedPaxTypeFare());
  }
  dc << "\n\n";
}

void
Diag690Collector::printPricingUnit(const PricingUnit& pu,
                                   CurrencyCode calculationCurrency,
                                   bool useInternationalRounding)
{
  if (!_active)
  {
    return;
  }

  DiagCollector& dc = *this;

  printPuType(pu.puType());

  for (FareUsage* fu : pu.fareUsage())
  {
    printFareUsage(*fu, calculationCurrency, useInternationalRounding);
  }

  // Minimum Fare Plus up in PU
  const MinFarePlusUp& puPlusUps = pu.minFarePlusUp();
  MoneyAmount puPlusUp = puPlusUps.getSum(CTM);
  if (puPlusUp > 0)
    dc << " CTM        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(CPM);
  if (puPlusUp > 0)
    dc << " CPM        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(COP);
  if (puPlusUp > 0)
    dc << " COP        " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(OJM);
  if (puPlusUp > 0)
    dc << " HRTOJ      " << std::setw(7) << puPlusUp << std::endl;
  puPlusUp = puPlusUps.getSum(HRT);
  if (puPlusUp > 0)
    dc << " HRTC      " << std::setw(7) << puPlusUp << std::endl;
}

void
Diag690Collector::printAmounts(const FareUsage& fu)
{
  DiagCollector& dc = *this;

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << "  SURCHARGE  " << std::setw(7) << fu.surchargeAmt() << "  TRANSFER   " << std::setw(7)
     << fu.transferAmt() << "  STOPOVER   " << std::setw(7) << fu.stopOverAmt() << std::endl;

  dc << "  DIFF       " << std::setw(7) << fu.differentialAmt() << "  EMS        " << std::setw(7)
     << fu.paxTypeFare()->mileageSurchargeAmt() << std::endl;
}

Indicator
Diag690Collector::formOfRefundInd(const ProcessTagPermutation* permutation)
{
  if (!permutation)
    return 'X';

  return permutation->formOfRefundInd();
}

Diag690Collector& Diag690Collector::operator<< (const char *msg)
{
  if (!_active || !Diag690Collector::_filterPassed || !msg)
    return *this;

  ((std::ostringstream&)*this) << msg;
  return *this;
}

Diag690Collector & Diag690Collector::operator<< (const std::string &msg)
{
  if (!_active || !Diag690Collector::_filterPassed)
    return *this;

  ((std::ostringstream&)*this) << msg;
  return *this;
}

Diag690Collector& Diag690Collector::operator<< (std::ostream& ( *pf )(std::ostream&))
{
  if (!_active || !Diag690Collector::_filterPassed)
    return *this;

  ((std::ostringstream&)*this) << pf;
  return *this;
}

void
Diag690Collector::printLine()
{
  if (_active && Diag690Collector::_filterPassed)
    DiagCollector::printLine();
}
}
