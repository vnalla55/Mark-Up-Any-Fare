#include "Common/TravelSegAnalysis.h"

#include "Common/LocUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"

namespace tse
{
Boundary
TravelSegAnalysis::selectTravelBoundary(const std::vector<TravelSeg*>& tvlSegs)
{
  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  for (; itr != tvlSegs.end(); itr++)
  {
    if (TravelSegUtil::isNotUSCanadaOrCanadaUS(*itr))
    {
      break;
    }

    if (itr == tvlSegs.end() - 1)
    {
      if ((*itr)->origin()->nation() == UNITED_STATES ||
          (*itr)->destination()->nation() == UNITED_STATES ||
          (*itr)->origin()->nation() == CANADA || (*itr)->destination()->nation() == CANADA)
      {
        return Boundary::USCA;
      }
      else
      {
        return Boundary::EXCEPT_USCA;
      }
    }
  } // end for

  for (itr = tvlSegs.begin(); itr != tvlSegs.end(); itr++)
  {
    if (((*itr)->origin()->subarea() != (*itr)->destination()->subarea()))
    {
      break;
    }

    if (itr == tvlSegs.end() - 1)
    {
      if ((*itr)->origin()->subarea() == EUROPE)
      {
        return Boundary::AREA_21;
      }
      else if ((*itr)->origin()->subarea() == NORTH_AMERICA)
      {
        return Boundary::AREA_11;
      }
      else
      {
        return Boundary::OTHER_SUB_IATA;
      }
    }

  } // end for

  bool area1 = false;
  bool area2 = false;
  bool area3 = false;

  for (itr = tvlSegs.begin(); itr != tvlSegs.end(); itr++)
  {
    if ((*itr)->origin()->area() == IATA_AREA1 || (*itr)->destination()->area() == IATA_AREA1)
    {
      area1 = true;
    }

    if ((*itr)->origin()->area() == IATA_AREA2 || (*itr)->destination()->area() == IATA_AREA2)
    {
      area2 = true;
    }

    if ((*itr)->origin()->area() == IATA_AREA3 || (*itr)->destination()->area() == IATA_AREA3)
    {
      area3 = true;
    }

    if (itr == tvlSegs.end() - 1)
    {
      if (area1 && area2 && area3)
      {
        return Boundary::ALL_IATA;
      }
      else if ((area1 && area2) || (area1 && area3) || (area2 && area3))
      {
        return Boundary::TWO_IATA;
      }
      else
      {
        return Boundary::ONE_IATA;
      }
    }
  }

  return Boundary::UNKNOWN;
}

//---------------------------------------------------------------------------
// getFirstIntlFlt() - Returns the carrier of the first international flight.
//---------------------------------------------------------------------------
CarrierCode
TravelSegAnalysis::getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs,
                                   TravelSeg*& primarySector)
{
  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  for (itr = tvlSegs.begin(); itr != tvlSegs.end(); itr++)
  {
    if (((*itr)->origin()->nation() != (*itr)->destination()->nation()))
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (airSeg != nullptr)
      {
        primarySector = *itr;
        return airSeg->carrier().empty() == false ? airSeg->carrier() : INDUSTRY_CARRIER;
      }
      else
      {
        primarySector = *itr;
        return INDUSTRY_CARRIER;
      }
    }
  }
  return "XX";
}

//---------------------------------------------------------------------------
// getFirstIntlFlt() - Returns the carrier of the first international flight.
//---------------------------------------------------------------------------
CarrierCode
TravelSegAnalysis::getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs) const
{
  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  for (itr = tvlSegs.begin(); itr != tvlSegs.end(); itr++)
  {
    if ((*itr)->origin()->nation() != (*itr)->destination()->nation())
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (LIKELY(airSeg != nullptr))
      {
        return airSeg->carrier().empty() == false ? airSeg->carrier() : INDUSTRY_CARRIER;
      }
      else
      {
        return INDUSTRY_CARRIER;
      }
    }
  }
  return "XX";
}

