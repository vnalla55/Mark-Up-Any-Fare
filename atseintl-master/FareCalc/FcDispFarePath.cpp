#include "FareCalc/FcDispFarePath.h"

#include "Common/CurrencyConverter.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareAmountAdjustment.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareUsageIter.h"
#include "FareCalc/FcDispFareUsage.h"
#include "FareCalc/FcUtil.h"
#include "Rules/RuleConst.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <vector>

namespace tse
{

namespace
{
Logger logger("atseintl.FareCalc.FcDispFarePath");
}

FcDispFarePath::FcDispFarePath(PricingTrx& trx,
                               const FarePath& farePath,
                               const FareCalcConfig& fcConfig,
                               FareCalcCollector& fcCollector,
                               const CalcTotals* calcTotals,
                               bool useNetPubFbc)
  : FcDispItem(trx, fcConfig, fcCollector),
    _farePath(farePath),
    _calcTotals(calcTotals),
    _useNetPubFbc(useNetPubFbc),
    _noDec(2)
{
  if (_calcTotals == nullptr)
    return;

  if (_farePath.calculationCurrency() == NUC)
  {
    _noDec = 2; // NUC has 2 decimal
  }
  else if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX && farePath.calculationCurrency() != NUC &&
           farePath.calculationCurrency() != farePath.baseFareCurrency())
  {
    _noDec = calcTotals->calcCurrencyNoDec;
  }
  else
  {
    _noDec = _calcTotals->convertedBaseFareNoDec;
  }
}

FcDispFarePath::~FcDispFarePath() {}

std::string
FcDispFarePath::toString() const
{
  if (!_farePath.processed())
    return std::string();

  FareCalc::FcStream os(&_trx, FareCalcConsts::FCL_MAX_LINE_LENGTH, 1);

  if (_calcTotals == nullptr)
    return os.str();

  FareUsageIter fuIter(_farePath);

  FareCalc::FareAmountAdjustment fareAmtAdjustment(*_calcTotals, fuIter.fareUsages());

  FcDispFareUsage dispFareUsage(_trx,
                                _farePath,
                                _calcTotals,
                                *_fcConfig.fcc(),
                                _fcCollector,
                                os,
                                fareAmtAdjustment,
                                _useNetPubFbc);

  std::for_each(fuIter.begin(), fuIter.end(), dispFareUsage);

  displayMinFarePlusUp(os);
  displayDifferential(os, fuIter.fareUsages());
  displayTotalTransferSurcharge(os);
  displayTotalStopOverSurcharge(os);
  displaySurcharges(os);
  displayConsolidatorPlusUp(os, true);
  displayLocalNucCurrency(fareAmtAdjustment, os);
  displayConsolidatorPlusUp(os, false);
  displayROE(os);
  displayBSR(os);
  displayIsiCode(os);
  displayTaxInfo(os);

  _fareCalculationLine = os.ufstr();

  LOG4CXX_DEBUG(logger, "Tkt FareCalc: [" << _fareCalculationLine << "]");
  return os.str();
}

void
FcDispFarePath::displayMinFarePlusUp(FareCalc::FcStream& os) const
{
  std::map<MinimumFareModule, std::string> messages;

  collect_minfare_plusup collectPlusUp(messages, _farePath, _trx);
  collectPlusUp();

  MinimumFareModule modules[] = { BHC, OSC, COM, DMC, CTM, CPM, OJM, RSC, COP, HRT };

  std::map<MinimumFareModule, std::string>::iterator msgIter;
  for (int i = 0, n = sizeof(modules) / sizeof(modules[0]); i < n; i++)
  {
    msgIter = messages.find(modules[i]);
    if (msgIter != messages.end())
    {
      if (_trx.getRequest()->ticketingAgent()->axessUser())
      {
        std::string minFareInfoStr;
        minFareInfoStr += ' ';
        int count = msgIter->second.length();
        char* pMinFareText = &msgIter->second[0];

        for (i = 0; i <= count - 1; i++)
        {
          minFareInfoStr += pMinFareText[i];

          if (pMinFareText[i] == ' ')
          {
            os << minFareInfoStr;
            minFareInfoStr.clear();
          }
        }
        if (minFareInfoStr.length())
        {
          os << minFareInfoStr;
        }
      }
      else
        os << ' ' << msgIter->second;
    }
  }
}

