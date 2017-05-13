//-------------------------------------------------------------------
//  File:        ACDiagCollector.C
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//  Created:     May 28, 2005
//
//  Description: Add-on construction diag. collector
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
//-------------------------------------------------------------------

#include "Diagnostic/ACDiagCollector.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/FareUtil.h"
#include "AddonConstruction/GatewayPair.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Money.h"
#include "DataModel/Billing.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>

namespace tse
{
const char* ACDiagCollector::SEPARATOR =
    "---------------------------------------------------------------\n";

void
ACDiagCollector::initialize(ConstructionJob* cj, DiagRequest* dr)
{
  _cJob = cj;
  _diagRequest = dr;
  _pricingTrx = &_cJob->trx();
  _travelDate = _cJob->travelDate();

  initialize(*_cJob->loc(CP_ORIGIN), *_cJob->loc(CP_DESTINATION));
}

void
ACDiagCollector::initialize(const Loc& orig, const Loc& dest)
{
  _isJointVenture = DiagnosticUtil::isJointVenture(*_pricingTrx);

  if (DiagnosticUtil::isAirlineAgent(*_pricingTrx))
    _publishingCarrier = _pricingTrx->billing()->partitionID();

  _restrictPrivateFareAmt = !DiagnosticUtil::isDisplayKeywordPresent(*_pricingTrx->getOptions());

  _fcRequested = DiagnosticUtil::isFCRequested(*_pricingTrx);

  bool isInternational =
      LocUtil::isInternational(orig, dest) || LocUtil::isForeignDomestic(orig, dest);

  _crossRefType = isInternational ? INTERNATIONAL : DOMESTIC;
}

void
ACDiagCollector::writeCommonHeader(const VendorCode& vendor, const bool forceHeader)
{
  if (!_active)
    return;

  if (_currentVendor == vendor && !forceHeader)
    return;

  _currentVendor = vendor;

  ACDiagCollector& dc = *this;

  dc << " \nADDON CONSTRUCTION DIAGNOSTIC " << _cJob->trx().diagnostic().diagnosticType()
     << "\nFROM: " << _cJob->od(CP_ORIGIN);

  if (_cJob->od(CP_ORIGIN) != _cJob->odCity(CP_ORIGIN))
    dc << '/' << _cJob->odCity(CP_ORIGIN) << '/';

  dc << "  TO: " << _cJob->od(CP_DESTINATION);

  if (_cJob->od(CP_DESTINATION) != _cJob->odCity(CP_DESTINATION))
    dc << '/' << _cJob->odCity(CP_DESTINATION) << '/';

  dc << "  CARRIER: " << _cJob->carrier();

  if (!vendor.empty())
    dc << "  VENDOR: " << vendor;

  dc << '\n';

  if (_cJob->isHistorical())
    dc << "HISTORICAL: YES" << '\n';
}

void
ACDiagCollector::gwQuote(const GatewayPair& gw)
{
  (*this) << gw.gateway1() << "-" << gw.gateway2();
}

void
ACDiagCollector::writeFiresHeader()
{
  if (!_active)
    return;

  ACDiagCollector& dc = *this;
  dc << SEPARATOR;

  if (_cJob->showDateIntervalDetails())
  {
    dc << "  MK1GW1GW2MK2                   O      AMT/CUR    TAR CLASS"
       << "\n"
       << "       CREATE     EFF     EXP    R DISC\n";
  }
  else
  {
    dc << "  MK1GW1GW2MK2    EFF     EXP    O O    AMT/CUR    TAR CLASS"
       << "\n"
       << "                                 R I\n";
  }

  dc << SEPARATOR;
}

void
ACDiagCollector::writeFiresFooter()
{
  (*this) << SEPARATOR;
}

/////////////////////////////////////////////////////////////////////
// add-on fares

void
ACDiagCollector::writeAddonFare(bool isOriginAddon,
                                const LocCode& interiorCity,
                                const LocCode& gatewayCity,
                                const DateTime& effectiveDate,
                                const DateTime& expireDate,
                                const Indicator owrt,
                                const CurrencyCode& curency,
                                const MoneyAmount amount,
                                const FareClassCode& fareClass,
                                const TariffNumber tariff)
{
  ACDiagCollector& dc = *this;

  setf(std::ios::left, std::ios::adjustfield);

  if (isOriginAddon)
    dc << "A " << interiorCity << gatewayCity << "       ";
  else
    dc << "A       " << gatewayCity << interiorCity << ' ';

  setf(std::ios::right, std::ios::adjustfield);

  if (_cJob->showDateIntervalDetails())
    dc << std::setw(9) << ' ' << std::setw(9) << ' ';
  else
  {
    formatDateTime(effectiveDate);
    formatDateTime(expireDate);
  }

  setf(std::ios::fixed, std::ios::floatfield);

  dc << owrt << "  ";

  if (!showFareAmount(tariff, _cJob->vendorCode(), _cJob->carrier()))
    dc << "       N/A     ";
  else
  {
    dc << std::setw(10);
    if (owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
      dc << Money(amount / 2, curency) << " ";
    else
      dc << Money(amount, curency) << " ";
  }

  setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(4) << tariff << fareClass << '\n';
}

void
ACDiagCollector::writeAddonFare(const AddonFareCortege& afc, bool isOriginAddon)
{
  const AddonFareInfo& af = *afc.addonFare();

  writeAddonFare(isOriginAddon,
                 af.interiorMarket(),
                 af.gatewayMarket(),
                 af.effDate(),
                 af.expireDate(),
                 af.owrt(),
                 af.cur(),
                 af.fareAmt(),
                 af.fareClass(),
                 af.addonTariff());
}

void
ACDiagCollector::writeAddonFare(const AddonFareInfo& af, bool isOriginAddon)
{
  writeAddonFare(isOriginAddon,
                 af.interiorMarket(),
                 af.gatewayMarket(),
                 af.effDate(),
                 af.expireDate(),
                 af.owrt(),
                 af.cur(),
                 af.fareAmt(),
                 af.fareClass(),
                 af.addonTariff());
}

/////////////////////////////////////////////////////////////////////
// specified & constructed fares

void
ACDiagCollector::writeSpecifiedOrConstructed(bool isSpecified,
                                             const Indicator inhibit,
                                             const LocCode& city1,
                                             const LocCode& city2,
                                             const DateTime& effectiveDate,
                                             const DateTime& expireDate,
                                             const Indicator owrt,
                                             const Directionality& directionality,
                                             const CurrencyCode& curency,
                                             const MoneyAmount amount,
                                             const FareClassCode& fareClass,
                                             const TariffNumber tariff,
                                             bool skipFareAmt)
{
  ACDiagCollector& dc = *this;

  setf(std::ios::left, std::ios::adjustfield);

  if (isSpecified)
    dc << "S" << inhibit << "   " << city1 << city2 << "    ";
  else
    dc << "C" << inhibit << city1 << "------" << city2 << " ";

  setf(std::ios::right, std::ios::adjustfield);

  if (_cJob->showDateIntervalDetails())
    dc << std::setw(9) << ' ' << std::setw(9) << ' ';
  else
  {
    formatDateTime(effectiveDate);
    formatDateTime(expireDate);
  }

  dc << owrt << ' ';

  if (directionality == BOTH)
    dc << 'B';
  if (directionality == FROM)
    dc << 'F';
  else
    dc << 'T';

  setf(std::ios::fixed, std::ios::floatfield);
  if (skipFareAmt || !showFareAmount(tariff, _cJob->vendorCode(), _cJob->carrier()))
    dc << "       N/A     ";
  else
    dc << std::setw(10) << Money(amount, curency) << " ";

  setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(4) << tariff << fareClass << '\n';
}

void
ACDiagCollector::writeSpecifiedFare(const FareInfo& fareInfo)
{
  const Indicator inhibit = (fareInfo.inhibit() == INHIBIT_N ? ' ' : fareInfo.inhibit());

  writeSpecifiedOrConstructed(true,
                              inhibit,
                              fareInfo.market1(),
                              fareInfo.market2(),
                              fareInfo.effDate(),
                              fareInfo.expireDate(),
                              fareInfo.owrt(),
                              fareInfo.directionality(),
                              fareInfo.currency(),
                              fareInfo.fareAmount(),
                              fareInfo.fareClass(),
                              fareInfo.fareTariff());
}

void
ACDiagCollector::writeConstructedFare(const ConstructedFare& cf)
{
  MoneyAmount fareAmount = 0;

  Indicator inhibit = ' ';
  if (cf.fareDisplayOnly())
    inhibit = INHIBIT_D;

  else if (cf.pricingOnly())
    inhibit = INHIBIT_P;

  bool skipFareAmt = !FareUtil::calculateTotalAmount(cf, *_cJob, fareAmount);

  writeSpecifiedOrConstructed(false,
                              inhibit,
                              cf.market1(),
                              cf.market2(),
                              cf.effInterval().effDate(),
                              cf.effInterval().expireDate(),
                              cf.specifiedFare()->owrt(),
                              cf.defineDirectionality(),
                              cf.specifiedFare()->currency(),
                              fareAmount,
                              cf.specifiedFare()->fareClass(),
                              cf.specifiedFare()->fareTariff(),
                              skipFareAmt);
}

void
ACDiagCollector::writeConstructedFare(const ConstructedFareInfo& cfi)
{
  const FareInfo& fareInfo = cfi.fareInfo();

  const Indicator inhibit = (fareInfo.inhibit() == INHIBIT_N ? ' ' : fareInfo.inhibit());

  writeSpecifiedOrConstructed(false,
                              inhibit,
                              fareInfo.market1(),
                              fareInfo.market2(),
                              fareInfo.effDate(),
                              fareInfo.expireDate(),
                              fareInfo.owrt(),
                              fareInfo.directionality(),
                              fareInfo.currency(),
                              fareInfo.fareAmount(),
                              fareInfo.fareClass(),
                              fareInfo.fareTariff());
}

void
ACDiagCollector::formatDateTime(const DateTime& dt)
{
  if (dt.isInfinity())
    (*this) << "99/99/99 ";
  else
    (*this) << std::setw(8) << dt.dateToString(MMDDYY, "/") << " ";
}

bool
ACDiagCollector::isPrivateFare(const TariffNumber& tariff,
                               const VendorCode& vendor,
                               const CarrierCode& carrier)
{
  const std::vector<TariffCrossRefInfo*>& tcrList =
      _pricingTrx->dataHandle().getTariffXRefByFareTariff(
          vendor, carrier, _crossRefType, tariff, _travelDate);

  if (!tcrList.empty() && (tcrList.front()->tariffCat() == RuleConst::PRIVATE_TARIFF))
    return true;

  return false;
}

bool
ACDiagCollector::showFareAmount(const TariffNumber& tariff,
                                const VendorCode& vendor,
                                const CarrierCode& fareCarrier)
{
  if (DiagnosticUtil::showFareAmount(
          !_restrictPrivateFareAmt, *_pricingTrx, fareCarrier, _publishingCarrier))
    return true;

  if (!isPrivateFare(tariff, vendor, fareCarrier))
    return true;

  return false;
}
}
