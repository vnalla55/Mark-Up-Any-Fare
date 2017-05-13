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

#include "Taxes/LegacyTaxes/TaxXG.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

using namespace tse;
using namespace std;

const string
TaxXG::TAX_CODE_XG("XG");
const string
TaxXG::TAX_CODE_XG1("XG1");
const string
TaxXG::TAX_CODE_XG3("XG3");
const string
TaxXG::TAX_CODE_XG4("XG4");
const string
TaxXG::TAX_CODE_CA1("CA1");
const string
TaxXG::TAX_CODE_SQ("SQ");
const string
TaxXG::TAX_CODE_SQ1("SQ1");
const string
TaxXG::TAX_CODE_SQ3("SQ3");

namespace
{

// ----------------------------------------------------------------------------
// Description:  applyTaxOnCA1Tax
// Following taxes XG, XG1, XG4 should be applied on tax CA1.
// ----------------------------------------------------------------------------
inline bool applyTaxOnCA1Tax(const TaxCode& taxCodeReg, bool zeroesBaseFare)
{
  return (taxCodeReg == TaxXG::TAX_CODE_XG && !zeroesBaseFare) ||
         (taxCodeReg == TaxXG::TAX_CODE_XG4 && !zeroesBaseFare) ||
         (taxCodeReg == TaxXG::TAX_CODE_XG1);
}

class TaxOnTaxCollector : public BaseTaxOnTaxCollector<TaxXG>
{
public:
  TaxOnTaxCollector(TaxXG& taxXG)
    : Base(taxXG)
    , _taxAmountCA1(0.0)
    , _taxAmountSQ(0.0)
    , _taxAmountSQ1(0.0)
    , _taxAmountSQ3(0.0)
  {}

  void collect(const TaxResponse::TaxItemVector& taxItemVec, const TaxCode& taxCodeReg, const TaxCode& taxOnTaxCode)
  {
    TaxResponse::TaxItemVector::const_iterator taxItemI;
    for (taxItemI = taxItemVec.begin(); taxItemI != taxItemVec.end(); taxItemI++)
    {

      if (taxOnTaxCode == (*taxItemI)->taxCode())
      {
        if (applyTaxItem(_taxAmountCA1, *taxItemI, TaxXG::TAX_CODE_CA1))
        {
          if (applyTaxOnCA1Tax(taxCodeReg, _tax.zeroesBaseFare()))
            _tax.updateCalculationDetails(*taxItemI);
        }
        else if (applyTaxItem(_taxAmountSQ, *taxItemI, TaxXG::TAX_CODE_SQ))
        {
          if (taxCodeReg == TaxXG::TAX_CODE_XG)
            _tax.updateCalculationDetails(*taxItemI);
        }
        else if (applyTaxItem(_taxAmountSQ1, *taxItemI, TaxXG::TAX_CODE_SQ1))
        {
          if (taxCodeReg == TaxXG::TAX_CODE_XG)
            _tax.updateCalculationDetails(*taxItemI);
        }
        else if (applyTaxItem(_taxAmountSQ3, *taxItemI, TaxXG::TAX_CODE_SQ3))
        {
          if (taxCodeReg == TaxXG::TAX_CODE_XG3)
            _tax.updateCalculationDetails(*taxItemI);
        }
        else if ((*taxItemI)->serviceFee())
        {
          _tax.setTaxableFare(_tax.taxableFare() + (*taxItemI)->taxAmount());
          continue;
        }
      }
    }
  }

  void setTaxAmountCA1(MoneyAmount value) { _taxAmountCA1 = value; }
  MoneyAmount taxAmountCA1() const { return _taxAmountCA1; }
  MoneyAmount taxAmountSQ() const { return _taxAmountSQ; }
  MoneyAmount taxAmountSQ1() const { return _taxAmountSQ1; }
  MoneyAmount taxAmountSQ3() const { return _taxAmountSQ3; }
private:
  MoneyAmount _taxAmountCA1;
  MoneyAmount _taxAmountSQ;
  MoneyAmount _taxAmountSQ1;
  MoneyAmount _taxAmountSQ3;
};

} // namespace anonymous

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxXG::TaxXG()
  : _canadianPt(false),
    _whollyCA(true),
    _origCAMaritime(false),
    _origCANotMaritime(false),
    _origUS(false),
    _usPoint(false),
    _hawaiianPt(false),
    _mexicanPt(false),
    _otherIntl(false),
    _zeroesBaseFare(false),
    _pointOfSaleLocation(nullptr)
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxXG::~TaxXG() {}

// ----------------------------------------------------------------------------
// Description:  TripTypesValidator
// ----------------------------------------------------------------------------

