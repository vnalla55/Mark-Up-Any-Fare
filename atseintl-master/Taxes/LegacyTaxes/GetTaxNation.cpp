// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/TaxNation.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/ColumbiaPOS.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/GetTaxNation.h"

namespace tse
{

FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(taxRefactorDiags);

const std::vector<const TaxNation*>&
GetTaxNation::get() const
{
  return _taxNationVector;
}

const Loc*
GetTaxNation::getPointOfSaleLocation(PricingTrx& trx) const
{
  const Loc* pointOfSaleLocation = TrxUtil::ticketingLoc(trx);

  if (trx.getRequest()->ticketPointOverride().empty())
  {
    pointOfSaleLocation = TrxUtil::saleLoc(trx);
  }

  return pointOfSaleLocation;
}

GetTaxNation::GetTaxNation(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  const CountrySettlementPlanInfo* cspi)
{
  _pointOfSaleLocation = getPointOfSaleLocation(trx);
  const TaxNation* taxNation = getTaxNation(trx, _pointOfSaleLocation->nation());

  addNation(trx, NATION_ALL);

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (trx.getRequest()->getSettlementMethod() == "TCH" ||
        (cspi && cspi->getSettlementPlanTypeCode() == "TCH"))
      addNation(trx, NATION_ALL);
  }
  else
    if (trx.getRequest()->getSettlementMethod() == "TCH")
      addNation(trx, NATION_ALL);

  if (taxNation == nullptr)
    return;

  const AirSeg* airSeg;
  std::vector<TravelSeg*>::iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  if (LocUtil::isColumbia(*_pointOfSaleLocation))
  {
    ColumbiaPOS columbiaPOS;

    if (columbiaPOS.chargeUSTaxes(trx, taxResponse))
      addNation(trx, UNITED_STATES);

    if (columbiaPOS.chargeMXTaxes(trx, taxResponse))
      addNation(trx, MEXICO);

    if (columbiaPOS.chargeAllTaxes(trx, taxResponse))
    {
      for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
      {
        airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        addNation(trx, (*travelSegI)->origin()->nation());

        if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
        {
          addNation(trx, (*travelSegI)->destination()->nation());
        }
      }
    }
  }

  std::vector<const Loc*>::iterator locI;
  bool rv = false;
  bool rc = false;

  switch (taxNation->taxCollectionInd())
  {
  case NONE:
    return;

  case SALE_COUNTRY:

    rv = addNation(trx, _pointOfSaleLocation->nation());

    if (!taxNation->collectionNation1().empty())
    {
      rv = addNation(trx, taxNation->collectionNation1());
    }

    if (!taxNation->collectionNation2().empty())
    {
      rv = addNation(trx, taxNation->collectionNation2());
    }
    return;

  case ALL:

    addNation(trx, TrxUtil::saleLoc(trx)->nation());
    addNation(trx, TrxUtil::ticketingLoc(trx)->nation());

    for (travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
         travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
         travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      addNation(trx, (*travelSegI)->origin()->nation());

      if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
      {
        addNation(trx, (*travelSegI)->destination()->nation());
      }

      for (locI = (*travelSegI)->hiddenStops().begin(); locI != (*travelSegI)->hiddenStops().end();
           locI++)
      {
        addNation(trx, (*locI)->nation());
      }
    }
    break;

  case SELECTED:
  case EXCLUDED:
    rv = addNation(trx, _pointOfSaleLocation->nation());

    for (travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
         travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
         travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      rc = findNationCollect((*travelSegI)->origin()->nation(), *taxNation);

      if (rc)
      {
        rv = addNation(trx, (*travelSegI)->origin()->nation());
      }

      if ((*travelSegI)->origin()->nation() != (*travelSegI)->destination()->nation())
      {
        rc = findNationCollect((*travelSegI)->destination()->nation(), *taxNation);

        if ((rc && taxNation->taxCollectionInd() == SELECTED) ||
            (!rc && taxNation->taxCollectionInd() == EXCLUDED))
        {
          rv = addNation(trx, (*travelSegI)->destination()->nation());
        }
      }
    }
    break;

  default:
    break;
  }

  for (size_t index = 0; index < _taxNationVector.size(); ++index)
  {
    const TaxNation* taxNation = getTaxNation(trx, _taxNationVector[index]->nation());
    if (UNLIKELY(!taxNation))
    {
      continue;
    }
    if (!taxNation->collectionNation1().empty())
    {
      rv = addNation(trx, taxNation->collectionNation1());
      if (rv)
      {
        index = _taxNationVector.size() - 1;
      }
    }
    if (UNLIKELY(!taxNation->collectionNation2().empty()))
    {
      rv = addNation(trx, taxNation->collectionNation2());
      if (rv)
      {
        index = _taxNationVector.size() - 1;
      }
    }
  }
}

bool
GetTaxNation::addNation(PricingTrx& trx, const NationCode& inCountry)
{
  if (findNation(trx, inCountry))
    return false;

  if (!TaxDiagnostic::isValidNation(trx, inCountry))
    return false;

  const TaxNation* taxNation = getTaxNation(trx, inCountry);

  if (UNLIKELY(taxNation == nullptr))
  {
    return false;
  }

  _taxNationVector.push_back(taxNation);

  return true;
}

bool
GetTaxNation::findNation(PricingTrx& trx, const NationCode& inCountry) const
{
  if (!fallback::taxRefactorDiags(&trx))
  {
    auto nationEquals = [&inCountry](const TaxNation* taxNation) { return taxNation->nation() == inCountry; };
    return std::any_of(_taxNationVector.begin(), _taxNationVector.end(), nationEquals);
  }
  else
  {
    std::vector<const TaxNation*>::const_iterator taxNationI;
    for (taxNationI = _taxNationVector.cbegin(); taxNationI != _taxNationVector.cend(); taxNationI++)
    {
      if ((*taxNationI)->nation() == inCountry)
        return true;
    }
    return false;
  }
}

bool GetTaxNation::findNationCollect(const NationCode& inCountry,
    const TaxNation& taxNation) const
{
  auto predicate = [&inCountry] (const NationCode& nationCode)
  {
    return inCountry == nationCode;
  };

  return std::any_of(taxNation.taxNationCollect().begin(),
      taxNation.taxNationCollect().end(), predicate);
}

const TaxNation*
GetTaxNation::getTaxNation(PricingTrx& trx, const NationCode& nation)
{
  const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
  if (exchangeTrx && exchangeTrx->currentTicketingDT().isValid())
  {
    const TaxNation* result = nullptr;
    DateTime originalTicketDate = trx.dataHandle().ticketDate();
    trx.dataHandle().setTicketDate(exchangeTrx->currentTicketingDT());
    result = trx.dataHandle().getTaxNation(nation, exchangeTrx->currentTicketingDT());
    trx.dataHandle().setTicketDate(originalTicketDate);
    return result;
  }
  else
  {
    return trx.dataHandle().getTaxNation(nation, trx.getRequest()->ticketingDT());
  }
}

} // end of tse namespace
