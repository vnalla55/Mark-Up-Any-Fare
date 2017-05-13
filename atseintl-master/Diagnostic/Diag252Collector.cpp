//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Diagnostic/Diag252Collector.h"

#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/GatewayPair.h"
#include "Common/Money.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/FareInfo.h"

#include <iomanip>

namespace tse
{
void
Diag252Collector::writeAddonFareDiagHeader(ConstructionPoint cp)
{
  if (!_active || !_diagRequest->display252Addons() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()))
    return;

  writeCommonHeader("", true);

  Diag252Collector& dc = *this;

  dc << " \nARBITRARY FARES FOR INTERIOR CITY " << _cJob->od(cp);

  if (_cJob->od(cp) != _cJob->odCity(cp))
    dc << '/' << _cJob->odCity(cp) << "/:\n";
  else
    dc << ":\n";

  dc << SEPARATOR << "V INT   EFFECTIVE  EXPIRE   AMOUNT      TARIFF FARE   ARB  FAIL\n"
     << "N    GTW                                       CLASS  ZONE ED\n" << SEPARATOR;

  _processedFaresCnt = 0;
}

void
Diag252Collector::writeAddonFare(const AddonFareInfo& af, AddonZoneStatus zs)
{
  if (!_active || !_diagRequest->display252Addons() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()))
    return;

  if (!_diagRequest->isValidForDiag(af, (zs != AZ_UNACCEPTABLE)))
    return;

  Diag252Collector& dc = *this;

  setf(std::ios::right, std::ios::adjustfield);

  dc << af.vendor().substr(0, 1) << std::setw(4) << af.interiorMarket() << af.gatewayMarket()
     << ' ';

  formatDateTime(af.effDate());
  formatDateTime(af.expireDate());

  dc.setf(std::ios::fixed, std::ios::floatfield);

  if (!showFareAmount(af.addonTariff(), _cJob->vendorCode(), _cJob->carrier()))
    dc << "       N/A     ";
  else
  {
    if (af.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      dc << std::setw(10) << Money(af.fareAmt() / 2, af.cur()) << " ";
    else
      dc << std::setw(10) << Money(af.fareAmt(), af.cur()) << " ";
  }

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(4) << af.addonTariff() << std::setw(8) << af.fareClass() << std::setw(4)
     << af.arbZone();

  if (zs == AZ_FAIL)
    dc << " ZONE";

  else if (zs == AZ_UNACCEPTABLE)
    dc << " UNACCEPTABLE";

  dc << '\n';

  _processedFaresCnt++;
}

void
Diag252Collector::writeAddonFareDiagFooter()
{
  if (!_active || !_diagRequest->display252Addons() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()))
    return;

  Diag252Collector& dc = *this;
  dc << SEPARATOR;
}

void
Diag252Collector::writeSpecFareDiagHeader(const GatewayPair& gw)
{
  if (!_active || !_diagRequest->display252Specifieds() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()) || !_diagRequest->isValidForDiag(gw))
    return;

  writeCommonHeader(_cJob->vendorCode());

  (*this) << " \nSPECIFIED FARES: ";

  gwQuote(gw);

  (*this) << "    " << _cJob->vendorCode() << '\n' << SEPARATOR
          << "V MK1   EFFECTIVE  EXPIRE   AMOUNT      TARIFF FARE   FAILED\n"
          << "N    MK2                                       CLASS\n" << SEPARATOR;

  _processedGWFaresCnt = 0;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
Diag252Collector::writeSpecifiedFares(const GatewayPair& gw,
                                      const SpecifiedFareList& specFares)
{
  if (!_active || !_diagRequest->display252Specifieds() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()) || !_diagRequest->isValidForDiag(gw))
    return;

  SpecifiedFareList::const_iterator i = specFares.begin();
  SpecifiedFareList::const_iterator ie = specFares.end();

  for (; i != ie; ++i)
    writeSpecifiedFare(gw, *i->_specFare, SP_OK);
}

#else

void
Diag252Collector::writeSpecifiedFares(const GatewayPair& gw,
                                      std::vector<const FareInfo*>& specFares)
{
  if (!_active || !_diagRequest->display252Specifieds() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()) || !_diagRequest->isValidForDiag(gw))
    return;

  std::vector<const FareInfo*>::iterator i = specFares.begin();
  std::vector<const FareInfo*>::iterator ie = specFares.end();

  for (; i != ie; ++i)
    writeSpecifiedFare(gw, **i, SP_OK);
}

#endif

void
Diag252Collector::writeSpecifiedFare(const GatewayPair& gw,
                                     const FareInfo& spFare,
                                     SpecFareStatus spStatus)
{
  if (!_active || !_diagRequest->isValidForDiag(spFare))
    return;

  Diag252Collector& dc = *this;

  setf(std::ios::right, std::ios::adjustfield);

  dc << spFare.vendor().substr(0, 1) << std::setw(4) << spFare.market1() << spFare.market2() << ' ';

  formatDateTime(spFare.effDate());
  formatDateTime(spFare.expireDate());

  dc.setf(std::ios::fixed, std::ios::floatfield);

  if (!showFareAmount(spFare.fareTariff(), _cJob->vendorCode(), _cJob->carrier()))
    dc << "       N/A     ";
  else
    dc << std::setw(10) << Money(spFare.fareAmount(), spFare.currency()) << " ";

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(4) << spFare.fareTariff() << std::setw(8) << spFare.fareClass();

  if (spStatus == CP_SITA_NOT_FOR_CONSTRUCTION)
    dc << " NOT FOR CONSTRUCTION";

  dc << '\n';

  _processedGWFaresCnt++;
  _processedFaresCnt++;
}

void
Diag252Collector::writeSpecFareDiagFooter(const GatewayPair& gw)
{
  if (!_active || !_diagRequest->display252Specifieds() ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()) || !_diagRequest->isValidForDiag(gw))
    return;
  (*this) << SEPARATOR;
}
}
