//----------------------------------------------------------------------------
//  File:        Diag893Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 893 Branded Fares - display reviewed and parsed data from Branded service
//  Updates:
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
#include "Diagnostic/Diag893Collector.h"

#include "BrandedFares/BrandFeatureItem.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "DBAccess/SvcFeesFareIdInfo.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace tse
{
Diag893Collector& Diag893Collector::operator << ( const PricingTrx::BrandedMarketMap& brandedMarketMap )
{
  *this << boost::format("%|=63|\n") % boost::io::group(std::setfill('*'), "");
  *this << boost::format("*%|=61|*\n") % "DIAGNOSTIC 893 - PARSED BRANDED SERVICE RESPONSE";

  PricingTrx::BrandedMarketMap::const_iterator marketResponseMapBeg = brandedMarketMap.begin();
  PricingTrx::BrandedMarketMap::const_iterator marketResponseMapEnd = brandedMarketMap.end();
  for(; marketResponseMapBeg!=marketResponseMapEnd; ++marketResponseMapBeg)
  {
    const std::vector<MarketResponse*>& marketResponseVector = marketResponseMapBeg->second;
    std::for_each(marketResponseVector.begin(), marketResponseVector.end(), *this << boost::lambda::_1 );
  }
  return *this;
}

Diag893Collector& Diag893Collector::operator << ( const MarketResponse* marketResponse )
{
  boost::format marketTitleFormatter(" MARKET %1%-%2% ");

  boost::format formatter("%|=63|\n"
                          "BRAND SOURCE: %|-4|\n"
                          "BRAND PROGRAMS COUNTER: %|-10|\n");
  formatter % boost::io::group(std::setfill('*'),
                marketTitleFormatter %
                marketResponse->marketCriteria()->departureAirportCode() %
                marketResponse->marketCriteria()->arrivalAirportCode());
  formatter % (marketResponse->dataSource() == BRAND_SOURCE_S8? "S8" : "CBAS" );
  formatter % marketResponse->brandPrograms().size();

  *this << formatter.str();

  std::for_each(marketResponse->brandPrograms().begin(),
                marketResponse->brandPrograms().end(), *this << boost::lambda::_1 );
  return *this;
}

Diag893Collector& Diag893Collector::operator << ( const BrandProgram* brandProgram )
{
  *this << boost::format("%|=63|\n")
             % boost::io::group(std::setfill('='), boost::to_upper_copy(std::string(brandProgram->programName())));
  *this << boost::format("PROGRAM ID: %|-15|\n" )
             % boost::to_upper_copy(std::string(brandProgram->programID()));
  *this << boost::format("PROGRAM CODE: %|-10|\n")
             % boost::to_upper_copy(std::string(brandProgram->programCode()));

  addMultilineInfo(*this, std::string("PROGRAM DESCRIPTION: ")
                     + boost::to_upper_copy(brandProgram->programDescription()));
  *this << boost::format("BRAND COUNTER: %|-10|\n")
             % brandProgram->brandsData().size();

  std::for_each(brandProgram->brandsData().begin(),
                brandProgram->brandsData().end(), *this << boost::lambda::_1 );
  return *this;
}

Diag893Collector& Diag893Collector::operator << ( const BrandInfo* brandInfo )
{
  boost::format formatter("%|=63|\n"
                          "BRAND CODE: %|-50|\n"
                          "BRAND NAME: %|-30|\n"
                          "TIER NUMBER: %|-6|\n"
                          "PRIMARY FARE ID TABLE: %|-10|\n"
                          "SECONDARY FARE ID TABLE: %|-10|\n");

  formatter % boost::io::group(std::setfill('-'), "")
            % boost::to_upper_copy(std::string(brandInfo->brandCode()))
            % boost::to_upper_copy(std::string(brandInfo->brandName()))
            % brandInfo->tier()
            % brandInfo->primaryFareIdTable()
            % brandInfo->secondaryFareIdTable();

  *this << formatter.str();
  addMultilineInfo(*this, std::string("PRIMARY BOOKING CODES: ")
                     + boost::to_upper_copy(joinElementsInCollection(brandInfo->primaryBookingCode())));
  addMultilineInfo(*this, std::string("SECONDARY BOOKING CODES: ")
                     + boost::to_upper_copy(joinElementsInCollection(brandInfo->secondaryBookingCode())));
  addMultilineInfo(*this, std::string("INCLUDED FARE BASIS CODES: ")
                     + boost::to_upper_copy(joinElementsInCollection(brandInfo->includedFareBasisCode())));
  addMultilineInfo(*this, std::string("EXCLUDED FARE BASIS CODES: ")
                     + boost::to_upper_copy(joinElementsInCollection(brandInfo->excludedFareBasisCode())));

  if(!brandInfo->fareIDdataPrimaryT189().empty())
  {
      *this << "\nTABLE 189 PRIMARY DATA\n";
      std::for_each(brandInfo->fareIDdataPrimaryT189().begin(),
                    brandInfo->fareIDdataPrimaryT189().end(), *this << boost::lambda::_1 );
  }
  if(!brandInfo->fareIDdataSecondaryT189().empty())
  {
      *this << "\nTABLE 189 SECONDARY DATA\n";
      std::for_each(brandInfo->fareIDdataSecondaryT189().begin(),
                    brandInfo->fareIDdataSecondaryT189().end(), *this << boost::lambda::_1 );
  }
  if(!brandInfo->getFeatureItems().empty() && isDDINFO())
  {
    *this << "\nTABLE 166 DATA\n";
    std::for_each(brandInfo->getFeatureItems().begin(),
                  brandInfo->getFeatureItems().end(), *this << boost::lambda::_1);
  }
  return *this;
}

Diag893Collector&
Diag893Collector::operator << ( const BrandFeatureItem* brandedFeatureItem)
{
  boost::format
    formatter("%|=63|\n"
              "SEQ NR      : %|-17|SUBCODE      : %|-17|\n"
              "SERVICE     : %|-17|APPLICATION  : %|-17|\n"
              "NAME        : %|-49|\n");

  formatter % boost::io::group(std::setfill('-'), "")
            % brandedFeatureItem->getSequenceNumber()
            % brandedFeatureItem->getSubCode()
            % brandedFeatureItem->getServiceType()
            % brandedFeatureItem->getApplication()
            % brandedFeatureItem->getCommercialName();

  *this << boost::to_upper_copy(formatter.str());
  return *this;
}

Diag893Collector&
Diag893Collector::operator << ( const SvcFeesFareIdInfo* svcFeesFareIdInfo )
{
  boost::format
    formatter("%|=63|\n"
              "ITEM NR     : %|-17|SEQ NR       : %|-17|\n"
              "VENDOR      : %|-17|VALIDITYIND  : %|-17|\n"
              "FAREAPPLIND : %|-17|OWRT         : %|-17|\n"
              "RULETARIFF  : %|-17|RULETARIFFIND: %|-17|\n"
              "FARECLASS   : %|-17|FARETYPE     : %|-17|\n"
              "PSGTYPE     : %|-17|ROUTING      : %|-17|\n"
              "SOURCE      : %|-17|RULE         : %|-17|\n"
              "BOOKINGCODE1: %|-17|BOOKINGCODE2 : %|-17|\n"
              "MINFAREAMT1 : %|-17|MINFAREAMT2  : %|-17|\n"
              "MAXFAREAMT1 : %|-17|MAXFAREAMT2  : %|-17|\n"
              "CUR1        : %|-17|CUR2         : %|-17|\n"
              "NODEC1      : %|-17|NODEC2       : %|-17|\n");

  formatter % boost::io::group(std::setfill('-'), "")
            % svcFeesFareIdInfo->itemNo()
            % svcFeesFareIdInfo->seqNo()
            % svcFeesFareIdInfo->vendor()
            % svcFeesFareIdInfo->validityInd()
            % svcFeesFareIdInfo->fareApplInd()
            % svcFeesFareIdInfo->owrt()
            % svcFeesFareIdInfo->ruleTariff()
            % svcFeesFareIdInfo->ruleTariffInd()
            % svcFeesFareIdInfo->fareClass()
            % svcFeesFareIdInfo->fareType()
            % svcFeesFareIdInfo->paxType()
            % svcFeesFareIdInfo->routing()
            % svcFeesFareIdInfo->source()
            % svcFeesFareIdInfo->rule()
            % svcFeesFareIdInfo->bookingCode1()
            % svcFeesFareIdInfo->bookingCode2()
            % svcFeesFareIdInfo->minFareAmt1()
            % svcFeesFareIdInfo->minFareAmt2()
            % svcFeesFareIdInfo->maxFareAmt1()
            % svcFeesFareIdInfo->maxFareAmt2()
            % svcFeesFareIdInfo->cur1()
            % svcFeesFareIdInfo->cur2()
            % svcFeesFareIdInfo->noDec1()
            % svcFeesFareIdInfo->noDec2();

  *this << boost::to_upper_copy(formatter.str());
  return *this;
}

} // tse
