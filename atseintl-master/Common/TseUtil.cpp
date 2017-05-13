//----------------------------------------------------------------------------
//
//  File:           TseUtil.cpp
//  Created:        4/7/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "Common/TseUtil.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <sstream>

#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

namespace tse
{

void
TseUtil::alert(const char* msg)
{
  openlog("TseServer", LOG_PID, LOG_USER);
  syslog(LOG_CRIT, "CVGALERT: %s", msg);
  closelog();
}

//----------------------------------------------------------------------------
// getTravelDate()
//----------------------------------------------------------------------------
DateTime
TseUtil::getTravelDate(const std::vector<TravelSeg*>& travelSegVec)

{
  std::vector<TravelSeg*>::const_iterator it = travelSegVec.begin();
  std::vector<TravelSeg*>::const_iterator end = travelSegVec.end();

  DateTime travelDate = (**it).departureDT();

  if (travelDate.isInfinity() || travelDate.isEmptyDate())
  {
    ++it;
    while (it != end)
    {
      travelDate = (**it).departureDT();
      if (!travelDate.isInfinity() && !travelDate.isEmptyDate())
      {
        break;
      }
      ++it;
    }
  }
  return travelDate;
}

//----------------------------------------------------------------------------
// getBookingDate()
//----------------------------------------------------------------------------
DateTime
TseUtil::getBookingDate(const std::vector<TravelSeg*>& travelSegVec)

{
  std::vector<TravelSeg*>::const_iterator it = travelSegVec.begin();
  std::vector<TravelSeg*>::const_iterator end = travelSegVec.end();

  DateTime travelDate = (**it).bookingDT();

  if (UNLIKELY(travelDate.isInfinity()))
  {
    ++it;
    while (it != end)
    {
      travelDate = (**it).bookingDT();
      if (!travelDate.isInfinity())
      {
        break;
      }
      ++it;
    }
  }
  return travelDate;
}

namespace
{

double
latitudeHemisphere(char c)
{
  switch (toupper(c))
  {
  case 'N':
    return 1.0;
  case 'S':
    return -1.0;
  default:
    return 0.0;
  }
}

double
longitudeHemisphere(char c)
{
  switch (toupper(c))
  {
  case 'E':
    return 1.0;
  case 'W':
    return -1.0;
  default:
    return 0.0;
  }
}
}

//----------------------------------------------------------------------------
// greatCircleMiles()
//----------------------------------------------------------------------------
uint32_t
TseUtil::greatCircleMiles(const Loc& p1, const Loc& p2)
{
  if (p1.loc() == p2.loc())
    return 0;

  if (p1.latdeg() == p2.latdeg() &&
      p1.lngdeg() == p2.lngdeg() &&
      p1.latmin() == p2.latmin() &&
      p1.lngmin() == p2.lngmin() &&
      p1.latsec() == p2.latsec() &&
      p1.lngsec() == p2.lngsec() &&
      p1.lathem() == p2.lathem() &&
      p1.lnghem() == p2.lnghem())
    return 0;

  const double de2ra = 0.01745329252;
  const double flattening = 1.000000 / 298.257223563;
  const double erad = 6378.137;
  const double km2mi = 0.621371;

  double lat1 = latitudeHemisphere(p1.lathem()) *
                (p1.latdeg() + p1.latmin() / 60.0 + p1.latsec() / 3600.0) * de2ra;
  double lon1 = -longitudeHemisphere(p1.lnghem()) *
                (p1.lngdeg() + p1.lngmin() / 60.0 + p1.lngsec() / 3600.0) * de2ra;
  double lat2 = latitudeHemisphere(p2.lathem()) *
                (p2.latdeg() + p2.latmin() / 60.0 + p2.latsec() / 3600.0) * de2ra;
  double lon2 = -longitudeHemisphere(p2.lnghem()) *
                (p2.lngdeg() + p2.lngmin() / 60.0 + p2.lngsec() / 3600.0) * de2ra;

  double F = (lat1 + lat2) / 2.0;
  double G = (lat1 - lat2) / 2.0;
  double L = (lon1 - lon2) / 2.0;

  double sing = sin(G);
  double cosl = cos(L);
  double cosf = cos(F);
  double sinl = sin(L);
  double sinf = sin(F);
  double cosg = cos(G);

  double S = sing * sing * cosl * cosl + cosf * cosf * sinl * sinl;
  double C = cosg * cosg * cosl * cosl + sinf * sinf * sinl * sinl;
  double W = atan2(sqrt(S), sqrt(C));
  double R = sqrt((S * C)) / W;
  double H1 = (3 * R - 1.0) / (2.0 * C);
  double H2 = (3 * R + 1.0) / (2.0 * S);
  double D = 2 * W * erad;

  uint32_t approxDistance =
      static_cast<uint32_t>(km2mi * (D * (1 + flattening * H1 * sinf * sinf * cosg * cosg -
                                          flattening * H2 * cosf * cosf * sing * sing)));
  return approxDistance;
}

//----------------------------------------------------------------------------
// getFirstUnflownAirSeg()
//----------------------------------------------------------------------------
TravelSeg*
TseUtil::getFirstUnflownAirSeg(const std::vector<TravelSeg*>& travelSegVec,
                               const DateTime& requestDT)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = travelSegVec.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = travelSegVec.end();

