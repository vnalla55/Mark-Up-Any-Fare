#include "Diagnostic/Diag970Collector.h"

#include "Common/FareCalcUtil.h"
#include "Common/Money.h"
#include "DataModel/FarePath.h"
#include "DBAccess/TaxCodeReg.h"
#include "FareCalc/FareCalcCollector.h"
#include "Taxes/LegacyTaxes/TaxItem.h"


namespace tse
{
namespace
{
void
outputTaxCode(std::ostream& dc, const TaxItem* taxItem)
{
  dc << std::setw(6) << taxItem->taxCode();
}

void
outputFailCode(std::ostream& dc, const TaxItem* taxItem)
{
  dc << " ";
  if (taxItem->failCode() == TaxItem::NONE)
    dc << "0";
  else
    dc << taxItem->failCode();
}

void
outputTaxType(std::ostream& dc, const TaxItem* taxItem)
{
  dc << std::setw(4) << taxItem->taxType();
}

void
outputTaxAmounts(std::ostream& dc, const TaxItem* taxItem)
{
  if (taxItem->taxType() != 'P')
    dc << std::setw(8) << taxItem->taxAmount() << taxItem->paymentCurrency() << " " << std::setw(8)
       << taxItem->taxAmount() << " " << std::setw(8) << taxItem->taxableFare();
  else
    dc << std::setw(8) << taxItem->taxAmt() * 100 << "    " << std::setw(8) << taxItem->taxAmount()
       << " " << std::setw(8) << taxItem->taxableFare();
}

void
outputDescription(std::ostream& dc, const TaxItem* taxItem)
{
  if (taxItem->taxDescription().empty())
    dc << "  INTERNATIONAL TOURISM ARRIVAL TAX";
  else
    dc << "  " << taxItem->taxDescription();
}

void
outputRoundRule(std::ostream& dc, const TaxItem* taxItem)
{
  RoundingRule rr = taxItem->taxcdRoundRule();

  switch (rr)
  {
  case UP:
    dc << "  ROUND UP      " << taxItem->taxcdRoundUnit();
    break;
  case DOWN:
    dc << "  ROUND DOWN    " << taxItem->taxcdRoundUnit();
    break;
  case NEAREST:
    dc << "  ROUND NEAREST " << taxItem->taxcdRoundUnit();
    break;
  default:
    dc << "  NO ROUNDING SPECIFIED";
    break;
  };
}

void
outputSequnceNumber(std::ostream& dc, const TaxItem* taxItem)
{
  dc << "  SEQUENCE NUMBER: " << taxItem->seqNo()
     << " FOR CARRIER CODE: " << taxItem->carrierCode();
}

void
outputTaxItem(std::ostream& dc, const TaxItem* taxItem)
{
  Money moneyPayment(taxItem->paymentCurrency());
  dc.precision(moneyPayment.noDec());

  dc << "    ";
  outputTaxCode(dc, taxItem);
  dc << " ";
  outputFailCode(dc, taxItem);
  dc << " ";
  outputTaxType(dc, taxItem);
  dc << " ";
  outputTaxAmounts(dc, taxItem);
  dc << std::endl << "    ";
  outputDescription(dc, taxItem);
  dc << std::endl << "    ";
  outputRoundRule(dc, taxItem);
  dc << std::endl << "    ";
  outputSequnceNumber(dc, taxItem);
  dc << std::endl;
}

} // namespace anon

bool
Diag970Collector::displayHeader(const PricingTrx& trx)
{
  bool result = false;

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    DiagCollector& dc(*this);

    dc << std::endl << "***************************************************************"
       << std::endl << " Diagnostic 970 - splitted taxes " << std::endl;

    if (!trx.getOptions()->isSplitTaxesByFareComponent() && !trx.getOptions()->isSplitTaxesByLeg())
      dc << " Tax splitting logic not enabled " << std::endl;
    else
      result = true;

    dc << "***************************************************************" << std::endl;
  }

  return result;
}

void
Diag970Collector::displayOption(const Itin* itin)
{
  if (!itin)
    return;

  DiagCollector& dc(*this);

  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.setf(std::ios::right, std::ios::adjustfield);

  dc << std::endl << " Option " << (_itinNo++) << std::endl;
}

void
Diag970Collector::displayTaxesByFareUsage(const FareCalc::FcTaxInfo::TaxesPerFareUsage& taxes,
                                          const Itin* itin,
                                          const FarePath* path)
{
  DiagCollector& dc(*this);

  uint16_t segmentOrder = 0;
  for (const TravelSeg* travelSeg : itin->travelSeg())
  {
    if (itin->segmentOrder(travelSeg) < segmentOrder)
      continue;

    for (const PricingUnit* pu : path->pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        FareCalc::FcTaxInfo::TaxesPerFareUsage::const_iterator taxesIter = taxes.find(fu);

        dc << std::endl << "   Fare component " << travelSeg->origAirport() << " - "
           << travelSeg->destAirport() << std::endl << std::endl
           << "    TXCODE FC TYPE TXDEFAMOUNT TXAMOUNT FAREAMNT" << std::endl;
        for (const TaxItem* taxItem : taxesIter->second.taxItems)
          outputTaxItem(dc, taxItem);
      }
    }
  }
}

void
Diag970Collector::displayTaxesByLeg(const FareCalc::FcTaxInfo::TaxesPerLeg& taxes)
{
  DiagCollector& dc(*this);

  for (const FareCalc::FcTaxInfo::TaxesPerLeg::value_type& leg : taxes)
  {
    dc << std::endl << "   Leg " << leg.first << std::endl << std::endl
       << "    TXCODE FC TYPE TXDEFAMOUNT TXAMOUNT FAREAMNT" << std::endl;
    for (const TaxItem* taxItem : leg.second.taxItems)
      outputTaxItem(dc, taxItem);
  }
}

} // namespace tse
