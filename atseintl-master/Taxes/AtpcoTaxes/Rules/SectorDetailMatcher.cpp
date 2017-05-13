// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Services/SectorDetail.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/PaymentDetail.h"
#include "Rules/SectorDetailMatcher.h"

#include <algorithm>
#include <string>

#include <stdint.h>

namespace tax
{
const std::set<std::string>
SectorDetailMatcher::VALID_TICKET_CODES{ "AD", "CD", "DF", "DG", "DP", "DT", "EM" };

bool areBothDigits(const char c1, const char c2)
{
  return isdigit(c1) && isdigit(c2);
}

bool
SectorDetailMatcher::matchEquipmentCode(const SectorDetailEntry& entry, const type::Index& flightId)
    const
{
  type::EquipmentCode equipment = _flights.at(_flightUsages.at(flightId).flightRefId()).equipment();
  return (equipment == entry.equipmentCode || entry.equipmentCode.empty());
}

bool
SectorDetailMatcher::matchCarrierCode(const SectorDetailEntry& entry, const type::Index& flightId)
    const
{
  type::CarrierCode marketingCarrier =
      _flights.at(_flightUsages.at(flightId).flightRefId()).marketingCarrier();
  return (marketingCarrier == entry.fareOwnerCarrier || entry.fareOwnerCarrier.empty());
}

type::Index
SectorDetailMatcher::getFareIndexForFlight(const type::Index& flightId) const
{
  long maxFlightIdxGivenMappingIdx = -1;
  type::Index mapping_fareUsage_It = 0;
  while (maxFlightIdxGivenMappingIdx < (long)flightId)
  {
    maxFlightIdxGivenMappingIdx +=
        _geoPathMapping.mappings()[mapping_fareUsage_It].maps().size() / 2;
    if (maxFlightIdxGivenMappingIdx < (long)flightId)
      mapping_fareUsage_It++;
  }
  return _fareUsages[mapping_fareUsage_It].index();
}

bool
SectorDetailMatcher::matchFareTypeCode(const SectorDetailEntry& entry, const type::Index& fareIdx)
    const
{
  if (entry.fareType.empty())
    return true;

  type::FareTypeCode flightFareTypeCode = _fares[fareIdx].type();
  return (flightFareTypeCode == entry.fareType);
}

bool
SectorDetailMatcher::matchFareRuleCode(const SectorDetailEntry& entry, const type::Index& fareIdx)
    const
{
  if (entry.rule.empty())
    return true;

  type::FareRuleCode flightFareRuleCode = _fares[fareIdx].rule();
  return (flightFareRuleCode == entry.rule);
}

bool
SectorDetailMatcher::matchCabinCode(const SectorDetailEntry& entry, const type::Index& flightId)
    const
{
  type::CabinCode cabinCode = _flights.at(_flightUsages.at(flightId).flightRefId()).cabinCode();
  return cabinCode == entry.cabinCode || entry.cabinCode == type::CabinCode::Blank;
}

bool
SectorDetailMatcher::matchReservationDesignator(const SectorDetailEntry& entry,
                                                const type::Index& flightId) const
{
  if (entry.reservationCodes[0] == " " && entry.reservationCodes[1] == " " &&
      entry.reservationCodes[2] == " ")
    return true;

  type::ReservationDesignatorCode reservationDesignatorCode =
      _flights.at(flightId).reservationDesignatorCode();

  if (reservationDesignatorCode.empty() || reservationDesignatorCode == " ")
    return false;

  return (reservationDesignatorCode == entry.reservationCodes[0] ||
          reservationDesignatorCode == entry.reservationCodes[1] ||
          reservationDesignatorCode == entry.reservationCodes[2]);
}

bool
SectorDetailMatcher::matchFareTariff(const SectorDetailEntry& entry, const type::Index& fareIdx)
    const
{
  Tariff tariff = _fares[fareIdx].tariff();
  return matches(entry.tariff, tariff);
}

std::string
SectorDetailMatcher::preparePattern(const type::FareBasisCode& fareBasisCode) const
{
  const std::size_t dashPos = fareBasisCode.find('-');
  if (dashPos == std::string::npos)
    return fareBasisCode;

  const std::size_t slashPos = fareBasisCode.find('/');
  if (slashPos == std::string::npos)
  {
    return fareBasisCode.back() == '-' ? fareBasisCode : fareBasisCode + '-';
  }

  if (dashPos + 1 == slashPos)
    return fareBasisCode;
  else if (dashPos + 1 < slashPos)
  {
    std::stringstream fbc1;
    fbc1 << fareBasisCode.substr(0, slashPos) << '-' << fareBasisCode.substr(slashPos);
    return fbc1.str();
  }

  return fareBasisCode;
}

bool
SectorDetailMatcher::matchFareBasisPart(const type::FareBasisCode& pattern,
                                        const type::FareBasisCode& fareBasis) const
{
  assert(!pattern.empty());
  assert(!fareBasis.empty());

  if (pattern == "-")
    return true;

  const std::size_t fareBasisLen = fareBasis.length();
  const std::size_t dashPos = pattern.find('-');

  // no wildcard, do exact match
  if (dashPos == std::string::npos)
    return pattern == fareBasis;

  const std::size_t dashPos2 = pattern.find('-', dashPos + 1);

  if (dashPos == 0) // -PATTERN-
  {
    assert(dashPos2 != std::string::npos);
    std::string matchStr = pattern.substr(1, dashPos2 - 1);
    std::size_t matchStrLen = dashPos2 - 1;

    // First hyphen must match at least one character
    std::size_t matchPos = fareBasis.find(matchStr);
    if (matchPos == 0 || matchPos == std::string::npos)
      return false;

    // The hyphen will not be used to replace a number when the immediately following or preceding
    // character in the Fare Basis Code on the ticket is also a number
    if (areBothDigits(matchStr.front(), fareBasis[matchPos - 1]))
      return false;

    if ((fareBasisLen > matchPos + matchStrLen) &&
        areBothDigits(matchStr.back(), fareBasis[matchPos + matchStrLen]))
      return false;

    return true;
  }
  else // dashPos > 0 : PAT-TERN- or PATTERN-
  {
    std::string matchStr = pattern.substr(0, dashPos);
    std::size_t matchStrLen = matchStr.length();

    // First part of pattern must match to start of Fare Basis Code
    std::size_t matchPos = fareBasis.find(matchStr);
    if (matchPos != 0)
      return false;

    if (fareBasisLen > matchStrLen &&
        areBothDigits(matchStr.back(), fareBasis[matchStrLen]))
      return false;

    if (dashPos2 == std::string::npos) // PATTERN-
      return true;

    // PAT-TERN-, match 2nd part
    std::string matchStr2 = pattern.substr(dashPos + 1, dashPos2 - dashPos -1);
    std::size_t matchStr2Len = matchStr2.length();
    std::size_t matchPos2 = fareBasis.find(matchStr2, matchStrLen);

    if (matchPos2 == std::string::npos)
      return false;

    // The hyphen will not be used to replace a number when the immediately following or preceding
    // character in the Fare Basis Code on the ticket is also a number
    if (matchPos2 > matchStrLen &&
        areBothDigits(matchStr2.front(), fareBasis[matchPos2 - 1]))
      return false;

    if ((fareBasisLen > matchPos2 + matchStr2Len) &&
        areBothDigits(matchStr2.back(), fareBasis[matchPos2 + matchStr2Len]))
      return false;

    return true;
  }
}

bool
SectorDetailMatcher::matchTicketDesignatorPart(const type::FareBasisCode& pattern,
                                               const type::FareBasisCode& ticketDesignator) const
{
  assert(!pattern.empty());
  assert(!ticketDesignator.empty());

  if (pattern == "/*")
    return true;

  if (pattern == "//*")
    return ticketDesignator.find("//") == 0;

  std::size_t asteriskPos = pattern.find('*');
  if (asteriskPos == std::string::npos)
    return pattern == ticketDesignator;

  std::vector<std::string> patternParts;
  std::size_t start = 0;
  while (asteriskPos != std::string::npos)
  {
    patternParts.push_back(pattern.substr(start, asteriskPos - start));
    patternParts.push_back("*");
    start = asteriskPos + 1;
    asteriskPos = pattern.find('*', start);
  }
  if (pattern.length() > start)
    patternParts.push_back(pattern.substr(start));

  start = 0;
  std::size_t i = 0;
  do
  {
    if (patternParts[i] != "*")
    {
      std::size_t matchPos = ticketDesignator.find(patternParts[i], start);
      if (matchPos == std::string::npos)
        return false;

      if (i == 0 && matchPos > 0)
        return false;

      start += patternParts[i].length();
      ++i;
    }
    else // wildcard
    {
      if (i + 1 == patternParts.size()) // wildcard at end of pattern
      {
        if (ticketDesignator.length() == start) // no chars match to *
        {
          return ticketDesignator[start - 1] != '/';
        }
        else if (ticketDesignator.length() > start)
        {
          return !areBothDigits(ticketDesignator[start - 1], ticketDesignator[start]);
        }
        else
          return false;
      }
      else // wildcard in middle of pattern
      {
        ++i; // move to non-wildcard
        std::size_t matchPos = ticketDesignator.find(patternParts[i], start);
        if (matchPos == std::string::npos)
          return false;

        if (matchPos == start) // nothing matched to wildcard
        {
          if (ticketDesignator[start - 1] == '/')
            return false;
        }
        else // something matched to wildcard
        {
          if (areBothDigits(ticketDesignator[start - 1], ticketDesignator[start]) ||
              areBothDigits(ticketDesignator[matchPos - 1], ticketDesignator[matchPos]))
            return false;

          start = matchPos + patternParts[i].length();
        }
      }
      ++i;
    }
  } while (i < patternParts.size());

  return true;
}

bool
SectorDetailMatcher::checkFareBasisTD(const type::FareBasisCode& entryPattern,
                                      const type::FareBasisCode& fareBasisCode) const
{
  std::string pattern = preparePattern(entryPattern);
  // Handle special cases first to simplify further code
  // -//* matches all Fare Basis Codes with '//' (slashes must be adjacent).
  if (pattern == "-//*")
    return fareBasisCode.find("//") != std::string::npos;

  // -/* matches all Fare Basis Codes with one or more ticket designators.
  if (pattern == "-/*")
    return fareBasisCode.find('/') != std::string::npos;

  std::size_t slashPos = pattern.find('/');
  std::size_t hyphenPos = pattern.find('-');
  std::size_t fareBasisSlashPos = fareBasisCode.find('/');

  // When a hyphen is present with no slash, processing will only match Fare Basis Codes
  // that have no ticket designator.
  if (slashPos == std::string::npos)
  {
    if (hyphenPos != std::string::npos && fareBasisSlashPos != std::string::npos)
      return false;

    if (fareBasisSlashPos == std::string::npos)
      return matchFareBasisPart(pattern, fareBasisCode);

    // Fare basis has ticket designator, pattern doesn't have one - accept all TDs
    std::string fareBasisPart = fareBasisCode.substr(0, fareBasisSlashPos);
    return matchFareBasisPart(pattern, fareBasisPart);
  }

  // Pattern has ticket designator, don't match if fareBasisCode without designator
  if (fareBasisSlashPos == std::string::npos)
    return false;

  std::string fareBasisPattern = pattern.substr(0, slashPos);
  std::string fareBasisPart = fareBasisCode.substr(0, fareBasisSlashPos);
  std::string ticketDesignatorPattern = pattern.substr(slashPos);
  std::string ticketDesignatorPart = fareBasisCode.substr(fareBasisSlashPos);

  // When a single slash is present, the slash and any alphanumerics following the slash match
  // against the last ticket designator in the Fare Basis Code
  if (std::count(ticketDesignatorPattern.begin(), ticketDesignatorPattern.end(), '/') == 1 &&
      ticketDesignatorPart.find("//") != std::string::npos)
    ticketDesignatorPart = ticketDesignatorPart.substr(1);

  return matchFareBasisPart(fareBasisPattern, fareBasisPart) &&
         matchTicketDesignatorPart(ticketDesignatorPattern, ticketDesignatorPart);
}

bool
SectorDetailMatcher::matchFareBasisTicketDesignator(const SectorDetailEntry& entry,
                                                    const type::Index& fareIdx) const
{
  if (entry.fareBasisTktDesignator.empty())
    return true;

  type::FareBasisCode fareBasisCode = _fares[fareIdx].basis();

  if (fareBasisCode.empty())
    return false;

  // sanity check - edits should prevent such values
  if (entry.fareBasisTktDesignator.front() == '/' ||
      entry.fareBasisTktDesignator.back()  == '/')
    return false;

  return checkFareBasisTD(entry.fareBasisTktDesignator, fareBasisCode);
}

bool
SectorDetailMatcher::checkTicketCode(const type::TicketCode ticketCode,
                                     const type::FareBasisCode fareBasisCode) const
{
  const char* temp = fareBasisCode.c_str();
  const char* patern = ticketCode.c_str();
  temp++;

  while (*temp != '/' && *(temp + 1) != '/' && *(temp + 1) != '\0')
  {
    if (*temp == *patern && *(temp + 1) == *(patern + 1))
      return true;
    temp++;
  }
  return false;
}

bool
SectorDetailMatcher::isValidTicketCode(const type::TicketCode ticketCode) const
{
  return VALID_TICKET_CODES.find(ticketCode) != VALID_TICKET_CODES.end();
}

bool
SectorDetailMatcher::matchTicketCode(const SectorDetailEntry& entry, const type::Index& fareIdx)
    const
{
  if (entry.ticketCode.empty())
    return true;

  type::FareBasisCode fareBasisCode = _fares[fareIdx].basis();

  if (!isValidTicketCode(entry.ticketCode))
    return false;

  return checkTicketCode(entry.ticketCode, fareBasisCode);
}

bool
SectorDetailMatcher::matchSectorDetails(const SectorDetailEntry& entry, const type::Index& flightId)
    const
{
  type::Index fareIdx = getFareIndexForFlight(flightId);
  return matchSectorDetails(entry, flightId, fareIdx);
}

bool
SectorDetailMatcher::matchSectorDetails(const SectorDetailEntry& entry,
                                        const type::Index& flightId,
                                        const type::Index& fareIdx) const
{
  return (matchEquipmentCode(entry, flightId) && matchCarrierCode(entry, flightId) &&
          matchCabinCode(entry, flightId) && matchFareTypeCode(entry, fareIdx) &&
          matchFareRuleCode(entry, fareIdx) && matchReservationDesignator(entry, flightId) &&
          matchFareTariff(entry, fareIdx) && matchFareBasisTicketDesignator(entry, fareIdx) &&
          matchTicketCode(entry, fareIdx));
}

} // namespace tax
