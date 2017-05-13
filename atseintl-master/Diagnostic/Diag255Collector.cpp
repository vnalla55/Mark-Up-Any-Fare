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

#include "Diagnostic/Diag255Collector.h"

#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/DiagRequest.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "DBAccess/SITAFareInfo.h"

#include <iomanip>
#include <sstream>

namespace tse
{
void
Diag255Collector::writeGWHeader(const ConstructionVendor& vendor, const GatewayPair& gw)
{
  _skipGW = !_active || !_diagRequest->isValidForDiag(vendor.vendor()) ||
            !_diagRequest->isValidForDiag(gw);

  if (_skipGW)
    return;

  writeCommonHeader(vendor.vendor());

  (*this) << " \nADD-ON/SPECIFIED FARE COMBINATIONS FOR: ";

  gwQuote(gw);

  (*this) << '\n';

  writeFiresHeader();
}

void
Diag255Collector::writeFarePair(const AddonFareInfo& af,
                                const FareInfo& sf,
                                const FareMatchCode matchResult)
{
  if (_diagRequest->isValidForDiag(af, sf))
  {
    writeAddonFare(af, true);
    writeSpecifiedFare(sf);

    setf(std::ios::left, std::ios::adjustfield);

    if (matchResult != FM_GOOD_MATCH)
    {
      std::string failCode;

      formatMatchResult(matchResult, &af, &sf, nullptr, failCode);

      (*this) << failCode << "\n \n";
    }
  }
}

void
Diag255Collector::writeBadFare(const AddonFareInfo& origAddon,
                               const FareInfo& specFare,
                               const AddonFareInfo& destAddon,
                               const FareMatchCode matchResult)
{
  if (_diagRequest->isValidForDiag(origAddon, specFare, destAddon))
  {
    writeAddonFare(origAddon, true);
    writeSpecifiedFare(specFare);
    writeAddonFare(destAddon, false);

    setf(std::ios::left, std::ios::adjustfield);

    if (matchResult != FM_GOOD_MATCH)
    {
      std::string failCode;

      formatMatchResult(matchResult, &origAddon, &specFare, &destAddon, failCode);

      (*this) << failCode << "\n \n";
    }
  }
}

void
Diag255Collector::writeGWFooter()
{
  if (!_skipGW)
    (*this) << SEPARATOR;
}

void
Diag255Collector::formatMatchResult(FareMatchCode matchResult,
                                    const AddonFareInfo* origAddon,
                                    const FareInfo* specFare,
                                    const AddonFareInfo* destAddon,
                                    std::string& failCode)
{
  const SITAAddonFareInfo* origSAFI = dynamic_cast<const SITAAddonFareInfo*>(origAddon);

  const SITAFareInfo* ssf = dynamic_cast<const SITAFareInfo*>(specFare);

  const SITAAddonFareInfo* destSAFI = dynamic_cast<const SITAAddonFareInfo*>(destAddon);

  static constexpr int MAX_LINE_LENGTH = 63;

  std::ostringstream failCodeStr;

  switch (matchResult)
  {
  case FM_DATE_INTERVAL_MISMATCH:
    failCodeStr << "ADDON FARE DATES "
                << "CANNOT BE MATCHED WITH SPEC FARE DATES";

    break;

  case FM_COMB_FARE_CLASS:
    failCodeStr << "ADDON FC " << origAddon->fareClass() << "  CANNOT BE MATCHED WITH SPEC FC "
                << specFare->fareClass();

    break;

  case FM_FARE_TARIFF:
    failCodeStr << "ADDON TARIFF " << origAddon->addonTariff()
                << "  CANNOT BE MATCHED WITH SPEC TARIFF " << specFare->fareTariff();

    break;

  case FM_OWRT:
    failCodeStr << "ADDON OWRT " << origAddon->owrt() << "  CANNOT BE MATCH WITH SPECIFIED OWRT "
                << specFare->owrt();

    break;

  case FM_SITA_TRULE:
    failCodeStr << "ORIG.ADDON TRULE " << origSAFI->throughRule()
                << "  IS NOT EQUAL TO DEST.ADDON TRULE " << destSAFI
        ? destSAFI->throughRule()
        : "EMPTY";

    break;

  case FM_SITA_TMPM:
    failCodeStr << "ORIG.ADDON TMPM " << origSAFI->throughMPMInd()
                << "  IS NOT EQUAL TO DEST.ADDON TMPM " << destSAFI
        ? destSAFI->throughMPMInd()
        : ' ';

    break;

  case FM_SITA_TRTG:
    failCodeStr << "ORIG.ADDON TRTG " << origSAFI->thruFareRouting()
                << "  IS NOT EQUAL TO DEST.ADDON TRTG " << destSAFI
        ? destSAFI->thruFareRouting()
        : "EMPTY";

    break;

  case FM_SITA_BMPM_CANT_WITH_MILEAGE:
    failCodeStr << "BMPM MATCH  BMPM " << origSAFI->baseMPMInd() << "  BUT SPECIFIED IS MILEAGE";

    break;

  case FM_SITA_BRTG:
    failCodeStr << "BRTG MATCH  ARTG " << origSAFI->baseFareRouting()
                << "  SRTG=" << ssf->routingNumber();

    break;

  case FM_SITA_BMPM_CANT_WITH_ROUTING:
    failCodeStr << "BMPM MATCH  BMPM " << origSAFI->baseMPMInd() << "  BUT SPECIFIED HAS ROUTING";

    break;

  case FM_SITA_BMPM_UNKNOWN:
    failCodeStr << "UNKNOWN MPM FLAG CODE " << origSAFI->baseMPMInd();

    break;

  case FM_SITA_ROUTE_CODE:
    failCodeStr << "ROUTE CODE MATCH ARC " << origSAFI->routeCode() << "  SRC " << ssf->routeCode();

    break;

  case FM_SITA_TARIFF_FAMILY:
    failCodeStr << "TAR.FAM.MATCH  ATF " << origSAFI->tariffFamily() << "  STF "
                << ssf->tariffFamily();

    break;

  case FM_SITA_FARE_QUALITY_INCLUDE:
    failCodeStr << "F.QUALITY INCLUDE MATCH  SFQ ";

    break;

  case FM_SITA_FARE_QUALITY_EXCLUDE:
    failCodeStr << "F.QUALITY EXCLUDE MATCH  SFQ ";

    break;

  case FM_SITA_FARE_QUALITY_UNKNOWN:
    failCodeStr << "UNKNOWN AFQIND " << origSAFI->fareQualInd() << "  SFQ ";

    break;

  case FM_SITA_RTAR:
    failCodeStr << "ADDON RTAR " << origSAFI->ruleTariff() << " CANNOT BE MATCHED WITH SPEC TARIFF "
                << specFare->fareTariff();

    break;

  case FM_SITA_ARULE_INCLUDE:
    failCodeStr << "ARULE INCLUDE MATCH  SRULE ";

    break;

  case FM_SITA_ARULE_EXCLUDE:
    failCodeStr << "ARULE EXCLUDE MATCH  SRULE ";

    break;

  case FM_SITA_ARULE_UNKNOWN:
    failCodeStr << "UNKNOWN ARULE EXCLUDE IND " << origSAFI->ruleExcludeInd() << "  SRULE ";

    break;

  case FM_SITA_CLFB_FBC:
    failCodeStr << "FARE CLASS MATCH  AFC " << origSAFI->fareClass() << "  SFC "
                << ssf->fareClass();

    break;

  case FM_SITA_CLFB_UNKNOWN:
    failCodeStr << "UNKNOWN CLFB CODE " << origSAFI->classFareBasisInd();

    break;

  case FM_SITA_GLOBAL_DBE:
    failCodeStr << "GLOBAL-DBE CLASS MATCH  SF DBE CLASS " << ssf->dbeClass();

    break;

  case FM_SITA_GLOBAL:
    failCodeStr << "DBE CLASS MATCH  SF DBE CLASS " << ssf->dbeClass();

    break;

  case FM_SITA_GLOBAL_UNKNOWN:
    failCodeStr << "GLOBAL CLASS MATCH UNKNOWN GLOBAL CLASS FLAG " << origSAFI->globalClassFlag();

    break;

  case FM_ADDON_DIRECTION:
    failCodeStr << "ADDON DIRECTIONALITY MATCH " << origAddon->directionality();

    break;

  default:
    failCodeStr << "UNKNOWN MATCH ERROR";
  }
  failCode = failCodeStr.str().substr(0, MAX_LINE_LENGTH);
}
}
