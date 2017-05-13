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

#include "Taxes/LegacyTaxes/TaxUI_01.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace std;

namespace
{

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxUI_01>
{
public:
  TaxOnTaxCollector(TaxUI_01& taxUI,
                    PricingTrx& trx,
                    const TaxCodeReg& taxCodeReg,
                    CalculationDetails& details,
                    std::vector<TaxItem*>& taxOnTaxItems,
                    const Itin& itin)
    : Base(taxUI)
    , _trx(trx)
    , _moneyAmount(0.0)
    , _taxCodeReg(taxCodeReg)
    , _details(details)
    , _taxOnTaxItems(taxOnTaxItems)
    , _itin(itin)
    , _diag(_trx.diagnostic().diagnosticType() == Diagnostic818 ? &_trx.diagnostic() : nullptr)
  {
    std::string str = utc::frCorsicaDefinition(_trx, _taxCodeReg);
    if (str.size() < 2)
    {
      if (_diag)
        _diag->insertDiagMsg("CORSICA AIRPORTS ARE NOT DEFINED - UTC CORSICADEFINITION\n");

      return;
    }

    _locCorsicaType = str[0];
    _locCorsicaCode = str.substr(1);

    str = utc::frCoastDefinition(_trx, _taxCodeReg);
    if (str.size() < 2)
    {
      if (_diag)
        _diag->insertDiagMsg("FR COAST AIRPORTS ARE NOT DEFINED - UTC FRCOASTDEFINITION\n");

      return;
    }

    _locCoastType = str[0];
    _locCoastCode = str.substr(1);
  }

  MoneyAmount getMoneyAmount() const { return _moneyAmount; }

  void collect(TaxItem* taxItem)
  {
    for (const TaxCode& taxCode : _taxCodeReg.taxOnTaxCode())
    {
      if (taxItem->taxCode() != taxCode)
        continue;

      MoneyAmount moneyAmount = taxItem->taxAmount();
      if (moneyAmount>EPSILON || _diag)
        moneyAmount = calcMoney(taxItem, moneyAmount);

      _details.taxableTaxes.push_back(std::make_pair(taxCode, moneyAmount));
      _moneyAmount += moneyAmount;
      _taxOnTaxItems.push_back(taxItem);
    }
  }

private:
  PricingTrx& _trx;
  MoneyAmount _moneyAmount;
  const TaxCodeReg& _taxCodeReg;
  CalculationDetails& _details;
  std::vector<TaxItem*>& _taxOnTaxItems;
  const Itin& _itin;
  Diagnostic* _diag;

  LocTypeCode _locCorsicaType;
  LocCode _locCorsicaCode;
  LocTypeCode _locCoastType;
  LocCode _locCoastCode;

  enum SegmentType : unsigned
  {
    UNKNOWN        = 0,

    CORSICA        = 0x1 << 1,
    EXEMPT         = 0x1 << 2,
    METROPOLITAN   = 0x1 << 3,
    CONTINENT      = 0x1 << 4,

    CORSICA_EXEMPT = CORSICA | EXEMPT
  };

  MoneyAmount calcMoney(const TaxItem* taxItem, const MoneyAmount& moneyAmount) const
  {
    unsigned partType = UNKNOWN;

    for (uint16_t index = taxItem->travelSegStartIndex();
         index <= taxItem->travelSegEndIndex(); index++)
    {
      const TravelSeg* seg = _itin.travelSeg()[index];
      if (!seg->isAir())
        continue;

      if (!static_cast<const AirSeg*>(seg)->isFake())
        partType |= static_cast<unsigned>(segmentType(seg));
    }

    if (_diag)
    {
      std::ostringstream stream;
      stream << "TAXITEM-" << taxItem->taxCode() << " "  << moneyAmount << " " <<
        _itin.travelSeg()[taxItem->travelSegStartIndex()]->origAirport() <<
        _itin.travelSeg()[taxItem->travelSegEndIndex()]->destAirport() << " ";

      _diag->insertDiagMsg(stream.str());
    }

    switch (partType)
    {
      case CORSICA:
      case EXEMPT:
      case CORSICA_EXEMPT:
        if (_diag)
          _diag->insertDiagMsg("0 PERCENTAGE - EXEMPT\n");
        return 0;

      case CONTINENT:
        if (_diag)
          _diag->insertDiagMsg("100 PERCENTAGE - CONTINENT\n");
        return 1.0 * moneyAmount;

      case UNKNOWN:
        if (_diag)
          _diag->insertDiagMsg("0 PERCENTAGE - UNKNOWN TYPE\n");
        return 0;
    }

    if (_diag)
      _diag->insertDiagMsg("65 PERCENTAGE - CONTINENT AND CORSICA\n");
    return 0.65 * moneyAmount;
  }

  bool isInLoc(const LocTypeCode& locType, const LocCode& locCode, const Loc& loc) const
  {
    return LocUtil::isInLoc(loc,
                            locType,
                            locCode,
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::TAXES,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            _trx.getRequest()->ticketingDT());
  }

  bool isCoriscaAirport(const Loc& loc) const
  {
    return isInLoc(_locCorsicaType, _locCorsicaCode, loc);
  }

  bool isCoastAirport(const Loc& loc) const
  {
    return isInLoc(_locCoastType, _locCoastCode, loc);
  }

  SegmentType segmentType(const TravelSeg* seg) const
  {
    if (isCoriscaAirport(*seg->origin()))
    {
      if (isCoriscaAirport(*seg->destination()))
        return CORSICA;
      else if (isCoastAirport(*seg->destination()))
        return EXEMPT;
      else
        return METROPOLITAN;
    }
    else if (isCoriscaAirport(*seg->destination()))
    {
      if (isCoastAirport(*seg->origin()))
        return EXEMPT;
      else
        return METROPOLITAN;
    }

    return CONTINENT;
  }

}; //class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxUI_01>

} // namespace anonymous

void
TaxUI_01::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("***\n***UI START SPN PROCESSING***\n***\n");

  TaxOnTaxCollector collector(*this,
    trx,
    taxCodeReg,
    _calculationDetails,
    _taxOnTaxItems,
    *(taxResponse.farePath()->itin()));

  for(TaxItem* taxItem : taxResponse.taxItemVector())
    collector.collect(taxItem);

  if (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath())
  {
    for(TaxItem* taxItem : taxResponse.farePath()->getExternalTaxes())
      collector.collect(taxItem);
  }

  _taxableFare = collector.getMoneyAmount();
  _taxAmount = _taxableFare * taxCodeReg.taxAmt();

  _applyFeeOnTax = true;
  _mixedTax = true;
  _calculationDetails.isTaxOnTax = true;
  _calculationDetails.taxableTaxSumAmount = collector.getMoneyAmount();

  _taxSplitDetails.setUseTaxableTaxSumAmount(true);
  _taxSplitDetails.setFareSumAmount(0.0);

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("***\n***UI END SPN PROCESSING***\n***\n");
}

