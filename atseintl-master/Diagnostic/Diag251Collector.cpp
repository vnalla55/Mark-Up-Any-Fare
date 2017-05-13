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

#include "Diagnostic/Diag251Collector.h"

#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/GatewayPair.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <iomanip>

namespace tse
{
void
Diag251Collector::writeArbZoneHeader(const LocCode& location)
{
  if (!_active)
    return;

  writeCommonHeader(_cJob->vendorCode());

  (*this) << " \nARBITRARY ZONES FOR  " << location << '\n' << SEPARATOR
          << "    ZONE  TARIFF  EFFECTIVE  DISCONTINUE  EXPIRATION  \n" << SEPARATOR;
}

void
Diag251Collector::writeArbZoneFooter()
{
  if (!_active)
    return;

  if (_zoneCount != 0)
    writeFiresFooter();

  _zoneCount = 0;
}

void
Diag251Collector::writeArbZone(const LocCode& location,
                               const AddonZone zone,
                               const TariffNumber tariff,
                               const TSEDateInterval& effInterval)
{
  if (!_active || !_diagRequest->isValidForDiag(_cJob->vendorCode()))
    return;

  if (_zoneCount == 0)
    writeArbZoneHeader(location);

  (*this) << std::setw(7) << zone << std::setw(8) << tariff << "   ";

  formatDateTime(effInterval.effDate());
  (*this) << "  ";

  formatDateTime(effInterval.discDate());
  (*this) << "    ";

  formatDateTime(effInterval.expireDate());

  (*this) << '\n';

  _zoneCount++;
}

void
Diag251Collector::writeVendor(const ConstructionVendor& vendor)
{
  if (!_active || !_diagRequest->isValidForDiag(vendor.vendor()))
    return;

  writeCommonHeader(vendor.vendor());

  Diag251Collector& dc = *this;

  dc << " \nGATEWAYS:\n" << SEPARATOR << "         ORIGIN       :       DESTINATION\n"
     << "     SINGLE-  DOUBLE- :     SINGLE-  DOUBLE-\n"
     << "     ENDED    ENDED   :     ENDED    ENDED\n" << SEPARATOR;

  writeGateways(vendor);

  _gwPairPerRow = 0;
  _gwPairCount = 0;

  CacheGatewayPairVec::const_iterator i = vendor.gateways().begin();

  for (; i != vendor.gateways().end(); ++i)
  {
    std::shared_ptr<GatewayPair> gw = *i;

    const LocCode& gateway1 = gw->gateway1();
    const LocCode& gateway2 = gw->gateway2();

    writeGWPair(gateway1, gateway2);
  }

  writeTotalGWPair();

  dc << SEPARATOR;
}

void
Diag251Collector::writeGateways(const ConstructionVendor& vndr)
{
  GwDescriptorVec origGW;
  GwDescriptorVec destGW;

  ConstructionVendor& vendor = const_cast<ConstructionVendor&>(vndr);

  bool okToAdd;
  GwDescriptorVec::iterator ogw, dgw;

  CacheGatewayPairVec::const_iterator gwPair = vendor.gateways().begin();

  for (; gwPair != vendor.gateways().end(); gwPair++)
  {
    if ((*gwPair)->isGw1ConstructPoint())
    {
      okToAdd = true;
      for (ogw = origGW.begin(); ogw != origGW.end(); ogw++)
      {
        if ((*ogw)._gateway == (*gwPair)->gateway1())
        {
          (*ogw)._isDEGateway |= (*gwPair)->isGw2ConstructPoint();
          okToAdd = false;
          break;
        }
      }

      if (okToAdd)
      {
        origGW.push_back(GwDescriptor((*gwPair)->gateway1(), (*gwPair)->isGw2ConstructPoint()));
      }
    }

    if ((*gwPair)->isGw2ConstructPoint())
    {
      okToAdd = true;
      for (dgw = destGW.begin(); dgw != destGW.end(); dgw++)
      {
        if ((*dgw)._gateway == (*gwPair)->gateway2())
        {
          (*dgw)._isDEGateway |= (*gwPair)->isGw1ConstructPoint();
          okToAdd = false;
          break;
        }
      }

      if (okToAdd)
      {
        destGW.push_back(GwDescriptor((*gwPair)->gateway2(), (*gwPair)->isGw1ConstructPoint()));
      }
    }
  }

  std::string s;

  ogw = origGW.begin();
  dgw = destGW.begin();
  for (; (ogw != origGW.end()) || (dgw != destGW.end());)
  {
    if (ogw != origGW.end())
    {
      s = (*ogw)._gateway;
      s += "    X          ";

      if ((*ogw)._isDEGateway)
        s += "X   :";
      else
        s += "    :";

      ogw++;
    }
    else
      s = "                      :";

    if (dgw != destGW.end())
    {
      s += " ";
      s += (*dgw)._gateway;
      s += "   X";

      if ((*dgw)._isDEGateway)
        s += "          X";

      dgw++;
    }

    ((DiagCollector&)*this) << s << "\n";
  }
}

void
Diag251Collector::writeGWPair(const LocCode& gw1, const LocCode& gw2)
{
  Diag251Collector& dc = *this;

  if (_gwPairCount == 0)
    dc << "SPECIFIED FAREMARKETS:\n    ";

  if (_gwPairPerRow > 0)
    dc << ", ";

  if (_gwPairPerRow > 5)
  {
    dc << "\n    ";

    _gwPairPerRow = 0;
  }

  dc << gw1 << "-" << gw2;

  _gwPairCount++;
  _gwPairPerRow++;
}

void
Diag251Collector::writeTotalGWPair()
{
  Diag251Collector& dc = *this;

  dc << "\nTOTAL GATEWAY PAIRS: " << _gwPairCount << "\n \n ";
}
}
