//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Diagnostic/Diag188Collector.h"

#include "Common/GoverningCarrier.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Diagnostic/DiagnosticUtil.h"

#include <iomanip>
#include <iostream>

namespace tse
{

namespace
{
const std::string HEADER = "*********************** START DIAG 188 ***********************\n";
const std::string FOOTER = "*********************** END   DIAG 188 ***********************\n";
const std::string TRAVEL_SEGMENTS           = "*** TRAVEL SEGMENTS ***";
const std::string EXCHANGE_ITIN             = "*** EXCHANGE ITIN ***";
const std::string FLOWN_ITIN                = "*** FLOWN ITIN ***";
const std::string TICKETED_ITIN             = "*** TICKETED ITIN ***";
const std::string PRIMARY_SECTOR            = "*** PRIMARY SECTOR ***";
const std::string PRIMARY_SECTOR_NULL       = "PRIMARY SECTOR IS NULL";
const std::string NUMBER_OF_FARE_MARKETS    = "NUMBER OF FARE MARKETS IN TRX: ";
const std::string SIDE_TRIPS                = "*** SIDE TRIPS ***";
const std::string NUMBER_OF_SIDE_TRIPS      = "NUMBER OF SIDE TRIPS: ";
const std::string SIDE_TRIP_NUMBER          = "SIDE TRIP NUMBER: ";
const std::string TPM_MILEAGE               = "TPM MILEAGE FOR GOVERNING CARRIER CALCULATION: ";
const std::string ORIGIN_DESTINATION        = "ORIGIN-DESTINATION : ";
const std::string GOV_CARRIER               = "GOVERNING CARRIER  : ";
const std::string GLOBAL_DIRECTION          = "GLOBAL DIRECTION   : ";
const std::string GEO_TRAVEL_TYPE           = "GEO TRAVEL TYPE    : ";
const std::string DIRECTION                 = "DIRECTION          : ";
const std::string TRAVEL_DATE               = "TRAVEL DATE        : ";
}


Diag188Collector&
Diag188Collector::operator << (const PricingTrx& trx)
{
  if ( !isActive() )
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  const ExchangePricingTrx* exchangePricingTrx = dynamic_cast<const ExchangePricingTrx*>(&trx);
  if ( exchangePricingTrx )
  {
    *this << static_cast<const ExchangePricingTrx&>(trx);
    return *this;
  }

  const RefundPricingTrx* refundPricingTrx = dynamic_cast<const RefundPricingTrx*>(&trx);
  if ( refundPricingTrx )
  {
    *this << static_cast<const RefundPricingTrx&>(trx);
    return *this;
  }

  const RexPricingTrx* rexPricingTrx = dynamic_cast<const RexPricingTrx*>(&trx);
  if ( rexPricingTrx )
  {
    *this << static_cast<const RexPricingTrx&>(trx);
    return *this;
  }

  printHeader();
  lineSkip( 1 );

  dc << TRAVEL_SEGMENTS << std::endl;
  printTravelSegments( trx.travelSeg() );
  lineSkip( 1 );

  printTpmSegs( trx.travelSeg() );
  lineSkip( 1 );

  printFareMarkets( trx );
  lineSkip( 1 );

  printFooter();

  return *this;
}

Diag188Collector&
Diag188Collector::operator << (const ExchangePricingTrx& excTrx)
{
  if ( !isActive() )
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  printHeader();
  lineSkip( 1 );

  dc << TRAVEL_SEGMENTS << std::endl;
  printTravelSegments( excTrx.travelSeg() );
  lineSkip( 1 );
  printTpmSegs( excTrx.travelSeg() );
  lineSkip( 1 );

  if ( !excTrx.exchangeItin().empty() )
  {
    dc << EXCHANGE_ITIN << std::endl;
    printTravelSegments( excTrx.exchangeItin()[0]->travelSeg() );
    lineSkip( 1 );
    printTpmSegs( excTrx.exchangeItin()[0]->travelSeg() );
    lineSkip( 1 );
  }

  printFareMarkets( excTrx );
  lineSkip( 1 );

  printFooter();

  return *this;
}

Diag188Collector&
Diag188Collector::operator << (const RefundPricingTrx& refTrx)
{
  if ( !isActive() )
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  printHeader();
  lineSkip( 1 );

  if ( !refTrx.newItin().empty() )
  {
    dc << FLOWN_ITIN << std::endl;
    printTravelSegments( refTrx.newItin().front()->travelSeg() );
    lineSkip( 1 );
    printTpmSegs( refTrx.newItin().front()->travelSeg() );
    lineSkip( 1 );
  }

  if ( !refTrx.exchangeItin().empty() )
  {
    dc << TICKETED_ITIN << std::endl;
    printTravelSegments( refTrx.exchangeItin().front()->travelSeg() );
    lineSkip( 1 );
    printTpmSegs( refTrx.exchangeItin().front()->travelSeg() );
    lineSkip( 1 );
  }

  printFareMarkets( refTrx );
  lineSkip( 1 );

  printFooter();

  return *this;
}

Diag188Collector&
Diag188Collector::operator << (const RexPricingTrx& rexTrx)
{
  if ( !isActive() )
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf( std::ios::left, std::ios::adjustfield );

  printHeader();
  lineSkip( 1 );

  dc << TRAVEL_SEGMENTS << std::endl;
  printTravelSegments( rexTrx.travelSeg() );
  lineSkip( 1 );
  printTpmSegs( rexTrx.travelSeg() );
  lineSkip( 1 );

  if ( !rexTrx.exchangeItin().empty() )
  {
    dc << EXCHANGE_ITIN << std::endl;
    printTravelSegments( rexTrx.exchangeItin().front()->travelSeg() );
    lineSkip( 1 );
    printTpmSegs( rexTrx.exchangeItin().front()->travelSeg() );
    lineSkip( 1 );
  }

  printFareMarkets( rexTrx );
  lineSkip( 1 );

  printFooter();

  return *this;
}

void
Diag188Collector::printArunk(const TravelSeg* travelSeg)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  if ( travelSeg )
  {
    dc << travelSeg->origAirport() << "-" << travelSeg->destAirport();
  }
  else
  {
    dc << "       ";
  }
  dc << "       ARUNK" << std::endl;
}