  std::vector<TravelSeg*>::const_iterator ufSegI = tvlSegIEnd;
  std::vector<TravelSeg*>::const_iterator openSegI = tvlSegIEnd;

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    TravelSeg* tvlSeg = (*tvlSegI);
    if (tvlSeg->segmentType() == Open)
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);

      if (!airSeg)
        continue;

      openSegI = tvlSegI;
    }
    else if (tvlSeg->segmentType() != Air)
    {
      continue;
    }

    // Since Open Air will still have date/time, we will compare date
    // regardless Open or not
    if (tvlSeg->departureDT() >= requestDT)
    {
      ufSegI = tvlSegI;
      break;
    }
  }

  if (std::distance(openSegI, ufSegI) == 1)
  {
    if (std::distance(travelSegVec.begin(), openSegI) > 0)
    {
      std::vector<TravelSeg*>::const_iterator i = openSegI;
      for (--i; i != travelSegVec.begin(); --i)
      {
        if ((*i)->segmentType() == Open)
          openSegI = i;
        else
          break;
      }
    }
    return *openSegI;
  }
  else if (ufSegI != tvlSegIEnd)
    return *ufSegI;

  return nullptr;
}

TravelSeg*
TseUtil::firstDatedSegBeforeDT(const std::vector<TravelSeg*>& tvlSegs,
                               const DateTime& refDT,
                               PricingTrx& trx)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegs.end();

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    TravelSeg* tvlSeg = *tvlSegI;

    if (tvlSeg->segmentType() == Arunk)
    {
      continue;
    }
    else if (tvlSeg->segmentType() == Open)
    {
      if (tvlSeg->departureDT().date() < refDT.date())
      {
        DateTime newDate = tvlSeg->departureDT().addDays(1);
        if (newDate.date() < refDT.date())
          return tvlSeg;
      }
    }
    else
    {
      if (tvlSeg->departureDT() < refDT)
      {
        // Considering time zone difference, give it 24 hours
        DateTime newDate(tvlSeg->departureDT().year(),
                         tvlSeg->departureDT().month(),
                         tvlSeg->departureDT().day(),
                         tvlSeg->departureDT().hours() + 24,
                         tvlSeg->departureDT().minutes(),
                         tvlSeg->departureDT().seconds());

        if (newDate < refDT)
          return tvlSeg;
      }
    }
  }

  return nullptr;
}

//----------------------------------------------------------------------------
// isNotAirSeg()
//----------------------------------------------------------------------------
bool
TseUtil::isNotAirSeg(const TravelSeg* tvlSeg)
{
  if (tvlSeg->segmentType() == Air)
  {
    return false;
  }
  else if (tvlSeg->segmentType() == Open)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);

    if (airSeg)
      return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// getGeoType