//---------------------------------------------------------------------------
// getLastIntlFlt() - Returns the carrier of the last international flight.
//---------------------------------------------------------------------------
CarrierCode
TravelSegAnalysis::getLastIntlFlt(const std::vector<TravelSeg*>& tvlSegs, TravelSeg*& primarySector)
{
  std::vector<TravelSeg*>::const_reverse_iterator itr = tvlSegs.rbegin();

  for (itr = tvlSegs.rbegin(); itr != tvlSegs.rend(); ++itr)
  {
    if ((*itr)->origin()->nation() != (*itr)->destination()->nation())
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (airSeg != nullptr)
      {
        primarySector = *itr;
        return airSeg->carrier().empty() == false ? airSeg->carrier() : INDUSTRY_CARRIER;
      }
      else
      {
        primarySector = *itr;
        return INDUSTRY_CARRIER;
      }
    }
  }
  return "XX";
}

TravelSeg*
TravelSegAnalysis::getHighestTPMSector(const std::vector<TravelSeg*>& tvlSegs)
{
  uint32_t maxTPM = 0;
  TravelSeg* ret = nullptr;

  std::vector<TravelSeg*>::const_iterator i = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator iEnd = tvlSegs.end();
  for (; i != iEnd; ++i)
  {
    TravelSeg* tvlSeg = *i;
    if (tvlSeg->isAir())
    {
      const uint32_t tempTPM = getTPM(*tvlSeg);
      if (tempTPM > maxTPM)
      {
        maxTPM = tempTPM;
        ret = tvlSeg;
      }
    }
  }

  return ret;
}

//---------------------------------------------------------------------------
// getCarrierInIATA1()
//--------------------------------------------------------------------------

bool
TravelSegAnalysis::getCarrierInIATA1(Itin& itn,
                                     std::vector<TravelSeg*>::iterator first,
                                     std::vector<TravelSeg*>::iterator last) const
{
  std::vector<TravelSeg*>::iterator itr = itn.travelSeg().begin();

  for (itr = first; itr != last; itr++)
  {
    if (((*itr)->origin()->area() == IATA_AREA1 || (*itr)->destination()->area() == IATA_AREA1) &&
        ((*itr)->origin()->area() != (*itr)->destination()->area()))
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (airSeg != nullptr)
      {
        itn.ticketingCarrier() = airSeg->carrier();
        itn.geoTravelType() = GeoTravelType::International;
        return true;
      }

      const ArunkSeg* arunkSeg = dynamic_cast<const ArunkSeg*>(*itr);

      if (arunkSeg != nullptr)
      {
        itn.ticketingCarrier() = INDUSTRY_CARRIER;
        itn.geoTravelType() = GeoTravelType::International;
        return true;
      }
    }
  }
  return false;
}

//---------------------------------------------------------------------------
// getTPM()
//---------------------------------------------------------------------------
uint32_t
TravelSegAnalysis::getTPM(const TravelSeg& tvlSeg) const
{
  uint32_t tpm = 0;

  const Loc& orig = *tvlSeg.origin();
  const Loc& dest = *tvlSeg.destination();

  DataHandle dataHandle;

  const Mileage* mileage =
      dataHandle.getMileage(orig.loc(), dest.loc(), TPM, GlobalDirection::EH, tvlSeg.departureDT());
  if (mileage == nullptr)
  {
    tpm = TseUtil::greatCircleMiles(orig, dest);
  }
  else
  {
    tpm = mileage->mileage();
  }
  return tpm;
}

void
TravelSegAnalysis::setGeoTravelType(TravelSeg*& tvlSeg) const
{
  Loc originLoc = *(tvlSeg->origin());
  Loc destLoc = *(tvlSeg->destination());
  if (LocUtil::isDomestic(originLoc, destLoc))
    tvlSeg->geoTravelType() = GeoTravelType::Domestic;
  else if (LocUtil::isInternational(originLoc, destLoc))
    tvlSeg->geoTravelType() = GeoTravelType::International;
  else if (LocUtil::isTransBorder(originLoc, destLoc))
    tvlSeg->geoTravelType() = GeoTravelType::Transborder;
  else if (LocUtil::isForeignDomestic(originLoc, destLoc))
    tvlSeg->geoTravelType() = GeoTravelType::ForeignDomestic;
}
}