bool
TaxXG::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{

  return validateFromTo(trx, taxResponse, taxCodeReg, startIndex, endIndex);

} // end of validateTrip

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::validateFromToBetween
//
// Description:  This function will validate TAX_FROM_TO and TAX_BETWEEN
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
// @param  endIndex - last TravelSeg to search to
//
// @return bool - true : tax applied;
//                false : tax is not applied;
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxXG::validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex)

{
  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  _failCode = 0;
  _zeroesBaseFare = false;

  _canadianPt = false;
  _whollyCA = true;
  _origCAMaritime = false;
  _origCANotMaritime = false;
  _origUS = false;
  _usPoint = false;
  _hawaiianPt = false;
  _mexicanPt = false;
  _otherIntl = false;

  uint16_t index = 0;

  // Check for the origin Maritime Canada - NewBrunswick/NovaScotia/NewFoundLand
  if (LocUtil::isCanada(*(*travelSegI)->origin()))
  {
    if (((*travelSegI)->origin()->state() == NEW_BRUNSWICK) ||
        ((*travelSegI)->origin()->state() == NOVA_SCOTIA) ||
        ((*travelSegI)->origin()->state() == NEW_FOUNDLAND))
    {
      _origCAMaritime = true;
    }
    else
    {
      _origCANotMaritime = true;
    }
  } // end board is in Canada.

  // Check for the origin in USA excluding PR/VI
  if ((LocUtil::isUS(*(*travelSegI)->origin())) &&
      (!LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin())))
  {
    _origUS = true;
  }

  for (; index <= endIndex; ++index, ++travelSegI)
  {
    const Loc& orig = *(*travelSegI)->origin();
    const Loc& dest = *(*travelSegI)->destination();

    // Check for wholly in US -- true if any brd/off in USA.
    if (LocUtil::isUS(orig) || LocUtil::isUS(dest))
    {
      _usPoint = true;
    }
    if (LocUtil::isHawaii(orig) || LocUtil::isHawaii(dest))
    {
      _hawaiianPt = true;
    }
    // Check for wholly in Canada -- true if any brd/off in Canada.
    if (LocUtil::isCanada(orig) || LocUtil::isCanada(dest))
    {
      _canadianPt = true;
    }
    // Check for total travel in Canada - true if all brd+off are in Canada.
    if (!LocUtil::isCanada(orig) || !LocUtil::isCanada(dest))
    {
      _whollyCA = false;
    }
    // Check for wholly in Mexico -- true if any brd/off in Mexico.
    if (LocUtil::isMexico(orig) || LocUtil::isMexico(dest))
    {
      _mexicanPt = true;
    }
    // Check for International
    if ((!LocUtil::isUS(orig) && !LocUtil::isCanada(orig)) ||
        (!LocUtil::isUS(dest) && !LocUtil::isCanada(dest)))
    {
      _otherIntl = true;
    }
    if (LocUtil::isUSTerritoryOnly(orig) || LocUtil::isUSTerritoryOnly(dest))
    {
      _otherIntl = true;
    }
  } // Loop TravelSeg set Indicators

  // Find PointOfSale
  _pointOfSaleLocation = TrxUtil::ticketingLoc(trx);

  if (trx.getRequest()->ticketPointOverride().empty())
  {
    _pointOfSaleLocation = TrxUtil::saleLoc(trx);
  }

  // Start index must be set to zero to get correct board city/carrier (for diagnostics and WPDF)
  startIndex = 0;

  // Perform each specific routine Taxes
  if (taxCodeReg.taxCode() == TAX_CODE_XG1)
    return validateXG1(trx, taxResponse, taxCodeReg);
  else
    return validateXG(trx, taxResponse, taxCodeReg);

  //  return false;
}