void
Diag188Collector::printFareMarket(const FareMarket& fm)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << ORIGIN_DESTINATION << fm.origin()->loc() << "-" << fm.destination()->loc() << std::endl;
  dc << GOV_CARRIER << fm.governingCarrier() << std::endl;
  dc << GLOBAL_DIRECTION
     << DiagnosticUtil::globalDirectionToString( fm.getGlobalDirection() ) << std::endl;
  dc << GEO_TRAVEL_TYPE
     << DiagnosticUtil::geoTravelTypeToString( fm.geoTravelType() ) << std::endl;
  dc << DIRECTION << DiagnosticUtil::directionToString( fm.direction() ) << std::endl;
  dc << TRAVEL_DATE << fm.travelDate().dateToSqlString() << std::endl;
  lineSkip( 1 );

  printTpmMileage( fm );
  lineSkip (1 );

  dc << TRAVEL_SEGMENTS << std::endl;
  printTravelSegments( fm.travelSeg() );

  if ( fm.primarySector() )
  {
    dc << PRIMARY_SECTOR << std::endl;
    printTravelSegment( fm.primarySector() );
  }
  else
  {
    dc << "   " << PRIMARY_SECTOR_NULL << std::endl;
  }

  printSideTrips(fm);
  lineSkip( 1 );

}

void
Diag188Collector::printFareMarketHeader(uint16_t fmNumber)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  lineSkip( 0 );
  dc << "*** FARE MARKET " << fmNumber << " ***" << std::endl;
  lineSkip( 0 );
}

void
Diag188Collector::printFareMarkets(const PricingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  lineSkip( 0 );
  dc << NUMBER_OF_FARE_MARKETS << trx.fareMarket().size() << std::endl;
  lineSkip(1 );

  uint16_t fmNumber = 0;
  for (FareMarket* fm : trx.fareMarket())
  {
    printFareMarketHeader( ++fmNumber );
    printFareMarket( *fm );
  }
}

void
Diag188Collector::printFooter()
{
  *this << FOOTER;
}

void
Diag188Collector::printHeader()
{
  *this << HEADER;
}

