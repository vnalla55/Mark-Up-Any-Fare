//-------------------------------------------------------------------
//
//  File:        AddonFareCortege.cpp
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Utility class to keep status of add-on fare
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

#include "AddonConstruction/AddonFareCortege.h"

#include "AddonConstruction/ConstructionJob.h"
#include "Common/TseConsts.h"
#include "DBAccess/AddonFareInfo.h"

using namespace tse;

void
AddonFareCortege::initialize(ConstructionJob& cj,
                             const AddonFareInfo* newAddonFare,
                             const TSEDateInterval& zoneInterval,
                             const Indicator splittedPart)
{
  static const char GENERIC_FIVE_STAR[] = "*****";
  static const char GENERIC_SIX_STAR[] = "******";

  _addonFare = newAddonFare;
  _fareDisplayOnly = (newAddonFare->inhibit() == INHIBIT_D);

  effInterval() = newAddonFare->effInterval();

  _dateIntervals.splittedPart() = splittedPart;
  _dateIntervals.setAddonZoneInterval(zoneInterval);

  // trap for debuging. please do not remove ======================>
  //
  // if( newAddonFare->fareClass() == "C*****" &&
  //    newAddonFare->gatewayMarket() == "NYC" &&
  //    newAddonFare->interiorMarket() == "SBA"
  //  )
  //  _gatewayFareCount = gatewayFareCount;
  //
  // >================================================== end of trap

  if (cj.isAtpco() || cj.isSMF())
  {
    // ATPCO add-on fare generic class check
    // ===== ====== ==== ======= ===== =====

    // first try Six Asterisk (******) fare class

    if (_addonFare->fareClass().find(GENERIC_SIX_STAR) == 0)
      _addonFareClass = SIX_STAR;

    // second try Alpha Five Asterisk (A*****) fare class

    else if (_addonFare->fareClass().find(GENERIC_FIVE_STAR) == 1)
      _addonFareClass = ALPHA_FIVE_STAR;

    else
      _addonFareClass = REGULAR;

    // ATPCO add-on fare footnote check
    // ===== ====== ==== ======== =====

    parseFootnotes();
  }
}

void
AddonFareCortege::parseFootnotes()
{
  static const char CHR_FOOTNOTE_TO = 'T';
  static const char CHR_FOOTNOTE_FROM = 'F';

  if (!_addonFare->footNote1().empty())
  {
    // lint -e{530}
    if (_addonFare->footNote1().find(CHR_FOOTNOTE_TO) != std::string::npos)
      _addonFootNote = FOOTNOTE_TO;

    else if (_addonFare->footNote1().find(CHR_FOOTNOTE_FROM) != std::string::npos)
      _addonFootNote = FOOTNOTE_FROM;

    if (_addonFootNote != DIRECTIONAL_FOOTNOTE_NOT_FOUND)
      return;
  }

  if (!_addonFare->footNote2().empty())
  {
    if (_addonFare->footNote2().find(CHR_FOOTNOTE_TO) != std::string::npos)
      _addonFootNote = FOOTNOTE_TO;

    else if (_addonFare->footNote2().find(CHR_FOOTNOTE_FROM) != std::string::npos)
      _addonFootNote = FOOTNOTE_FROM;
  }
}