void
FcDispFarePath::displayDifferential(FareCalc::FcStream& os, const std::vector<FareUsage*>& fuv)
    const
{
  MoneyAmount tempDiffAmount = 0;
  std::vector<DifferentialData*>::const_iterator diffIter = _calcTotals->differentialData.begin();
  std::vector<DifferentialData*>::const_iterator diffIterEnd = _calcTotals->differentialData.end();
  for (; diffIter != diffIterEnd; diffIter++)
  {
    if (!(*diffIter))
      continue;
    DifferentialData& di = **diffIter;
    if (di.amount() == 0)
      continue;

    // LOG4CXX_DEBUG(logger, " ORIGIN-LOC: "<<di.origin()->loc()
    //  <<" DEST-LOC: "<<di.destination()->loc());

    //  Display Differential cities, amount and high class
    std::ostringstream outNumb;
    outNumb.setf(std::ios::fixed, std::ios::floatfield);
    outNumb.precision(_noDec);
    if (di.hipAmount() == 0.0)
    {
      outNumb << di.amount();
      tempDiffAmount = di.amount();
    }
    else
    {
      outNumb << di.hipAmount();
      tempDiffAmount = di.hipAmount();
    }

    os << ' ' << "D ";

    // QT - PL 14125: TODO - rewrite this whole function to use FU Iter. Below is the first cut -
    // the obvious fix.

    std::vector<FareUsage*>::const_iterator fuIter;
    fuIter = std::find_if(
        fuv.begin(),
        fuv.end(),
        FareCalc::Equal<FareUsage>(std::mem_fun<const PaxTypeFare*>(&FareUsage::paxTypeFare),
                                   di.throughFare()));
    if (fuIter != fuv.end())
    {
      if ((*fuIter)->isOutbound())
      {
        os << (*di.fareMarket().begin())->boardMultiCity();
        os << (*di.fareMarket().begin())->offMultiCity();
      }
      else
      {
        os << (*di.fareMarket().begin())->offMultiCity();
        os << (*di.fareMarket().begin())->boardMultiCity();
      }
    }
    else
    {
      if (di.throughFare()->fareMarket()->direction() == FMDirection::INBOUND)
      {
        os << (*di.fareMarket().begin())->offMultiCity();
        os << (*di.fareMarket().begin())->boardMultiCity();
      }
      else
      {
        os << (*di.fareMarket().begin())->boardMultiCity();
        os << (*di.fareMarket().begin())->offMultiCity();
      }
    }

    // FareCalc::Group fcGroup(os);

    if (di.travelSeg().size() >= 2 && !di.fareHigh()->isRouting())
    {
      // fcGroup.startGroup();
      std::ostringstream diffPercent;

      if (os.lastCharDigit())
        diffPercent << ' ';

      if (di.fareHigh()->mileageSurchargePctg() > 0)
      {
        diffPercent << di.fareHigh()->mileageSurchargePctg();
      }
      else
      {
        diffPercent << ' ';
      }

      diffPercent << 'M';

      os << diffPercent.str();
    }

    // Process mileage
    PaxTypeFare* ptf = di.throughFare();
    if (!ptf)
    {
      os << outNumb.str();
      os << di.fareClassHigh();
      continue;
    }

    if (!di.hipLowOrigin().empty() || !di.hipLowDestination().empty() ||
        !di.hipHighOrigin().empty() || !di.hipHighDestination().empty()) // HIP
    {
      std::string hipStr;
      hipStr += ' ';
      if (di.hipLowOrigin() == di.hipHighOrigin() &&
          di.hipLowDestination() == di.hipHighDestination()) // 3.
      {
        hipStr += (di.hipLowOrigin() + di.hipLowDestination());
      }
      else if ((di.hipLowOrigin().empty() && di.hipLowDestination().empty()) || // 4.
               (di.hipHighOrigin().empty() && di.hipHighDestination().empty()))
      {
        hipStr += ((di.hipHighOrigin().empty()) ? di.hipCabinLow() : di.hipCabinHigh());
        hipStr += '/';
        hipStr += (di.hipLowOrigin() + di.hipHighOrigin() + di.hipLowDestination() +
                   di.hipHighDestination());
      }
      else // 5.
      {
        hipStr += di.hipCabinHigh();
        hipStr += '/';
        hipStr += (di.hipHighOrigin() + di.hipHighDestination());
        hipStr += ' ';
        hipStr += di.hipCabinLow();
        hipStr += '/';
        hipStr += (di.hipLowOrigin() + di.hipHighDestination());
      }
      os << hipStr;
    }

    os << std::fixed << std::setprecision(_noDec) << tempDiffAmount;

    if ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
        (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _farePath)))
    {
      os << di.fareClassHigh();

      /*        bool isc = PaxTypeUtil::isChild  (_trx, _calcTotals->farePath->paxType()->paxType(),
       ptf->vendor());
       bool isi = PaxTypeUtil::isInfant (_trx, _calcTotals->farePath->paxType()->paxType(),
       ptf->vendor());
       if (ptf->isDiscounted() && (isc || isi))
       {
       std::string cpt = (isc) ? FareCalcConsts::FC_APPEND_CHILD : FareCalcConsts::FC_APPEND_INFANT;
       switch (_fcConfig.fcChildInfantFareBasis())
       {
       case  FareCalcConsts::FC_ONE:
       os <<  cpt;
       break;
       case  FareCalcConsts::FC_TWO:
       os <<  ('/' + cpt);
       break;
       case  FareCalcConsts::FC_THREE:
       MoneyAmount discPCT = 0.0f;
       try {
       discPCT = ptf->discountInfo().discPercent();
       }
       catch (...) {}// already inited to usable value

       if (discPCT)
       {
       os << '/' << cpt << convertAmount(discPCT, 0);
       }
       break;
       }  // end of switch
       }  // end of  if (ptf->isDiscounted() && (isc || isi)) */
    } // end of if (_fcConfig.fareBasisDisplayOption () == FC_YES)

  } // end for (; diffIter != diffIterEnd; diffIter++)

  if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    differentialForExchange(os);
}

