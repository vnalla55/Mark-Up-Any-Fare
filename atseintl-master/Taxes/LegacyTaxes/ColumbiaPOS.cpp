// ----------------------------------------------------------------------------
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

#include "Taxes/LegacyTaxes/ColumbiaPOS.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DBAccess/Loc.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

using namespace tse;
using namespace std;

const ColumbiaPOS::CarrierList
ColumbiaPOS::NotChargeUSTaxes("COLUMBIA_POS_CARRIERS_NOT_CHARGE_US");

const ColumbiaPOS::CarrierList
ColumbiaPOS::ChargeAllTaxes("COLUMBIA_POS_CARRIERS_CHARGE_ALL");

const ColumbiaPOS::CarrierList
ColumbiaPOS::ChargeMXTaxes("COLUMBIA_POS_CARRIERS_CHARGE_MX");

const ColumbiaPOS::CarrierList
ColumbiaPOS::ChargeUSTaxes("COLUMBIA_POS_CARRIERS_CHARGE_US");

Logger
ColumbiaPOS::_logger("atseintl.Taxes.ColumbiaPOS");

ColumbiaPOS::CarrierList::CarrierList() {}

ColumbiaPOS::CarrierList::CarrierList(const std::string& paramName)
{
  getCarriers(paramName, _carriers);
}

ColumbiaPOS::CarrierList::~CarrierList() {}

bool
ColumbiaPOS::CarrierList::
operator()(const CarrierCode& s) const
{
  return _carriers.find(s) != _carriers.end();
}

bool
ColumbiaPOS::CarrierList::
operator()(const CarrierCode& s1, const CarrierCode& s2) const
{
  return (_carriers.find(s1) != _carriers.end() && _carriers.find(s2) != _carriers.end());
}

void
ColumbiaPOS::CarrierList::getCarriers(const std::string& paramName,
                                      std::set<std::string>& carriers,
                                      tse::ConfigMan* config)
{
  if (!config)
    return;

  std::string carriersStr;
  if (!config->getValue(paramName, carriersStr, "TAX_SVC"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, paramName, "TAX_SVC");
  }

  if (!carriersStr.empty())
    parseCarriers(carriersStr, carriers);
}

void
ColumbiaPOS::CarrierList::parseCarriers(const std::string& carriersStr,
                                        std::set<std::string>& carriers) const
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator("|");
  tokenizer tokens(carriersStr, separator);
  tokenizer::iterator tokenI;

  for (tokenI = tokens.begin(); tokenI != tokens.end(); ++tokenI)
  {
    carriers.insert(tokenI->data());
  }
}

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

ColumbiaPOS::ColumbiaPOS()
  : _canadianPt(false), _usPoint(false), _mexicanPt(false), _otherIntl(false)
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

ColumbiaPOS::~ColumbiaPOS() {}

// ----------------------------------------------------------------------------
// Description:  ColumbiaPOS
// ----------------------------------------------------------------------------

bool
ColumbiaPOS::chargeUSTaxes(PricingTrx& trx, TaxResponse& taxResponse)
{
  _canadianPt = false;
  _usPoint = false;
  _mexicanPt = false;
  _otherIntl = false;

  const Itin* itin = taxResponse.farePath()->itin();

  if (itin->validatingCarrier().empty())
    return false;

  CarrierCode validatingCarrier = itin->validatingCarrier();

  if (NotChargeUSTaxes(validatingCarrier))
    return false;

  // Now Check for Itinerary type and carrier for each segments
  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  // Check for all domestic Columbia travel, then check all the itin for the Carrier
  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    const Loc& orig = *(airSeg)->origin();
    const Loc& dest = *(airSeg)->destination();

    if (LocUtil::isCanada(orig) || LocUtil::isCanada(dest))
    {
      _canadianPt = true;
    }

    if ((LocUtil::isUS(orig) && !LocUtil::isUSTerritoryOnly(orig)) ||
        (LocUtil::isUS(dest) && !LocUtil::isUSTerritoryOnly(dest)))
    {
      _usPoint = true;
    }

    if (!LocUtil::isUS(orig) && !LocUtil::isCanada(orig) && !LocUtil::isUS(dest) &&
        !LocUtil::isCanada(dest))
    {
      _otherIntl = true;
    }

    if (LocUtil::isUSTerritoryOnly(orig) || LocUtil::isUSTerritoryOnly(dest))
    {
      _otherIntl = true;
    }
  }

  // If all columbia set, then check all itin..
  // Otherwise, check only the 1st carrier of the itin for US/CA
  // And use the 1st carrier of going to international.

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  if (_otherIntl)
  {
    for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      const Loc& orig = *(airSeg)->origin();
      const Loc& dest = *(airSeg)->destination();
      const string marketingCarrierCode = (airSeg)->marketingCarrierCode();

      if (LocUtil::isCanada(orig) || LocUtil::isCanada(dest))
      {
        if (ChargeUSTaxes(marketingCarrierCode, validatingCarrier))
          return true;
      }
      else if (LocUtil::isUS(orig) || LocUtil::isUS(dest))
      {
        if (ChargeUSTaxes(marketingCarrierCode, validatingCarrier))
        {
          return true;
        }
      }

    } // for loop
  }
  else
  {
    if (_canadianPt || (_usPoint && !_otherIntl))
    {
      for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
      {
        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        const string marketingCarrierCode = (airSeg)->marketingCarrierCode();

        if (ChargeUSTaxes(marketingCarrierCode, validatingCarrier))
        {
          return true;
        }
        else
        {
          break; // check only the 1st carrier for wholly US/CA
        }
      }
    }
  }
  // if valid US point then check for TA and LR carrier, else return false
  if (!_usPoint)
    return false;

  return chargeMXTaxes(trx, taxResponse);
}

