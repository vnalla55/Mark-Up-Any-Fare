#include "Common/CurrencyConversionFacade.h"
#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diag923Collector.h"
#include "Taxes/LegacyTaxes/TaxItem.h"


#include <string>

namespace tse
{

Diag923Collector& Diag923Collector::operator<<(ShoppingTrx& shoppingTrx)
{
  Diag923Collector& dc = *this;
  if (_active)
  {
    _shoppingTrx = &shoppingTrx;

    typedef std::vector<ShoppingTrx::Leg> LegVector;
    typedef std::vector<FareMarket*> FMVector;

    const LegVector& legsVec = _shoppingTrx->legs();

    LegVector::size_type legSize = legsVec.size();
    if (!legSize)
      return dc;

    dc << "***************************************************" << std::endl;
    dc << "923 : CHEAPEST OW/HRT FARE AMOUNT " << std::endl;
    dc << "***************************************************" << std::endl;
    dc << '\n';

    for (LegVector::size_type legNo = 0; legNo < legSize; ++legNo)
    {
      dc << "***************** LEG " << legNo + 1 << " of " << legSize << " *********************"
         << std::endl;
      const ItinIndex& itinIndex = legsVec[legNo].carrierIndex();

      ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
      ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();
      for (; iGIter != iGEndIter; ++iGIter)
      {
        const ItinIndex::ItinCell* curCell =
            ShoppingUtil::retrieveDirectItin(itinIndex, iGIter->first, ItinIndex::CHECK_NOTHING);
        if (curCell)
        {
          Itin* curItin = curCell->second;
          FMVector& fareMarket = curItin->fareMarket();
          for (const auto elem : fareMarket)
          {
            dc << *elem;
            dc << '\n';
          }
        }
      }
    }
  }
  return dc;
}

Diag923Collector& Diag923Collector::operator<<(FareMarket& fareMarket)
{
  Diag923Collector& dc = *this;

  if (_active)
  {
    const CurrencyCode& calculationCurrency = _shoppingTrx->journeyItin()->calculationCurrency();
    const SoloSurcharges::SurchargesDetails* surchargesDetails = nullptr;

    if (_surchargesDetailsMap != nullptr)
    {
      const SoloSurcharges::SurchargesDetailsMap::const_iterator surchargesDetailsIt =
          _surchargesDetailsMap->find(&fareMarket);

      if (surchargesDetailsIt != _surchargesDetailsMap->end() &&
          surchargesDetailsIt->second.size() > 0)
      {
        surchargesDetails = &surchargesDetailsIt->second;
      }
    }

    dc << "FARE MARKET DETAILS: " << std::endl;
    dc << "  Origin: " << fareMarket.origin()->loc() << std::endl;
    dc << "  Destination: " << fareMarket.destination()->loc() << std::endl;
    dc << "  Board multi city: " << fareMarket.boardMultiCity() << std::endl;
    dc << "  Off multi city: " << fareMarket.offMultiCity() << std::endl;
    dc << "  Travel date: " << fareMarket.travelDate().dateToString(DDMMMYY, "") << std::endl;
    dc << "  Marketing carrier: " << fareMarket.governingCarrier() << std::endl;

    if (surchargesDetails == nullptr || surchargesDetails->size() == 0)
    {
      dc << "  ERROR: Surcharges and YQYR not computed." << std::endl;
    }
    else
    {
      dc << "  Surcharges amount: "
         << Money(surchargesDetails->back()._surchargesAmount, calculationCurrency)
         << "\n     (estimated using maximum of first: " << surchargesDetails->size() - 1 << " valid fare(s))"
         << std::endl;
      dc << "  YQYR amount: " << Money(surchargesDetails->front()._taxAmount, calculationCurrency)
         << "\n     (estimation for all fares using first valid fare)" << std::endl;
    }

    dc << "\n";
    dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX\n";
    dc << "                            NUM R I              TYP TYP\n";
    dc << "- -- - ---- --------------- --- - - -------- --- --- ---\n";

    FareMarket::owrt_iterator cheapestOWAmount = fareMarket.ow_begin();
    if (cheapestOWAmount == fareMarket.ow_end())
    {
      dc << "No One Way fare found\n";
    }
    else
    {
      PaxTypeFare& cheapestOW = **cheapestOWAmount;
      dc << " Cheapest OW:\n";
      dc << cheapestOW;
    }

    FareMarket::owrt_iterator cheapestHRTAmount = fareMarket.hrt_begin();
    if (cheapestHRTAmount == fareMarket.hrt_end())
    {
      dc << "  No Half Round Trip fares found\n";
    }
    else
    {
      PaxTypeFare& cheapestHRT = **cheapestHRTAmount;
      dc << " Cheapest HRT:\n";
      dc << cheapestHRT;
    }

    const std::string param =
        _shoppingTrx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);

    if (param == "PLUSUP" || param == "PLUSUP_DETAILS" || param == "ALLFARES")
    {
      if (surchargesDetails != nullptr && surchargesDetails->size() > 0)
      {
        dc << "\nFare used for maximum Surcharges estimation:\n";
        dc << surchargesDetails->back() << "\n";
        dc << "\nFare used for YQYR estimation (for all fares):\n";
        dc << surchargesDetails->front() << "\n";

        if ((param == "PLUSUP_DETAILS" || param == "ALLFARES") && surchargesDetails->size() > 1)
        {
          dc << "\nFull Surcharges calculation for:" << surchargesDetails->size() - 1
             << " fare(s)\n";

          for (size_t surchargesDetailIdx = 0, surchargesDetailSize = surchargesDetails->size() - 1;
               surchargesDetailIdx < surchargesDetailSize;
               ++surchargesDetailIdx)
          {
            dc << "#" << surchargesDetailIdx + 1 << "\n";
            dc << (*surchargesDetails)[surchargesDetailIdx] << "\n";
          }

          if (param == "ALLFARES")
          {
            dc << "All valid fares:\n";
            for (size_t ptfIdx = 0, ptfSize = fareMarket.allPaxTypeFare().size(); ptfIdx < ptfSize;
                 ++ptfIdx)
            {
              PaxTypeFare* paxTypeFare = fareMarket.allPaxTypeFare()[ptfIdx];

              if (paxTypeFare->isValid())
              {
                dc << "#" << ptfIdx + 1 << " NUC fare amount: "
                   << Money(paxTypeFare->nucFareAmount(), calculationCurrency) << "\n";
                dc << *paxTypeFare << "\n";
              }
            }
          }
        }
      }
      else
      {
        dc << " Surcharges/YQYR details: not found." << std::endl;
      }
    }
  }
  return dc;
}

