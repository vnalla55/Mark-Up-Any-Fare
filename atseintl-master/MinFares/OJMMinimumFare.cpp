//----------------------------------------------------------------------------
//
//  File:           OJMMinimumFare.cpp
//  Created:        3/4/2004
//  Authors:
//
//  Description:    A class to represent data and methods for OJM
//
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

#include "MinFares/OJMMinimumFare.h"

#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
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
logger("atseintl.MinFares.OJMMinimumFare");

OJMMinimumFare::OJMMinimumFare(PricingTrx& trx, FarePath& farePath)
  : MinimumFare(farePath.itin()->travelDate()), _trx(trx), _farePath(farePath)
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    adjustPortExcDates(_trx);
  _isNetRemitFp = typeid(farePath) == typeid(NetRemitFarePath);
}

MoneyAmount
OJMMinimumFare::process(PricingUnit& pu)
{
  if (UNLIKELY(pu.exemptOpenJawRoundTripCheck()))
    return 0.0;

  _isWpNet = false;
  DiagMonitor diagMonitor(_trx, Diagnostic767);
  _diag = &diagMonitor.diag();
  _diagEnabled = (*_diag).isActive();
  printDiagHeader();
  printFareInfo(pu);

  if (!qualifyCat10Subcat101Byte13(pu))
  {
    if (UNLIKELY(_diagEnabled))
    {
      (*_diag) << "HIGHEST RT OJ NOT APPLIED - PU DOES NOT QUALIFY CAT 10\n";
    }
    return 0.0;
  }

  if (!HighRTChecker::isSameCabin(pu))
  {
    if (_diagEnabled)
    {
      (*_diag) << "MIXED CABIN - HIGHEST RT OJ NOT APPLIED\n";
    }
    return 0.0;
  }
  // print the round trip fares:
  printFareInfo(pu, true);

  MoneyAmount OJ_FareAmt = 0;
  if (!HighRTChecker::calculateFareAmt(pu, OJ_FareAmt))
    return 0.0;

  MoneyAmount RT_FareAmt;
  // determine the highest round trip base fare:
  const FareUsage* highestRtFu = HighRTChecker::getHighestRoundTripFare(pu, RT_FareAmt);
  LOG4CXX_INFO(logger, "Highest RT OJ: RT_FareAmt = " << RT_FareAmt);

  MinFarePlusUpItem* hrtojPlusUp = getHrtojPlusUp(OJ_FareAmt, RT_FareAmt, highestRtFu);
  processNetFares(pu);
  if (!hrtojPlusUp)
  {
    return 0.0;
  }
  pu.minFarePlusUp().addItem(OJM, hrtojPlusUp);
  return hrtojPlusUp->plusUpAmount;
}

void
OJMMinimumFare::processNetFares(PricingUnit& pu)
{
  if (!_isNetRemitFp)
  {
    MoneyAmount OJ_NetFareAmt = 0;
    _isWpNet = HighRTChecker::calculateNetFareAmt(pu, OJ_NetFareAmt);
    if (_isWpNet)
    {
      printDiagHeader();
      printFareInfo(pu);
      printFareInfo(pu, true);
      MoneyAmount RT_NetFareAmt;
      const FareUsage* highestRtNetFu =
          HighRTChecker::getHighestRoundTripNetFare(pu, RT_NetFareAmt);

      MinFarePlusUpItem* hrtojNetPlusUp =
          getHrtojPlusUp(OJ_NetFareAmt, RT_NetFareAmt, highestRtNetFu);

      if (hrtojNetPlusUp)
        pu.hrtojNetPlusUp() = hrtojNetPlusUp;
    }
  }
}

MinFarePlusUpItem*
OJMMinimumFare::getHrtojPlusUp(const MoneyAmount& OJ_FareAmt,
                               const MoneyAmount& RT_FareAmt,
                               const FareUsage* highestRtFu) const
{
  if (RT_FareAmt <= (OJ_FareAmt + EPSILON))
  {
    LOG4CXX_INFO(logger,
                 "Highest RT OJ: RT_FareAmt ("
                     << RT_FareAmt << ") is less than, or equal OJ_FareAmt (" << OJ_FareAmt
                     << ") - Highest RT OJ check does not apply");
    return nullptr;
  }

  MinFarePlusUpItem* hrtojPlusUp = nullptr;
  _trx.dataHandle().get(hrtojPlusUp);
  if (!hrtojPlusUp)
  {
    LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (hrtPlusUp)");
    return nullptr;
  }

  HighRTChecker::getHrtPlusUp(OJ_FareAmt, RT_FareAmt, highestRtFu, *hrtojPlusUp);
  printPlusUps(hrtojPlusUp);
  return hrtojPlusUp;
}

void
OJMMinimumFare::printPlusUps(const MinFarePlusUpItem* ojmPlusUp) const
{
  if (!_diagEnabled)
    return;

  (*_diag) << "''BASE FARE NUC" << std::setw(8) << ojmPlusUp->baseAmount << "  "
           << ojmPlusUp->boardPoint << ojmPlusUp->offPoint << "         PLUS UP" << std::setw(8)
           << ojmPlusUp->plusUpAmount << " ''\n";
}

void
OJMMinimumFare::printFareInfo(const PricingUnit& pu, bool isRt) const
{
  if (LIKELY(!_diagEnabled))
    return;

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();
  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const PaxTypeFare* ojFare = (**fuIter).paxTypeFare();
    if (!ojFare)
      return;
    printFareInfo(*ojFare, *_diag, OJM, isRt ? "  " : "* ", isRt, 0, _isWpNet);
  }
}

void
OJMMinimumFare::printDiagHeader()
{
  if (UNLIKELY(_diagEnabled))
  {
    std::string fp;
    if (_isNetRemitFp)
      fp = "TFD";
    else if (_isWpNet)
      fp = "NET";

    (*_diag) << " \n"
             << "HIGHEST RT OPEN JAW " << fp << std::endl
             << "CITY      GOV  CLASS                         DIR FARE  GLOB EXCL\n"
             << "PAIR      CXR           CUR  AMOUNT  RTG TAG I/O TYPE  IND  IND\n";
  }
}

bool
OJMMinimumFare::qualifyCat10Subcat101Byte13(const PricingUnit& pu) const
{
  return HighRTChecker::qualifyCat10Subcat101Byte13(pu);
}
}