void
Diag188Collector::printSideTrips(const FareMarket& fm)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << SIDE_TRIPS << std::endl;
  dc << NUMBER_OF_SIDE_TRIPS << fm.sideTripTravelSeg().size() << std::endl;

  uint16_t sideTripNumber = 0;
  for (std::vector<TravelSeg*> sideTrip : fm.sideTripTravelSeg())
  {
    dc << SIDE_TRIP_NUMBER << ++sideTripNumber << std::endl;
    for (TravelSeg* travelSeg : sideTrip)
    {
      printTravelSegment( travelSeg );
    }
  }
}

void
Diag188Collector::printTpmSeg(const AirSeg* airSeg, uint32_t tpmSeg, uint32_t tpmTotal)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  if ( airSeg )
  {
    dc << airSeg->origAirport() << "-" << airSeg->carrier() << "-" << airSeg->destAirport();
    dc << "    " << std::setw(5) << tpmSeg << "     ";
    if ( tpmTotal > 0 )
      dc << std::setw(6) << tpmTotal;
  }
  dc << std::endl;
}

void
Diag188Collector::printTpmMileage(const FareMarket& fm)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  const bool skipTpm = fm.travelBoundary().isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA) ||
                       fm.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA);
  if ( skipTpm )
  {
    dc << TPM_MILEAGE << "SKIP" << std::endl;
  }
  else
  {
    dc << TPM_MILEAGE << std::endl;
    lineSkip( 1 );
    printTpmSegs( fm.travelSeg() );
  }
}

void
Diag188Collector::printTpmSegs(const std::vector<TravelSeg*> travelSegs)
{
  *this << "SEGMENTS      TPM       TOTAL" << std::endl;

  PricingTrx* prcTrx = dynamic_cast<PricingTrx*>( trx() );
  if ( !prcTrx )
  {
    *this << "UNABLE TO DETERMINE TPM. UNSUPPORTED TRANSACTION TYPE." << std::endl;
    return;
  }

  GoverningCarrier govCxrUtil( prcTrx );
  AirSeg* prevAirSeg = nullptr;
  uint32_t segmentTpm = 0;
  uint32_t totalTpm   = 0;

  for (TravelSeg* travelSeg : travelSegs)
  {
    AirSeg* currAirSeg = dynamic_cast<AirSeg*>(travelSeg);

    if ( !currAirSeg )
    {
      if ( prevAirSeg )
      {
        printTpmSeg( prevAirSeg, segmentTpm, totalTpm );
      }
      printArunk( travelSeg );
      segmentTpm = 0;
      totalTpm   = 0;
      prevAirSeg = nullptr;
    }
    else if ( prevAirSeg && prevAirSeg->carrier() == currAirSeg->carrier() )
    {
      printTpmSeg( prevAirSeg, segmentTpm, 0 );
      segmentTpm = govCxrUtil.getTPM( *currAirSeg );
      totalTpm += segmentTpm;
      prevAirSeg = currAirSeg;
    }
    else
    {
      if ( prevAirSeg )
      {
        printTpmSeg( prevAirSeg, segmentTpm, totalTpm );
      }
      segmentTpm = govCxrUtil.getTPM( *currAirSeg );
      totalTpm = segmentTpm;
      prevAirSeg = currAirSeg;
    }
  }

  if ( prevAirSeg )
    printTpmSeg( prevAirSeg, segmentTpm, totalTpm );
}

void
Diag188Collector::printTravelSegment(const TravelSeg* tvlSeg)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);

  if ( airSeg )
  {
    dc << std::setiosflags(std::ios::right) << std::setw(2) << tvlSeg->pnrSegment() << " ";
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << airSeg->carrier();
    dc << std::setw(4) << airSeg->flightNumber();
    dc << std::setw(2) << airSeg->getBookingCode() << " ";
    dc << std::setw(5) << airSeg->departureDT().dateToString(DDMMM, "") << " ";
    dc << airSeg->origAirport() << airSeg->destAirport() << " ";
    dc << DiagnosticUtil::geoTravelTypeTo3CharString(airSeg->geoTravelType()) << std::endl;
  }
  else if ( tvlSeg )
  {
    dc << "                 ";
    dc << tvlSeg->origAirport() << tvlSeg->destAirport() << std::endl;
  }
  else
  {
    dc << std::endl;
  }
}

void
Diag188Collector::printTravelSegments(const std::vector<TravelSeg*> travelSegs)
{
  for (TravelSeg* travelSeg : travelSegs)
  {
    printTravelSegment( travelSeg );
  }
}

} // namespace tse
