//-------------------------------------------------------------------
//
//  File:        AlternateCarriersTemplate.cpp
//  Authors:     Doug Batchelor
//  Created:     May 10, 2005
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Alternate Carriers that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/AlternateCarriersSection.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/MultiTransportMarkets.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "FareDisplay/FDHeaderMsgController.h"

#include <set>
#include <sstream>
#include <string>

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Template.AlternateCarriersSection");

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  AlternateCarriersSection::buildDisplay()
//
// This is the main processing method of the AlternateCarriersSection class.
// It requires a reference to the Fare Display Transaction. When
// called, it first iterates through the collection of fields,
// and displays the column headers.  It then iterates through each
// of the collected fares and for each one, iterates through the
// collection of fields maintained by the AlternateCarriersSection object.
//
// Example:
// THE FOLLOWING CARRIERS ALSO PUBLISH FARES DFW-LON:
// AC AF AY AZ BA BD CO DL EI FI IB KL LH LO LX NW OK SK SN SQ TW
// UA US VS YY
// ADDITIONAL CARRIERS PARTICIPATE IN YY FARES
//
//
// @param   field   - a reference to a FareDisplayTrx.
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
void
AlternateCarriersSection::buildDisplay()
{
  std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* mktCxrMap = nullptr;
  _trx.preferredCarriers() =
      FareDisplayUtil::getUniqueSetOfCarriers(_trx, isIncludeAddonTable(), mktCxrMap);

  //
  // Format Display of Alternate Carriers
  //

  CarrierCode carrier = _trx.requestedCarrier();
  const LocCode& orig = _trx.boardMultiCity();
  const LocCode& dest = _trx.offMultiCity();
  int16_t altCxrs = 0;
  int16_t lineLength = 63;
  bool YYFaresExist = false;
  bool firstTime = true;

  for (const auto& prefferedCxrCode : _trx.preferredCarriers())
  {
    if (prefferedCxrCode == carrier)
    {
      continue;
    }

    if (prefferedCxrCode == INDUSTRY_CARRIER)
    {
      YYFaresExist = true;
    }

    if (altCxrs < lineLength)
    {
      if (firstTime)
      {
        _trx.response() << FareDisplayUtil::getAltCxrMsg(orig, dest) << std::endl;
        firstTime = false;
      }

      _trx.response() << prefferedCxrCode << " ";
      altCxrs += 3;
    }
    else
    {
      _trx.response() << std::endl;
      _trx.response() << prefferedCxrCode << " ";
      altCxrs = 3;
    }
  }

  _trx.response() << std::endl;

  if (YYFaresExist)
  {
    _trx.response() << FareDisplayUtil::ALT_CXR_YY_MSG << std::endl;
  }
}
bool
AlternateCarriersSection::isIncludeAddonTable() const
{
  std::string configVal("Y");
  if (!(Global::config())
           .getValue("INCLUDE_ADDON_FOR_ALTERNATECARRIERS", configVal, "FAREDISPLAY_SVC"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "INCLUDE_ADDON_FOR_ALTERNATECARRIERS", "FAREDISPLAY_SVC");
  }

  return !(configVal == "N" || configVal == "n" || configVal == "0");
}
} // tse namespace