// ----------------------------------------------------------------------------
// Description:  ColumbiaPOS
// ----------------------------------------------------------------------------

bool
ColumbiaPOS::chargeMXTaxes(PricingTrx& trx, TaxResponse& taxResponse)
{
  _usPoint = false;
  _mexicanPt = false;
  _otherIntl = false;

  const Itin* itin = taxResponse.farePath()->itin();

  if (itin->validatingCarrier().empty())
    return false;

  CarrierCode validatingCarrier = itin->validatingCarrier();

  if (NotChargeUSTaxes(validatingCarrier))
    return false;

  // Now Check for Itinerary type and carrier for each segments
  std::vector<TravelSeg*>::const_iterator travelSegI;

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  // Check if any Mexico Point
  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    const Loc& orig = *(airSeg)->origin();
    const Loc& dest = *(airSeg)->destination();

    if (LocUtil::isMexico(orig) || LocUtil::isMexico(dest))
    {
      _mexicanPt = true;
    }

    if ((LocUtil::isUS(orig) && !LocUtil::isUSTerritoryOnly(orig)) ||
        (LocUtil::isUS(dest) && !LocUtil::isUSTerritoryOnly(dest)))
    {
      _usPoint = true;
    }

    if (!LocUtil::isUS(orig) && !LocUtil::isMexico(orig) && !LocUtil::isUS(dest) &&
        !LocUtil::isMexico(dest))
    {
      _otherIntl = true;
    }

    if (LocUtil::isUSTerritoryOnly(orig) || LocUtil::isUSTerritoryOnly(dest))
    {
      _otherIntl = true;
    }
  }

  // If there is no US and no Mexico involve, normal routine - do not use exception
  if ((!_usPoint) && (!_mexicanPt))
  {
    return false;
  }

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  // If int'l from/to US or from/to MX, check for carrier TA or LR
  if (_otherIntl)
  {
    for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      const Loc& orig = *(airSeg)->origin();
      const Loc& dest = *(airSeg)->destination();
      const string marketingCarrierCode = (airSeg)->marketingCarrierCode();

      if (LocUtil::isMexico(orig) || LocUtil::isMexico(dest))
      {
        if (ChargeMXTaxes(marketingCarrierCode, validatingCarrier))
        {
          return true;
        }
      }
    }
  }
  else
  { // wholly within US and MX
    if (_usPoint || (_mexicanPt && !_otherIntl))
    {
      for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
      {
        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        const string marketingCarrierCode = (airSeg)->marketingCarrierCode();

        if (ChargeMXTaxes(marketingCarrierCode))
        {
          if (ChargeMXTaxes(validatingCarrier))
          {
            return true;
          }
          else
          {
            break; // check only the 1st carrier
          }
        }
      }
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// Description:  ColumbiaPOS
// ----------------------------------------------------------------------------

bool
ColumbiaPOS::chargeAllTaxes(PricingTrx& trx, TaxResponse& taxResponse)
{
  const Itin* itin = taxResponse.farePath()->itin();

  if (itin->validatingCarrier().empty())
    return false;

  CarrierCode validatingCarrier = itin->validatingCarrier();

  if (ChargeAllTaxes(validatingCarrier))
    return true;

  return false;
}

// ColumbiaPOS::ColumbiaPOS
