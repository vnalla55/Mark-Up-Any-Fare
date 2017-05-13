//----------------------------------------------------------------------------
//
//  File:           RTMinimumFare.cpp
//  Created:        2/28/2011
//  Authors:
//
//  Description:    A class to represent data and methods for RTM
//
//
//  Updates:
//
//  Copyright Sabre 2011
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

#include "MinFares/RTMinimumFare.h"

#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/TrxUtil.h"
#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetRemitFarePath.h"
#include "DBAccess/CarrierPreference.h"
#include "MinFares/HighRTChecker.h"
#include "MinFares/MinFareLogic.h"
#include "Rules/RuleConst.h"

namespace tse
{
static Logger
logger("atseintl.MinFares.RTMinimumFare");

const MoneyAmount RTMinimumFare::ONE_HUNDREDTHS = 0.01;

RTMinimumFare::RTMinimumFare(PricingTrx& trx, FarePath& farePath)
  : MinimumFare(farePath.itin()->travelDate()), _trx(trx), _farePath(farePath)
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    adjustPortExcDates(_trx);
  _isNetRemitFp = typeid(farePath) == typeid(NetRemitFarePath);
}

MoneyAmount
RTMinimumFare::process(PricingUnit& pu)
{
  _isWpNet = false;
  DiagMonitor diagMonitor(_trx, Diagnostic768);
  _diag = &diagMonitor.diag();
  _diagEnabled = (*_diag).isActive();
  printDiagHeader();
  printFareInfo(pu);

  if (LIKELY(!qualifyCat10Subcat102Byte13(pu)))
  {
    if (UNLIKELY(_diagEnabled))
    {
      (*_diag) << "HIGHEST RT CHECK NOT APPLIED - PU DOES NOT QUALIFY CAT 10\n";
    }
    return 0.0;
  }

  if (!HighRTChecker::isSameCabin(pu))
  {
    if (_diagEnabled)
    {
      (*_diag) << "MIXED CABIN - HIGHEST RT CHECK NOT APPLIED\n";
    }
    return 0.0;
  }

  // print the round trip fares:
  printFareInfo(pu, true);

  MoneyAmount HRT_FareAmt = 0;
  if (!HighRTChecker::calculateFareAmt(pu, HRT_FareAmt))
    return 0.0;

  MoneyAmount RT_FareAmt;
  // determine the highest round trip base fare:
  const FareUsage* highestRtFu = HighRTChecker::getHighestRoundTripFare(pu, RT_FareAmt);
  LOG4CXX_INFO(logger, "Highest RT: RT_FareAmt = " << RT_FareAmt);

  MinFarePlusUpItem* hrtPlusUp = getHrtPlusUp(HRT_FareAmt, RT_FareAmt, highestRtFu);
  processNetFares(pu);
  if (!hrtPlusUp)
  {
    return 0.0;
  }
  pu.minFarePlusUp().addItem(HRT, hrtPlusUp);
  return hrtPlusUp->plusUpAmount;
}

void
RTMinimumFare::processNetFares(PricingUnit& pu)
{
  if (!_isNetRemitFp)
  {
    MoneyAmount netFareAmt = 0;
    if ((_isWpNet = HighRTChecker::calculateNetFareAmt(pu, netFareAmt)))
    {
      printDiagHeader();
      printFareInfo(pu);
      printFareInfo(pu, true);
      MoneyAmount RT_NetFareAmt;
      const FareUsage* highestRtNetFu =
          HighRTChecker::getHighestRoundTripNetFare(pu, RT_NetFareAmt);

      MinFarePlusUpItem* netPlusUp = getHrtPlusUp(netFareAmt, RT_NetFareAmt, highestRtNetFu);

      if (netPlusUp)
        pu.hrtcNetPlusUp() = netPlusUp;
    }
  }
}

MinFarePlusUpItem*
RTMinimumFare::getHrtPlusUp(const MoneyAmount& HRT_FareAmt,
                            const MoneyAmount& RT_FareAmt,
                            const FareUsage* highestRtFu) const
{
  if (RT_FareAmt <= (HRT_FareAmt + EPSILON))
  {
    LOG4CXX_INFO(logger,
                 "Highest RT: RT_FareAmt ("
                     << RT_FareAmt << ") is less than, or equal HRT_FareAmt (" << HRT_FareAmt
                     << ") - Highest RT check does not apply");
    return nullptr;
  }

  MinFarePlusUpItem* hrtPlusUp = nullptr;
  _trx.dataHandle().get(hrtPlusUp);
  if (!hrtPlusUp)
  {
    LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (hrtPlusUp)");
    return nullptr;
  }

  HighRTChecker::getHrtPlusUp(HRT_FareAmt, RT_FareAmt, highestRtFu, *hrtPlusUp);
  // Ignoring plusUp that is less than one hundredths (0.01)
  //@todo Apply one_hundredths check to OJ code also.
  if (hrtPlusUp->plusUpAmount >= ONE_HUNDREDTHS)
  {
    printPlusUps(hrtPlusUp);
    return hrtPlusUp;
  }
  return nullptr;
}

void
RTMinimumFare::printPlusUps(const MinFarePlusUpItem* plusUp) const
{
  if (!_diagEnabled)
    return;

  (*_diag) << "''BASE FARE NUC" << std::setw(8) << plusUp->baseAmount << "  " << plusUp->boardPoint
           << plusUp->offPoint << "         PLUS UP" << std::setw(8) << plusUp->plusUpAmount
           << " ''\n";
}

void
RTMinimumFare::printFareInfo(const PricingUnit& pu, bool isRt) const
{
  if (LIKELY(!_diagEnabled))
    return;

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();
  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const PaxTypeFare* rtFare = (**fuIter).paxTypeFare();
    if (!rtFare)
      return;
    printFareInfo(*rtFare, *_diag, HRT, isRt ? "  " : "* ", isRt, 0, _isWpNet);
  }
}

void
RTMinimumFare::printDiagHeader()
{
  if (UNLIKELY(_diagEnabled))
  {
    std::string fp;
    if (_isNetRemitFp)
      fp = "TFD";
    else if (_isWpNet)
      fp = "NET";

    (*_diag) << " \n"
             << "HIGHEST RT CHECK " << fp << std::endl
             << "CITY      GOV  CLASS                         DIR FARE  GLOB EXCL\n"
             << "PAIR      CXR           CUR  AMOUNT  RTG TAG I/O TYPE  IND  IND\n";
  }
}

bool
RTMinimumFare::qualifyCat10Subcat102Byte13(const PricingUnit& pu) const
{
  return HighRTChecker::qualifyCat10Subcat102Byte13(pu);
}
}
