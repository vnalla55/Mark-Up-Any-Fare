// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/Diversity/ADIsSolutionNeededCheck.h"

#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/SopVecOutputDecorator.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"

namespace tse
{

ADIsSolutionNeededCheck&
ADIsSolutionNeededCheck::examine(const DiversityModelCallback& divCtx,
                                 const DiversityModel::SOPCombination& comb,
                                 MoneyAmount price)
{
  Stat stat = divCtx.getStat(comb, price);

  examine(divCtx, price, *comb.datePair, comb.oSopVec, stat);
  return (*this);
}

ADIsSolutionNeededCheck&
ADIsSolutionNeededCheck::examine(const DiversityModelCallback& divCtx,
                                 const ShoppingTrx::FlightMatrix::value_type& solution,
                                 DatePair datePair,
                                 bool isPresent)
{
  MoneyAmount price = solution.second->getTotalNUCAmount();
  Stat stat = divCtx.getStat(solution, datePair);

  if (!isPresent)
  {
    examine(divCtx, price, datePair, solution.first, stat);
  }
  else
  {
    TSE_ASSERT(stat._numOpt != 0 ||
               !"Get statistic failed for option is supposed to be present in AltDatesStatistic");

    stat._numOpt--;
    stat._sameConnectingCityAndCxrOpt.clear();
    examine(divCtx, price, datePair, solution.first, stat);
  }

  return (*this);
}

DiagCollector& operator<<(DiagCollector& dc, const ADIsSolutionNeededCheck& op)
{
  dc << "option (" << SopVecOutputDecorator(*op._trx, op._sops) << ")"
     << " to DatePair(" << op._datePair.first.dateToIsoExtendedString() << ", "
     << op._datePair.second.dateToIsoExtendedString() << "), "
     << "carrier " << op._stat._govCarrier;

  if (op._stat._solKind.is_initialized() && (*op._stat._solKind & DiversityUtil::Snowman))
  {
    dc << " (snowman)";
  }

  dc << ", fare level " << op._stat._fareLevel << " amount " << op._price
     << " (#options = " << op._stat._numOpt << ")";

  if (op._skipReason)
  {
    dc << ": " << op._skipReason;
  }

  dc << "\n";

  return dc;
}

bool
ADIsSolutionNeededCheck::isSnowmanNeeded(const DiversityModelCallback& divCtx,
                                         shpq::SopIdxVecArg sops,
                                         const Stat& stat)
{
  if (!stat._minTravelSegCount.is_initialized())
    return true;

  size_t snowmanSegCount = DiversityUtil::getTravelSegCount(*divCtx.getTrx(), sops);
  bool result = (snowmanSegCount <= *stat._minTravelSegCount);

  return result;
}

void
ADIsSolutionNeededCheck::examine(const DiversityModelCallback& divCtx,
                                 MoneyAmount price,
                                 DatePair datePair,
                                 shpq::SopIdxVecArg sops,
                                 const Stat& stat)
{
  _skipReason = nullptr;
  _price = price;
  _datePair = datePair;
  _sops = sops;
  _stat = stat;
  _trx = divCtx.getTrx();

  if (UNLIKELY(divCtx.checkDatePairFilt(datePair)))
  {
    _isNeeded = false;
    return;
  }

  if (divCtx.isDatePairFareCutoffReached(stat._firstFareLevelAmount, price))
  {
    _isNeeded = false;
    _skipReason = "date pair cut-off amount has been reached";
    return;
  }

  {
    const uint16_t flNumberRequired = divCtx.getTrx()->diversity().getAltDates()._fareLevelNumber;
    const bool isFareLevelNeeded = (stat._fareLevel <= flNumberRequired);
    if (!isFareLevelNeeded)
    {
      _isNeeded = false;
      _skipReason = "max fare level has been reached";
      return;
    }
  }

  if (!divCtx.isFirstCxrFareLevel(stat))
  {
    _isNeeded = false;
    _skipReason = "max one carrier per fare level limitation";
    return;
  }

  if (!divCtx.isNumOptionNeeded(stat))
  {
    _isNeeded = false;
    _skipReason = stat.isSnowman() ? "max number of options per snowman has been reached"
                                   : "max number of options per carrier has been reached";
    return;
  }

  if (stat.isSnowman() && !isSnowmanNeeded(divCtx, sops, stat))
  {
    _isNeeded = false;
    _skipReason = "snowman has greater number of segments than other option(s) in this fare level";
    return;
  }

  if (!stat._sameConnectingCityAndCxrOpt.empty())
  {
    _isNeeded = false;
    _skipReason = "similar connecting city check";
    return;
  }

  _isNeeded = true;
}

} // ns tse