Diag923Collector& Diag923Collector::operator<<(SoloSurcharges::SurchargesDetail surchargesDetail)
{
  if (!_active)
  {
    return *this;
  }

  Diag923Collector& dc = *this;
  const CurrencyCode& calculationCurrency = _shoppingTrx->journeyItin()->calculationCurrency();

  if (surchargesDetail._paxTypeFare->currency() != calculationCurrency)
  {
    const int conversionRoundFactor = 100; // TODO conversion is wrong if amount is small (?)
    const CurrencyCode ptfCurrency = surchargesDetail._paxTypeFare->currency();
    CurrencyConversionFacade ccFacade;
    Money surcharges(ptfCurrency);
    Money tax(ptfCurrency);

    ccFacade.convert(
        surcharges,
        Money(conversionRoundFactor * surchargesDetail._surchargesAmount, calculationCurrency),
        *_shoppingTrx);
    surcharges.value() /= conversionRoundFactor;
    ccFacade.convert(
        tax,
        Money(conversionRoundFactor * surchargesDetail._taxAmount, calculationCurrency),
        *_shoppingTrx);
    tax.value() /= conversionRoundFactor;

    dc << " Surcharges: " << Money(surchargesDetail._surchargesAmount, calculationCurrency) << " ("
       << surcharges << ")"
       << "\tYQYR: " << Money(surchargesDetail._taxAmount, calculationCurrency) << " (" << tax
       << ")\n";
  }
  else
  {
    dc << " Surcharges: " << Money(surchargesDetail._surchargesAmount, calculationCurrency)
       << "\tYQYR: " << Money(surchargesDetail._taxAmount, calculationCurrency) << "\n";
  }

  dc << " NUC fare amount: (old) " << Money(surchargesDetail._orgNucFareAmount, calculationCurrency)
     << " (new) " << Money(surchargesDetail._paxTypeFare->nucFareAmount(), calculationCurrency)
     << "\n";
  dc << " Validating carrier: " << surchargesDetail._validatingCarrier << "\n";
  dc << " Surcharges/YQYR fare:" << std::endl;
  dc << *surchargesDetail._paxTypeFare;
  dc << " Surcharges/YQYR itin travel segments: " << std::endl;

  for (const TravelSeg* travelSeg : surchargesDetail._farePath->itin()->travelSeg())
  {
    dc << *travelSeg << "\n";
  }

  dc << " Surcharges calculation travel segments: " << std::endl;

  for (const TravelSeg* travelSeg : surchargesDetail._farePath->pricingUnit().front()->travelSeg())
  {
    dc << *travelSeg << "\n";
  }

  return dc;
}