// Takes the input as geo travel type and returns 3 char geo type
//----------------------------------------------------------------------------
std::string
TseUtil::getGeoType(GeoTravelType geoTravelType)
{
  // Do Geo Travel type
  std::string gtt;
  switch (geoTravelType)
  {
  case GeoTravelType::UnknownGeoTravelType:
    gtt = "";
    break;
  case GeoTravelType::Domestic:
    gtt = "DOM";
    break;
  case GeoTravelType::International:
    gtt = "INT";
    break;
  case GeoTravelType::Transborder:
    gtt = "TRB";
    break;
  case GeoTravelType::ForeignDomestic:
    gtt = "FDM";
    break;
  default:
    break;
  }
  return gtt;
}

//----------------------------------------------------------------------------
// getGeoType
// Takes the input as char geo type and returns geo travel  type
//----------------------------------------------------------------------------
GeoTravelType
TseUtil::getGeoType(char geoTravelType)
{
  switch (geoTravelType)
  {
  case 'I':
    return GeoTravelType::International;
  case 'D':
    return GeoTravelType::Domestic;
  case 'T':
    return GeoTravelType::Transborder;
  case 'F':
    return GeoTravelType::ForeignDomestic;
  }
  return GeoTravelType::UnknownGeoTravelType;
}

//----------------------------------------------------------------------------
// boolValue()
// Converts text param to boolean value
//----------------------------------------------------------------------------
bool
TseUtil::boolValue(const std::string& parm)
{
  return (parm == "Y" || parm == "Yes" || parm == "TRUE" || parm == "y" || parm == "yes" ||
          parm == "true" || parm == "1");
}

bool
TseUtil::FFOwrtApplicabilityPred::
operator()(const PaxTypeFare* ptFare)
{
  if (!ptFare)
  {
    return false;
  }

  if (ptFare->isKeepForRoutingValidation())
  {
    return false;
  }

  bool isOneWayRequested = (_trx.journeyItin()->travelSeg().size() == 1);

  if (((ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) && isOneWayRequested))
  {
    _trx.reportError().errorCode = ErrorResponseException::FARE_APPLICABLE_FOR_ROUNDTRIP_ONLY;
    _trx.reportError().message = "$FARE APPLICABLE FOR ROUNDTRIP ONLY";

    return false;
  }
  else if ((ptFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) && !isOneWayRequested)
  {
    _trx.reportError().errorCode = ErrorResponseException::FARE_APPLICABLE_FOR_ONEWAY_ONLY;
    _trx.reportError().message = "$FARE APPLICABLE FOR ONEWAY TRAVEL ONLY";

    return false;
  }

  return true;
}

void
TseUtil::splitTextLine(const std::string& str, std::vector<std::string>& vec)
{
  splitTextLine(str, vec, DISPLAY_MAX_SIZE, false);
}

void
TseUtil::splitTextLine(const std::string& str,
                       std::vector<std::string>& vec,
                       size_t maxLineLen,
                       const bool remSpacesAtBeg)
{
  uint32_t lineLen = str.length();
  // in most cases this will be true
  if (lineLen <= maxLineLen)
  {
    vec.push_back(str);
    return;
  }
  size_t pos = 0;
  while (pos < lineLen)
  {
    if (remSpacesAtBeg && str[pos] == ' ')
    {
      ++pos;
      continue;
    }
    std::string shortLine;
    shortLine.assign(str, pos, maxLineLen);
    if (lineLen > pos + maxLineLen) // BREAK LINE
    {
      size_t cPos = shortLine.find_last_of("@/,: ");
      if (cPos != std::string::npos)
      {
        cPos++;
        shortLine.assign(str, pos, cPos);

        pos += cPos;
      }
      else
        pos += maxLineLen;
    }
    else
      pos += maxLineLen;
    vec.push_back(shortLine);
  }
}

} //tse
