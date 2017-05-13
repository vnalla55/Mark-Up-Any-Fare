// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/TrxUtil.h"
#include "Taxes/LegacyTaxes/TaxSP31.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/Common/PartialTaxableFare.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "Common/Money.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Common/FallbackUtil.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/Tax.h"

using namespace tse;
using namespace std;

namespace tse
{
  FIXEDFALLBACK_DECL(fallbackSplitYQYR_TaxShop);
}

namespace
{

  class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxSP31>
  {
  public:
    TaxOnTaxCollector(TaxSP31& tax)
      : Base(tax)
      , _moneyAmount(0.0)
    {}

    void collect(const TaxResponse::TaxItemVector& taxItemVec, const TaxCode& taxOnTaxCode, const Itin* const itin)
    {
      for (TaxItem* taxItem: taxItemVec)
      {
        if (taxOnTaxCode == taxItem->taxCode())
        {
          if (isWithinPartialSeg(taxItem, itin))
          {
            _moneyAmount += taxItem->taxAmount();
            _tax.updateCalculationDetails(taxItem);
          }
        }
      }
    }

    MoneyAmount getMoneyAmount() const { return _moneyAmount; }
  private:
    bool isWithinPartialSeg(const TaxItem* const taxItem, const Itin* const itin)
    {
      const TravelSeg* travelSegStart = itin->travelSeg()[taxItem->travelSegStartIndex()];
      const TravelSeg* travelSegEnd = itin->travelSeg()[taxItem->travelSegEndIndex()];

      return (itin->segmentOrder(travelSegStart) >= _tax.travelSegPartialStartOrder()) &&
        (itin->segmentOrder(travelSegEnd) <= _tax.travelSegPartialEndOrder());

    }
  private:
    MoneyAmount _moneyAmount;
  };

} // namespace anon

const string
TaxSP31::TAX_CODE_XS("XS");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP31::TaxSP31() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP31::~TaxSP31() {}

// ---------------------------------------
// Description:  TaxSP31::validateTransit
//
// 1. If the taxCode equals XS, returns true.
// 2. If the travelType is DOMESTIC, returns true.
// 3. If the validated segment is the only one in the whole itinerary, returns true.
// 4. If there is any stopover beginning in the domestic part immediately before or ending
//    in the domestic part immediately after the validated segment, returns true.
// 5. If there is any round trip in the domestic part immediately before or immediately
//    after the validated segment, returns true.
// ---------------------------------------
bool
TaxSP31::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  //
  // Tax XS works on all break fares
  //
  if (taxCodeReg.taxCode() == TAX_CODE_XS)
    return true;

  if (taxCodeReg.travelType() == DOMESTIC)
    return true;

  Itin* itin = taxResponse.farePath()->itin();
  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = itin->travelSeg().begin() + travelSegIndex;

  if ((*travelSegI == itin->travelSeg().front()) && (*travelSegI == itin->travelSeg().back()))
    return true;

  if (travelSegI != itin->travelSeg().begin())
  {
    std::vector<TravelSeg*>::const_iterator travelSegFromI = travelSegI;

    for (; travelSegFromI != itin->travelSeg().begin();)
    {
      travelSegFromI--;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegFromI);

      if (!airSeg)
        continue;

      travelSegI = travelSegFromI;

      for (; *travelSegI != itin->travelSeg().back();)
      {
        travelSegI++;
        airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (airSeg)
          break;
      }

      if ((*travelSegFromI)->isForcedStopOver())
        return true;

      if ((*travelSegI)->isStopOver(
              (*travelSegFromI), (*travelSegFromI)->geoTravelType(), TravelSeg::TAXES) &&
          !(*travelSegFromI)->isForcedConx())
        return true;

      if ((*travelSegFromI)->origin()->nation() != (*travelSegI)->destination()->nation())
        break;

      if ((*travelSegFromI)->origin()->loc() == (*travelSegI)->destination()->loc())
        return true;
    }
  }

  travelSegI = itin->travelSeg().begin() + travelSegIndex;

  if (*travelSegI != itin->travelSeg().back())
  {
    std::vector<TravelSeg*>::const_iterator travelSegToI = travelSegI;

    for (; *travelSegToI != itin->travelSeg().back();)
    {
      travelSegToI++;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegToI);

      if (!airSeg)
        continue;

      travelSegI = travelSegToI;

      for (; travelSegI != itin->travelSeg().begin();)
      {
        travelSegI--;
        airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (airSeg)
          break;
      }

      if ((*travelSegI)->isForcedStopOver())
        return true;

      if ((*travelSegToI)
              ->isStopOver((*travelSegI), (*travelSegI)->geoTravelType(), TravelSeg::TAXES) &&
          !(*travelSegI)->isForcedConx())
        return true;

      if ((*travelSegToI)->destination()->nation() != (*travelSegI)->origin()->nation())
        break;

      if ((*travelSegToI)->destination()->loc() == (*travelSegI)->origin()->loc())
        return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

  return false;
}