Diag923Collector& Diag923Collector::operator<<(const PaxTypeFare& paxFare)
{
  Diag923Collector& dc = *this;
  if (_active)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << cnvFlags(paxFare);

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor())
       << std::setw(5) << paxFare.ruleNumber();

    std::string fareBasis = paxFare.createFareBasis(*_shoppingTrx, false);
    if (fareBasis.size() > 15)
      fareBasis = fareBasis.substr(0, 15) + "*";
    dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";
    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";
    else
      dc << "  ";

    dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

    if (!paxFare.isFareClassAppMissing())
      dc << std::setw(4) << paxFare.fcaFareType();
    else
      dc << "UNK ";

    if (!paxFare.isFareClassAppSegMissing())
    {
      if (paxFare.fcasPaxType().empty())
        dc << "*** ";
      else
        dc << std::setw(4) << paxFare.fcasPaxType();
    }
    else
      dc << "UNK ";

    if (paxFare.isKeepForRoutingValidation())
      dc << " SR";

    dc << '\n';
    if (Vendor::displayChar(paxFare.vendor()) == '*')
    {
      dc << std::setw(5) << " " << std::setw(4) << paxFare.vendor();
    }
    else
      dc << std::setw(9) << " ";
    dc << std::setw(27) << " ";
    dc << std::setw(8) << Money(paxFare.nucFareAmount(), NUC);
    dc << '\n';
  }

  return dc;
}

Diag923Collector& Diag923Collector::operator<<(const TravelSeg& travelSeg)
{
  Diag923Collector& dc = *this;

  if (!_active)
  {
    return dc;
  }

  dc << " " << std::setw(3) << travelSeg.origin()->loc();
  dc << " " << std::setw(3) << travelSeg.destination()->loc();
  std::string depDTStr = travelSeg.departureDT().dateToString(DDMMMYY, "");
  std::string arrDTStr = travelSeg.arrivalDT().dateToString(DDMMMYY, "");
  dc << " " << std::setw(5) << depDTStr;
  dc << " " << std::setw(5) << arrDTStr;

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&travelSeg);

  if (airSeg)
  {
    dc << " " << airSeg->carrier() << " " << airSeg->flightNumber();
  }

  return dc;
}

void
Diag923Collector::setSurchargesDetailsMap(
    const SoloSurcharges::SurchargesDetailsMap* surchargesDetailsMap)
{
  _surchargesDetailsMap = surchargesDetailsMap;
}

} // namespcae tse