void
FcDispFarePath::displayTotalTransferSurcharge(FareCalc::FcStream& os) const
{
  if (_calcTotals->transferSurcharge.count != 0 && _calcTotals->transferSurcharge.total > 0)
  {
    os << " " << _calcTotals->transferSurcharge.count << "S";
    os << std::fixed << std::setprecision(2) << _calcTotals->transferSurcharge.total;
  }
}

void
FcDispFarePath::displayTotalStopOverSurcharge(FareCalc::FcStream& os) const
{
  if (_calcTotals->stopOverSurcharge.count != 0 && _calcTotals->stopOverSurcharge.total > 0)
  {
    os << " " << _calcTotals->stopOverSurcharge.count << "S";
    os << std::fixed << std::setprecision(2) << _calcTotals->stopOverSurcharge.total;
  }
}

void
FcDispFarePath::displayLocalNucCurrency(const FareCalc::FareAmountAdjustment& fareAmtAdjustment,
                                        FareCalc::FcStream& os) const
{
  LOG4CXX_DEBUG(logger, "CALC CUR : " << _farePath.calculationCurrency());
  LOG4CXX_DEBUG(logger, "ORIG CUR : " << _farePath.itin()->originationCurrency());

  if ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
      (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _farePath)))
  {
    os << ' ';
  }
  else
  {
    if (os.lastCharAlpha())
      os << ' ';
  }

  os << _farePath.calculationCurrency();

  bool isDomesticPeru = ItinUtil::isDomesticPeru(&_trx, _farePath.itin());

  if (fareAmtAdjustment.isAdjusted())
  {
    if (isDomesticPeru && _farePath.calculationCurrency() != NUC)
      (const_cast<FarePath&>(_farePath)).unroundedTotalNUCAmount() += fareAmtAdjustment.adjAmount();
    else
      (const_cast<FarePath&>(_farePath)).increaseTotalNUCAmount(fareAmtAdjustment.adjAmount());
  }

  LOG4CXX_DEBUG(logger, "FP TOTAL AMOUNT: " << _farePath.getTotalNUCAmount());

  if (isDomesticPeru && _farePath.calculationCurrency() != NUC)
    os << convertAmount(_farePath.unroundedTotalNUCAmount(), _noDec, _noDec);
  else
    os << convertAmount(_farePath.getTotalNUCAmount(), _noDec, _noDec);
}