// ----------------------------------------------------------------------------
// Description:  Tax SPN# 27 - XG/XG3/XG4 Process
// ----------------------------------------------------------------------------
bool
TaxXG::validateXG(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  // Check for XG/XG3/XG4 taxes.
  bool geoMatch = true;
  bool originMatch = true;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (!airSeg)
    return false;

  const Loc& orig = *(airSeg)->origin();

  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  geoMatch = LocUtil::isInLoc(orig,
                              taxCodeReg.loc1Type(),
                              taxCodeReg.loc1(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::TAXES,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              trx.getRequest()->ticketingDT());

  if ((!geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
      (geoMatch && taxCodeReg.loc1ExclInd() == YES) ||
      (!geoMatch && taxCodeReg.loc1ExclInd() != YES))
  {
    originMatch = false;
  }

  // Check for Needed to Zeroes out the Base Fare
  if (((_otherIntl) || (_hawaiianPt) || (_origUS) || (!_origCAMaritime && !_origCANotMaritime)) ||
      (!originMatch) || (taxCodeReg.taxCode() == TAX_CODE_XG3))
  {
    _zeroesBaseFare = true;
  }

  if ((taxCodeReg.taxCode() == TAX_CODE_XG4) && (_otherIntl))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  Apply ONLY originating in Maritime Provinces
// ----------------------------------------------------------------------------
bool
TaxXG::validateXG1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  // Check if XG1 - if not Originating in CA/Maritime, no XG1 applies
  if (!_origCAMaritime)
    return false;

  // Check if XG1 - if there is no US Point, no XG1 applies
  if (!_usPoint)
    return false;

  bool geoMatch = true;
  bool originMatch = true;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (!airSeg)
    return false;

  const Loc& orig = *(airSeg)->origin();

  if (airSeg->equipmentType() == TRAIN)
    return false;

  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  geoMatch = LocUtil::isInLoc(orig,
                              taxCodeReg.loc1Type(),
                              taxCodeReg.loc1(),
                              Vendor::SABRE,
                              MANUAL,
                              LocUtil::TAXES,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              trx.getRequest()->ticketingDT());

  if ((!geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
      (geoMatch && taxCodeReg.loc1ExclInd() == YES) ||
      (!geoMatch && taxCodeReg.loc1ExclInd() != YES))
  {
    originMatch = false;
  }

  // Check for Needed to Zeroes out the Base Fare
  if ((_otherIntl) || (_hawaiianPt) || (_origUS) || (!originMatch) ||
      (!_origCAMaritime && !_origCANotMaritime))
  {
    _zeroesBaseFare = true;
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  ApplyTaxOnTax
// ----------------------------------------------------------------------------
void
TaxXG::applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  _applyFeeOnTax = true;

  _taxSplitDetails.setTotShopRestEnabled(true);

  TaxOnTaxCollector collector(*this);
  const bool taxShoppingRequest = (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath());

  std::vector<std::string>::iterator taxOnTaxCodeI;
  for (taxOnTaxCodeI = taxCodeReg.taxOnTaxCode().begin();
       taxOnTaxCodeI != taxCodeReg.taxOnTaxCode().end();
       taxOnTaxCodeI++)
  {
    collector.collect(taxResponse.taxItemVector(), taxCodeReg.taxCode(), *taxOnTaxCodeI);
    if (taxShoppingRequest)
      collector.collect(taxResponse.farePath()->getExternalTaxes(), taxCodeReg.taxCode(), *taxOnTaxCodeI);
  }

  if (_zeroesBaseFare)
  {
    _taxableFare = 0.0;
    _taxableFareAdjusted = 0.0;

    _taxSplitDetails.setTotShopRestIncludeBaseFare(false);
  }

  if ((taxCodeReg.taxCode() == TAX_CODE_XG) && (_zeroesBaseFare))
  {
    collector.setTaxAmountCA1(0.0);
    _applyFeeOnTax = false;
  }
  else if ((taxCodeReg.taxCode() == TAX_CODE_XG4) && (_zeroesBaseFare))
  {
    collector.setTaxAmountCA1(0.0);
  }

  if (taxCodeReg.taxCode() == TAX_CODE_XG)
  {
    _taxableFareAdjusted += collector.taxAmountCA1() + collector.taxAmountSQ() + collector.taxAmountSQ1();
    _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();

    _taxableFare += collector.taxAmountCA1() + collector.taxAmountSQ() + collector.taxAmountSQ1();
    _taxAmount = _taxableFare * taxCodeReg.taxAmt();
    _mixedTax = true;
  }
  else if (taxCodeReg.taxCode() == TAX_CODE_XG1 ||
      taxCodeReg.taxCode() == TAX_CODE_XG4)
  {
    _taxableFareAdjusted += collector.taxAmountCA1();
    _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();

    _taxableFare += collector.taxAmountCA1();
    _taxAmount = _taxableFare * taxCodeReg.taxAmt();
    _mixedTax = true;
  }
  else if (taxCodeReg.taxCode() == TAX_CODE_XG3)
  {
    _taxableFareAdjusted += collector.taxAmountSQ3();
    _taxAmountAdjusted = _taxableFareAdjusted * taxCodeReg.taxAmt();

    _taxableFare += collector.taxAmountSQ3();
    _taxAmount = _taxableFare * taxCodeReg.taxAmt();
    _mixedTax = true;
  }

  if (_taxAmount == 0.0)
  {
    _failCode = TaxDiagnostic::NO_TAX_CODE;

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_TAX_CODE, Diagnostic820);
  }
}
