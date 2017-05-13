
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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
#include <algorithm>

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "DBAccess/TaxReissue.h"
#include "DataModel/Agent.h"
#include "Taxes/TaxInfo/TaxInfoBuilderMisc.h"
#include "Taxes/Common/LocRestrictionValidator.h"

using namespace tse;

namespace tse
{
FALLBACK_DECL(taxReissueCat33OnlyInd);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::TaxInfoBuilderMisc
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderMisc::TaxInfoBuilderMisc() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::~TaxInfoBuilderMisc
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
TaxInfoBuilderMisc::~TaxInfoBuilderMisc() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::buildDetails
//
// Description:  Build US1 Info.
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxInfoBuilderMisc::buildDetails(TaxTrx& trx)
{
  const std::vector<TaxReissue*>& reissues = trx.dataHandle().getTaxReissue(taxCode(), ticketingDT());

  auto reissue = fallback::taxReissueCat33OnlyInd(&trx)
                     ? reissues.begin()
                     : std::find_if(reissues.begin(),
                                    reissues.end(),
                                    [](const TaxReissue* reissue)
                                    { return reissue->cat33onlyInd() != 'Y'; });

  if (reissue == reissues.end() || (*reissue)->refundInd() != 'N')
    std::get<Response::TAX::REFUNDABLE>(response().taxAttrValues()) = TaxInfoBuilder::YES;
  else
    std::get<Response::TAX::REFUNDABLE>(response().taxAttrValues()) = TaxInfoBuilder::NO;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::validateTax
//
// Description: Validate tax records.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderMisc::validateTax(TaxTrx& trx)
{
  const std::vector<TaxCodeReg*>& taxCodeReg = getTaxCodeReg(trx);

  if (taxCodeReg.empty())
  {
    std::get<Response::TAX::ERROR>(response().taxAttrValues()) = TAX_NOT_FOUND;
    return false;
  }

  std::vector<TaxCodeReg*>::const_iterator taxCodeRegIt = taxCodeReg.begin();
  std::vector<TaxCodeReg*>::const_iterator taxCodeRegItEnd = taxCodeReg.end();

  for (; taxCodeRegIt != taxCodeRegItEnd; taxCodeRegIt++)
  {
    // Validate Travel dates
    if (!validateTravelDate(trx, *taxCodeRegIt))
      continue;

    // Validate Origin restriction
    if (!validateOriginRestriction(trx, **taxCodeRegIt))
      continue;

    // Validate Point of Sale
    if (!validatePosPoi(trx,
                        (*taxCodeRegIt)->posExclInd(),
                        (*taxCodeRegIt)->posLocType(),
                        (*taxCodeRegIt)->posLoc()))
      continue;

    // Validate Point of Issue
    if (!validatePosPoi(trx,
                        (*taxCodeRegIt)->poiExclInd(),
                        (*taxCodeRegIt)->poiLocType(),
                        (*taxCodeRegIt)->poiLoc()))
      continue;

    _taxCodeReg = *taxCodeRegIt;

    std::get<Response::TAX::CURRENCY>(response().taxAttrValues()) = _taxCodeReg->taxCur();

    if ((*taxCodeRegIt)->taxType() == 'P')
    {
      std::get<Response::TAX::AMOUNT>(response().taxAttrValues()) =
          percentageToString(_taxCodeReg->taxAmt());
    }
    else
    {
      std::get<Response::TAX::AMOUNT>(response().taxAttrValues()) = amtToString(
          _taxCodeReg->taxAmt(), _taxCodeReg->taxCur(), trx.getRequest()->ticketingDT());
    }

    return true;
  }

  std::get<Response::TAX::ERROR>(response().taxAttrValues()) = NO_DATA_ON_FILE;
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::validateOriginRestriction
//
// Description: Validate origin restriction
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderMisc::validateOriginRestriction(TaxTrx& trx, TaxCodeReg& taxCodeReg)
{
  LocTypeCode locType;
  LocCode loc;
  Indicator locExclInd;

  // Check for Origin restriction
  if (taxCodeReg.loc1Appl() == LocRestrictionValidator::TAX_ORIGIN)
  {
    locType = taxCodeReg.loc1Type();
    loc = taxCodeReg.loc1();
    locExclInd = taxCodeReg.loc1ExclInd();
  }
  else if (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_ORIGIN)
  {
    locType = taxCodeReg.loc2Type();
    loc = taxCodeReg.loc2();
    locExclInd = taxCodeReg.loc2ExclInd();
  }
  else
    return true;

  // Origin restriction must be validated only if locType is city/airport or zone

  if (locType == LOCTYPE_CITY || locType == LOCTYPE_AIRPORT)
    return (locExclInd == LocRestrictionValidator::TAX_EXCLUDE
                ? loc != trx.getRequest()->ticketingAgent()->agentCity()
                : loc == trx.getRequest()->ticketingAgent()->agentCity());

  if (locType == LOCTYPE_ZONE)
  {
    if (isInZone(trx, loc))
      return (locExclInd == LocRestrictionValidator::TAX_EXCLUDE ? false : true);
    else
      return (locExclInd == LocRestrictionValidator::TAX_EXCLUDE ? true : false);
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::validatePosPoi
//
// Description: Validate point of sale or point of issue
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderMisc::validatePosPoi(TaxTrx& trx, Indicator& exclInd, LocType& locType, LocCode& loc)
{
  if (loc.empty())
    return true;

  // Point of Sale/Issue restriction must be validated only if locType is nation or zone

  if (locType == LOCTYPE_NATION)
  {
    if (loc == trx.getRequest()->ticketingAgent()->agentLocation()->nation())
      return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? false : true);

    if (!trx.taxInfoRequest()->countryCodes().empty())
    {
      if (std::find(trx.taxInfoRequest()->countryCodes().begin(),
                    trx.taxInfoRequest()->countryCodes().end(),
                    loc) != trx.taxInfoRequest()->countryCodes().end())
        return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? false : true);
      else
        return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? true : false);
    }
    // No match
    return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? true : false);
  }

  if (locType == LOCTYPE_ZONE)
  {
    if (isInZone(trx, loc))
      return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? false : true);
    else
      return (exclInd == LocRestrictionValidator::TAX_EXCLUDE ? true : false);
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::isInZone
//
// Description: Check whether or not a agent location is in a given zone
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderMisc::isInZone(TaxTrx& trx, LocCode& loc)
{
  Loc* agentLocation = const_cast<Loc*>(trx.getRequest()->ticketingAgent()->agentLocation());

  if (LocUtil::isInZone(*agentLocation,
                        Vendor::SABRE,
                        loc,
                        MANUAL,
                        LocUtil::TAXES,
                        GeoTravelType::International,
                        EMPTY_STRING(),
                        ticketingDT()))
  {
    return true;
  }

  if (!trx.taxInfoRequest()->countryCodes().empty())
  {
    // Loop thru all additional country codes
    std::vector<NationCode>::const_iterator countryIt =
        trx.taxInfoRequest()->countryCodes().begin();
    std::vector<NationCode>::const_iterator countryItEnd =
        trx.taxInfoRequest()->countryCodes().end();

    for (; countryIt != countryItEnd; countryIt++)
    {
      agentLocation->nation() = *countryIt;

      if (LocUtil::isInZone(*agentLocation,
                            Vendor::SABRE,
                            loc,
                            MANUAL,
                            LocUtil::TAXES,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDT()))
      {
        return true;
      }
    } // end loop
  }
  // No match
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxInfoBuilderMisc::validateAirport
//
// Description: Validate airport
//
// </PRE>
// ----------------------------------------------------------------------------
bool
TaxInfoBuilderMisc::validateAirport(TaxTrx& trx)
{
  return true;
}
