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

#include "Diagnostic/Diag254Collector.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DateInterval.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/SitaConstructedFare.h"
#include "DBAccess/AddonFareInfo.h"

namespace tse
{
void
Diag254Collector::writeConstructedFare(const ConstructedFare& constructedFare)
{
  if (!_active || !_diagRequest->isValidForDiag(constructedFare) ||
      !_diagRequest->isValidForDiag(_cJob->vendorCode()))
    return;

  if (_fareCount == 0)
    writeHeader();
  else
    (*this) << " \n";

  _fareCount++;

  // origin add-on

  if (constructedFare.origAddon() != nullptr)
  {
    writeAddonFare(*constructedFare.origAddon(), true);

    if (_cJob->showDateIntervalDetails())
      writeDateIntervals(*constructedFare.origAddon(), true, constructedFare);
  }

  // specified fare

  writeSpecifiedFare(*constructedFare.specifiedFare());

  if (_cJob->showDateIntervalDetails())
    writeDateIntervals(constructedFare);

  // destination addon

  if (constructedFare.destAddon() != nullptr)
  {
    writeAddonFare(*constructedFare.destAddon(), false);

    if (_cJob->showDateIntervalDetails())
      writeDateIntervals(*constructedFare.destAddon(), false, constructedFare);
  }

  // constructed fare

  ACDiagCollector::writeConstructedFare(constructedFare);

  if (_cJob->showDateIntervalDetails())
    writeDateInterval(constructedFare.effInterval(), 'C');
}

void
Diag254Collector::writeFooter()
{
  if (_fareCount > 0)
  {
    (*this) << SEPARATOR << std::endl;

    _fareCount = 0;
  }
}

void
Diag254Collector::writeHeader()
{
  writeCommonHeader(_cJob->vendorCode(), true);

  writeFiresHeader();
}

void
Diag254Collector::writeDateIntervals(const AddonFareCortege& afc,
                                     const bool isOriginAddon,
                                     const ConstructedFare& cf)
{
  writeDateInterval(afc.addonFare()->effInterval(), 'A');

  writeDateInterval(
      afc.dateIntervals().addonZoneInterval(), 'Z', afc.dateIntervals().splittedPart());

  if (cf.specifiedFare()->vendor() == ATPCO_VENDOR_CODE)
  {
    const AtpcoFareDateInterval& di = static_cast<const AtpcoConstructedFare&>(cf).dateIntervals();

    if (di.showCombFareClassInterval(isOriginAddon))
      writeDateInterval(di.combFareClassInterval(isOriginAddon), 'M', di.splittedPart());
  }
}

void
Diag254Collector::writeDateIntervals(const ConstructedFare& cf)
{
  writeDateInterval(cf.specifiedFare()->effInterval(), 'S');

  if (cf.specifiedFare()->vendor() == ATPCO_VENDOR_CODE)
  {
    const AtpcoFareDateInterval& di = static_cast<const AtpcoConstructedFare&>(cf).dateIntervals();

    writeDateInterval(di.trfXRefInterval(), 'T', di.splittedPart(), di.trfXRefInterval().inhibit());
  }
}

void
Diag254Collector::writeDateInterval(const TSEDateInterval& interval,
                                    const Indicator description,
                                    const Indicator splittedPart,
                                    const Indicator inhibit)
{
  Diag254Collector& dc = *this;

  dc << description << '.' << splittedPart << (inhibit == INHIBIT_N ? '.' : inhibit) << "..";

  formatDateTime(interval.createDate());

  formatDateTime(interval.effDate());
  formatDateTime(interval.expireDate());

  formatDateTime(interval.discDate());

  dc << "\n";
}
}