// ----------------------------------------------------------------------------
// Description:  taxCreate
//
// Determines the partial fare - only if the taxable partial fare and thru
// total fare are equal.
// ----------------------------------------------------------------------------
void
TaxSP31::taxCreate(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t travelSegStartIndex,
                   uint16_t travelSegEndIndex)
{
  _failCode = TaxDiagnostic::NONE;

  // lint -e{530}
  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegStartIndex];
  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if ((*taxItemI)->taxCode() == taxCodeReg.taxCode())
    {
      if (taxResponse.farePath()->itin()->segmentOrder(travelSeg) <=
          (*taxItemI)->travelSegThruEndOrder())
      {
        _taxAmount = 0.0;
        _failCode = TaxDiagnostic::NO_TAX_ADDED;

        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_ADDED, Diagnostic820);
        return;
      }
    }
  }

  PartialTaxableFare partialTaxableFare;

  if ((!partialTaxableFare.locate(trx, taxResponse, taxCodeReg, travelSegStartIndex)) ||
      (partialTaxableFare.thruTotalFare() != partialTaxableFare.taxablePartialFare()))
  {
    _taxAmount = 0.0;
    _failCode = TaxDiagnostic::NO_TAX_ADDED;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_ADDED, Diagnostic820);
    return;
  }

  _partialTax = true;
  _paymentCurrency = partialTaxableFare.paymentCurrency();
  _thruTotalFare = partialTaxableFare.thruTotalFare();
  _taxablePartialFare = partialTaxableFare.taxablePartialFare();
  _partialLocalMiles = partialTaxableFare.partialLocalMiles();
  _partialThruMiles = partialTaxableFare.partialThruMiles();
  _travelSegPartialStartOrder = partialTaxableFare.travelSegLocalStartOrder();
  _travelSegPartialEndOrder = partialTaxableFare.travelSegLocalEndOrder();
  _travelSegThruStartOrder = partialTaxableFare.travelSegThruStartOrder();
  _travelSegThruEndOrder = partialTaxableFare.travelSegThruEndOrder();

  _taxableFare = _taxablePartialFare;
  _taxAmount = taxCodeReg.taxAmt() * _taxablePartialFare;

  Money targetMoney(_paymentCurrency);
  _paymentCurrencyNoDec = targetMoney.noDec(trx.ticketingDate());
}

// ----------------------------------------------------------------------------
// Description:  applyTaxOnTax
//
// Sums up the taxable taxes, basing on the segment order provided by the
// partial fare calculation.
// ----------------------------------------------------------------------------

void
TaxSP31::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (fallback::fixed::fallbackSplitYQYR_TaxShop())
    applyTaxOnTax_old(trx, taxResponse, taxCodeReg);
  else
    applyTaxOnTax_new(trx, taxResponse, taxCodeReg);
}