void
FcDispFarePath::displayConsolidatorPlusUp(FareCalc::FcStream& os, bool firstPosition) const
{
  if (LIKELY(!_farePath.itin()->isPlusUpPricing()))
    return;

  FarePath fp = _farePath;
  ConsolidatorPlusUp* cPlusUp = fp.itin()->consolidatorPlusUp();

  if ((firstPosition && !cPlusUp->isCanadianPointOfSale()) ||
      (!firstPosition && cPlusUp->isCanadianPointOfSale()))
    return;

  os << " PLUS" << convertAmount(cPlusUp->fareCalcAmount(), _noDec, _noDec);
}

void
FcDispFarePath::displayROE(FareCalc::FcStream& os) const
{
  FareCalc::Group fcGroup(os, true);
  os << "END";
  if (_calcTotals->fclROE.size() > 1)
  {
    os << _calcTotals->fclROE;
  }
}

void
FcDispFarePath::displayBSR(FareCalc::FcStream& os) const
{
  if (_calcTotals->fclBSR.length() > 1)
    os << ' ' << _calcTotals->fclBSR.substr(1);
}

void
FcDispFarePath::displayIsiCode(FareCalc::FcStream& os) const
{
  if (_trx.getRequest()->diagnosticNumber() == Diagnostic854 ||
      (_fcConfig.domesticISI() == FareCalcConsts::FC_YES &&
       _farePath.calculationCurrency() == _farePath.itin()->originationCurrency()) ||
      (_fcConfig.internationalISI() == FareCalcConsts::FC_YES &&
       _farePath.calculationCurrency() != _farePath.itin()->originationCurrency()))
  {
    if (os.lastCharAlpha())
      os << ' ';
    os << _fcCollector.IATASalesCode();
  }
}

void
FcDispFarePath::displayTaxInfo(FareCalc::FcStream& os) const
{
  const std::vector<std::string>& publishedZpTaxInfo = _calcTotals->publishedZpTaxInfo();
  if (publishedZpTaxInfo.size() > 0)
  {
    os << ' ' << FareCalcConsts::TAX_CODE_ZP;
    std::copy(publishedZpTaxInfo.begin(),
              publishedZpTaxInfo.end(),
              std::ostream_iterator<std::string>(os));
  }
  const std::vector<std::string>& xtTaxInfo = _calcTotals->xtTaxInfo();
  if (xtTaxInfo.size() > 0)
  {
    os << ' ' << FareCalcConsts::TAX_CODE_XT;
    std::copy(xtTaxInfo.begin(), xtTaxInfo.end(), std::ostream_iterator<std::string>(os));
  }

  if (_fcConfig.fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
  {
    const std::vector<std::string>& xfTaxInfo = _calcTotals->xfTaxInfo();
    if (xfTaxInfo.size() > 0)
    {
      os << ' ';
      std::copy(xfTaxInfo.begin(), xfTaxInfo.end(), std::ostream_iterator<std::string>(os));
    }
  }
  else if (_trx.getRequest()->isTicketEntry() || !applicableTaxesMoreThanTaxBox())
  {
    const std::vector<std::string>& xfTaxInfo = _calcTotals->xfTaxInfo();
    if (xfTaxInfo.size() > 0)
    {
      os << ' ' << FareCalcConsts::TAX_CODE_XF;
      std::copy(xfTaxInfo.begin() + 1, xfTaxInfo.end(), std::ostream_iterator<std::string>(os));
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// collect_minfare_plusup
//////////////////////////////////////////////////////////////////////////////

/**
 * Collect all plus up message within the fare path
 */
void
collect_minfare_plusup::
operator()()
{
  std::for_each(_farePath.pricingUnit().begin(), _farePath.pricingUnit().end(), *this);

  // For COM and DMC across PU plus ups.
  const std::vector<FarePath::PlusUpInfo*>& fpPlusUps = _farePath.plusUpInfoList();
  std::vector<FarePath::PlusUpInfo*>::const_iterator fpPlusUpIter = fpPlusUps.begin();
  for (; fpPlusUpIter != fpPlusUps.end(); fpPlusUpIter++)
  {
    insertPlusUpMessage((*fpPlusUpIter)->module(), *((*fpPlusUpIter)->minFarePlusUp()));
  }

  // For OSC
  const std::vector<FarePath::OscPlusUp*>& oscPlusUps = _farePath.oscPlusUp();
  std::vector<FarePath::OscPlusUp*>::const_iterator oscIter = oscPlusUps.begin();
  for (; oscIter != oscPlusUps.end(); oscIter++)
  {
    insertPlusUpMessage(OSC, **oscIter);
  }

  // For RSC
  const std::vector<FarePath::RscPlusUp*>& rscPlusUps = _farePath.rscPlusUp();
  std::vector<FarePath::RscPlusUp*>::const_iterator rscIter = rscPlusUps.begin();
  for (; rscIter != rscPlusUps.end(); rscIter++)
  {
    insertPlusUpMessage(RSC, **rscIter);
  }
}

void
collect_minfare_plusup::
operator()(const FareUsage* fareUsage)
{
  const MinFarePlusUp& plusUps = fareUsage->minFarePlusUp();
  MinFarePlusUp::const_iterator plusUpIter = plusUps.begin();
  for (; plusUpIter != plusUps.end(); plusUpIter++)
  {
    if (plusUpIter->first == BHC)
    {
      // Find corresponding HIP plus up
      MinFarePlusUp::const_iterator hipIter = plusUps.find(HIP);
      if (hipIter != plusUps.end())
      {
        insertPlusUpMessage(plusUpIter->first, *(plusUpIter->second), hipIter->second);
        continue;
      }
    }
    insertPlusUpMessage(plusUpIter->first, *(plusUpIter->second));
  }
}

void
collect_minfare_plusup::
operator()(PricingUnit* pricingUnit)
{
  std::for_each(pricingUnit->fareUsage().begin(), pricingUnit->fareUsage().end(), *this);

  MinFarePlusUp& plusUps = pricingUnit->minFarePlusUp();
  if ( _trx.getOptions()->isCat35Net() || _trx.getRequest()->isWpNettRequested() )
  {
    plusUps.erase(OJM);
    if (pricingUnit->hrtojNetPlusUp())
      plusUps.addItem(OJM, pricingUnit->hrtojNetPlusUp());
  }

  if (_trx.getOptions()->isCat35Net() || _trx.getRequest()->isWpNettRequested())
  {
    plusUps.erase(HRT);
    if (pricingUnit->hrtcNetPlusUp())
      plusUps.addItem(HRT, pricingUnit->hrtcNetPlusUp());
  }

  MinFarePlusUp::const_iterator plusUpIter = plusUps.begin();
  for (; plusUpIter != plusUps.end(); plusUpIter++)
  {
    insertPlusUpMessage(plusUpIter->first, *(plusUpIter->second));
  }
}

void
collect_minfare_plusup::insertPlusUpMessage(MinimumFareModule module,
                                            const MinFarePlusUpItem& plusUp,
                                            const MinFarePlusUpItem* hipPlusUp)
{
  std::map<MinimumFareModule, std::string>::iterator msgIter = _messages.find(module);
  if (msgIter == _messages.end())
  {
    _messages.insert(std::make_pair(module, getPlusUpStr(module, plusUp, hipPlusUp)));
  }
  else
  {
    if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    {
      msgIter->second += ' '; // adding a space between min fare info
    }
    msgIter->second += getPlusUpStr(module, plusUp);
  }
}

std::string
collect_minfare_plusup::getPlusUpStr(MinimumFareModule module,
                                     const MinFarePlusUpItem& plusUp,
                                     const MinFarePlusUpItem* hipPlusUp)
{
  std::stringstream msgStream;
  msgStream.setf(std::ios::fixed, std::ios::floatfield);
  msgStream.precision(2);

  switch (module)
  {
  case HIP:
    // HIP is displayed within Fare Component. It is not handled here.
    break;
  case BHC:
  {
    msgStream << "P ";
    if (!plusUp.constructPoint.empty())
      msgStream << "C/" << plusUp.constructPoint << " ";

    // Port Exchange - Override BHC does not require associated override HIP
    if ((hipPlusUp) || (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    {
      const BhcPlusUpItem& bhcPlusUp = dynamic_cast<const BhcPlusUpItem&>(plusUp);
      msgStream << bhcPlusUp.boardPoint << bhcPlusUp.offPoint << " " << bhcPlusUp.fareBoardPoint
                << bhcPlusUp.fareOffPoint << bhcPlusUp.plusUpAmount;
    }
  }
  break;

  case CTM:
  case COM:
  case DMC:
  case COP:
  case OJM:
  case HRT:
  {
    msgStream << "P ";
    if (!plusUp.constructPoint.empty())
      msgStream << "C/" << plusUp.constructPoint << " ";
    msgStream << plusUp.boardPoint << plusUp.offPoint << plusUp.plusUpAmount;
  }
  break;

  case CPM:
  {
    msgStream << "P R/";
    msgStream << plusUp.boardPoint << plusUp.offPoint << plusUp.plusUpAmount;
  }
  break;

  case OSC:
  {
    const FarePath::OscPlusUp* oscPlusUp = dynamic_cast<const FarePath::OscPlusUp*>(&plusUp);
    if (oscPlusUp != nullptr)
    {
      msgStream << "H ";
      msgStream << plusUp.boardPoint << plusUp.offPoint << plusUp.plusUpAmount;
    }
  }
  break;

  case RSC:
  {
    const FarePath::RscPlusUp* rscPlusUp = dynamic_cast<const FarePath::RscPlusUp*>(&plusUp);
    if (rscPlusUp != nullptr)
    {
      msgStream << "U ";
      if (!rscPlusUp->constructPoint.empty())
        msgStream << "C/" << rscPlusUp->constructPoint << " ";

      msgStream << rscPlusUp->boardPoint << rscPlusUp->offPoint;
      if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
      {
        if (!rscPlusUp->constructPoint2.empty())
          msgStream << " C/" << rscPlusUp->constructPoint2;

        if ((!rscPlusUp->inboundBoardPoint.empty()) && (!rscPlusUp->inboundOffPoint.empty()))
          msgStream << " " << rscPlusUp->inboundBoardPoint << rscPlusUp->inboundOffPoint;
      }
      else
      {
        if ((rscPlusUp->boardPoint != rscPlusUp->inboundOffPoint) ||
            (rscPlusUp->offPoint != rscPlusUp->inboundBoardPoint))
          msgStream << rscPlusUp->inboundOffPoint << rscPlusUp->inboundBoardPoint;
      }
      if (_trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
      {
        if (rscPlusUp->plusUpAmount > 0)
          msgStream << rscPlusUp->plusUpAmount;
      }
      else
      {
        msgStream << rscPlusUp->plusUpAmount;
      }
    }
  }
  break;

  default:
    break;
  }

  return msgStream.str();
}

bool
FcDispFarePath::applicableTaxesMoreThanTaxBox() const
{
  int16_t applicableTaxes = 0;

  // copy the counting number of taxable taxes concept from FareCalculation::getTaxOverride()
  std::vector<TaxOverride*>::const_iterator taxI = _trx.getRequest()->taxOverride().begin();

  for (; taxI != _trx.getRequest()->taxOverride().end(); taxI++)
  {
    applicableTaxes++;
  }

  // copy the counting number of taxable taxes concept from FareCalculation::getTotalXT()
  if (_farePath.itin()->getTaxResponses().empty())
    return false;

  if (const TaxResponse* taxResponse = TaxResponse::findFor(&_farePath))
  {
    for (const TaxRecord* taxRecord : taxResponse->taxRecordVector())
    {
      if ((taxRecord->getTaxAmount() < EPSILON) &&
          !taxRecord->isTaxFeeExempt()) // Skip zero amount Tax.
      {
        continue;
      }
      applicableTaxes++;
    }
  }
  if (applicableTaxes > (_fcConfig.noofTaxBoxes() - '0')) // get nbr tax box from database
  {
    return true;
  }
  return false;
}

void
FcDispFarePath::differentialForExchange(FareCalc::FcStream& os) const
{
  ExchangePricingTrx* excTrx = dynamic_cast<ExchangePricingTrx*>(&_trx);
  if (excTrx == nullptr)
    return;

  if (excTrx->exchangeOverrides().differentialOverride().empty())
    return;

  std::vector<DifferentialOverride*>::iterator diffOverI =
      excTrx->exchangeOverrides().differentialOverride().begin();
  std::vector<DifferentialOverride*>::iterator diffOverE =
      excTrx->exchangeOverrides().differentialOverride().end();

  for (; diffOverI != diffOverE; ++diffOverI)
  {
    DifferentialOverride& diffOver = *(*diffOverI);
    displayDifferentialForExchange(os, diffOver);
  }
}

void
FcDispFarePath::displayDifferentialForExchange(FareCalc::FcStream& os,
                                               DifferentialOverride& diffOver) const
{
  os << ' ' << "D ";
  os << diffOver.highDiffFareOrigin(); // A13 - Differential Origin
  os << diffOver.highDiffFareDestination(); // A14 - Differential Destination

  if (!diffOver.mileageOnDiff().empty()) // Mileage Surcharge Differential
  {
    if (diffOver.mileageOnDiff()[0] == '0')
      os << " M";
    else
      os << diffOver.mileageOnDiff() << "M";
  }

  if (!diffOver.highHipOrigin().empty() || !diffOver.highHipDestination().empty() ||
      !diffOver.lowHipOrigin().empty() || !diffOver.lowHipDestination().empty()) // HIP
  {
    std::string hipStr;
    hipStr += ' ';
    if (diffOver.lowHipOrigin() == diffOver.highHipOrigin() &&
        diffOver.lowHipDestination() == diffOver.highHipDestination()) // 3.
    {
      hipStr += (diffOver.lowHipOrigin() + diffOver.lowHipDestination());
    }
    else if ((diffOver.lowHipOrigin().empty() && diffOver.lowHipDestination().empty()) || // 4.
             (diffOver.highHipOrigin().empty() && diffOver.highHipDestination().empty()))
    {
      if (!diffOver.hipConstructedCity().empty())
      {
        hipStr += "C/";
        hipStr += diffOver.hipConstructedCity();
        hipStr += ' ';
        hipStr += (diffOver.highHipOrigin() + diffOver.highHipDestination());
      }
      else
      {
        hipStr += ((diffOver.highHipOrigin().empty()) ? diffOver.lowFareCabin()
                                                      : diffOver.highFareCabin());
        hipStr += '/';
        hipStr += (diffOver.lowHipOrigin() + diffOver.highHipOrigin() +
                   diffOver.lowHipDestination() + diffOver.highHipDestination());
      }
    }
    else // 5.
    {
      hipStr += diffOver.highFareCabin();
      hipStr += '/';
      hipStr += (diffOver.highHipOrigin() + diffOver.highHipDestination());
      hipStr += ' ';
      hipStr += diffOver.lowFareCabin();
      hipStr += '/';
      hipStr += (diffOver.lowHipOrigin() + diffOver.lowHipDestination());
    }
    os << hipStr;
  }

  os << std::fixed << std::setprecision(_noDec) << diffOver.amount(); // C50 - Differential amount

  if ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
      (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _farePath)))
    os << diffOver.highFareClass();
}

void
FcDispFarePath::displaySurcharges(FareCalc::FcStream& os) const
{
  uint16_t surchargesCount = 0;
  const SurchargeData* previous = nullptr;
  std::map<const TravelSeg*, std::vector<SurchargeData*> >::const_iterator mit =
      _calcTotals->surcharges.begin();
  std::map<const TravelSeg*, std::vector<SurchargeData*> >::const_iterator mie =
      _calcTotals->surcharges.end();
  for (; mit != mie; ++mit)
  {
    std::vector<SurchargeData*>::const_iterator it = mit->second.begin();
    std::vector<SurchargeData*>::const_iterator ie = mit->second.end();

    for (; it != ie; ++it)
    {
      if ((*it)->amountNuc() != 0 && (*it)->selectedTkt() && (*it)->fcFpLevel())
      {
        MoneyAmount surcharge = (*it)->amountNuc() * (*it)->itinItemCount();
        if (surchargesCount == 0)
        {
          if ((_fcConfig.fareBasisDisplayOption() == FareCalcConsts::FC_YES) ||
              (TrxUtil::getValidatingCxrFbcDisplayPref(_trx, _farePath)))
            os << ' ';
        }
        else if (os.lastCharAlpha() || _fcConfig.multiSurchargeSpacing() == FareCalcConsts::FC_YES)
          os << ' ';

        FareCalc::Group fcGroup(os, true);
        if (previous && (previous->fcBrdCity() == (*it)->fcBrdCity() &&
                         previous->fcOffCity() == (*it)->fcOffCity()))
        {
          if (!os.lastCharSpace())
            os << " ";
        }
        else
          os << "Q " << (*it)->fcBrdCity() << (*it)->fcOffCity();

        os << std::fixed << std::setprecision(_noDec) << surcharge;

        previous = *it;
        surchargesCount++;
      }
    }
  }
}

} // namespace tse