inline bool hasTaxItems(const TaxResponse& taxResponse)
{
  return !taxResponse.taxItemVector().empty() ||
    (taxResponse.farePath() && !taxResponse.farePath()->getExternalTaxes().empty());
}

void
TaxSP31::applyTaxOnTax_new(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if ((taxCodeReg.taxOnTaxCode().empty()) || !hasTaxItems(taxResponse))
    return;

  TaxOnTaxCollector taxOnTaxCollector(*this);
  const bool taxShoppingRequest = (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath());

  for (const std::string& taxOnTaxCode: taxCodeReg.taxOnTaxCode())
  {
    taxOnTaxCollector.collect(taxResponse.taxItemVector(), taxOnTaxCode, taxResponse.farePath()->itin());
    if (taxShoppingRequest)
      taxOnTaxCollector.collect(taxResponse.farePath()->getExternalTaxes(), taxOnTaxCode, taxResponse.farePath()->itin());
  }

  if (taxOnTaxCollector.getMoneyAmount())
  {
    if (taxCodeReg.taxOnTaxExcl() == YES)
    {
      _taxAmount = taxOnTaxCollector.getMoneyAmount() * taxCodeReg.taxAmt();
      _taxableFare += _taxAmount;
    }
    else
    {
      _taxableFare += taxOnTaxCollector.getMoneyAmount();
      _taxAmount = _taxableFare * taxCodeReg.taxAmt();
    }

    _mixedTax = true;

    if (_taxablePartialFare != _thruTotalFare)
    {
      _taxablePartialFare = _taxableFare;
      return;
    }

    _thruTotalFare = _taxableFare;
    _taxablePartialFare = _taxableFare;
  }
}

void
TaxSP31::applyTaxOnTax_old(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if ((taxCodeReg.taxOnTaxCode().empty()) || (taxResponse.taxItemVector().empty()))
    return;

  MoneyAmount moneyAmount = 0.0;
  TravelSeg* travelSegStart;
  TravelSeg* travelSegEnd;

  std::vector<std::string>::iterator taxOnTaxCodeI;
  std::vector<TaxItem*>::const_iterator taxItemI;

  for (taxOnTaxCodeI = taxCodeReg.taxOnTaxCode().begin();
       taxOnTaxCodeI != taxCodeReg.taxOnTaxCode().end();
       taxOnTaxCodeI++)
  {
    for (taxItemI = taxResponse.taxItemVector().begin();
         taxItemI != taxResponse.taxItemVector().end();
         taxItemI++)
    {
      if ((*taxOnTaxCodeI) == (*taxItemI)->taxCode())
      {
        travelSegStart =
            taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegStartIndex()];
        travelSegEnd =
            taxResponse.farePath()->itin()->travelSeg()[(*taxItemI)->travelSegEndIndex()];

        if ((taxResponse.farePath()->itin()->segmentOrder(travelSegStart) >=
             _travelSegPartialStartOrder) &&
            (taxResponse.farePath()->itin()->segmentOrder(travelSegEnd) <=
             _travelSegPartialEndOrder))
        {
          moneyAmount += (*taxItemI)->taxAmount();
          updateCalculationDetails(*taxItemI);
        }
      }
    }
  }

  if (moneyAmount)
  {
    if (taxCodeReg.taxOnTaxExcl() == YES)
    {
      _taxAmount = moneyAmount * taxCodeReg.taxAmt();
      _taxableFare += _taxAmount;
    }
    else
    {
      _taxableFare += moneyAmount;
      _taxAmount = _taxableFare * taxCodeReg.taxAmt();
    }

    _mixedTax = true;

    if (_taxablePartialFare != _thruTotalFare)
    {
      _taxablePartialFare = _taxableFare;
      return;
    }

    _thruTotalFare = _taxableFare;
    _taxablePartialFare = _taxableFare;
  }
}
