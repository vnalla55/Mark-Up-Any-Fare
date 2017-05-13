//----------------------------------------------------------------------------
//
//      File: TaxResponseFormatter.h
//      Description: Class to format Tax responses back to sending client
//      Created: February 17, 2005
//      Authors: Mike Carroll
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

#pragma once

#include "Common/XMLConstruct.h"
#include "DataModel/TaxTrx.h"


namespace tse
{

class TaxResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::TaxResponseFormatter
  //
  // Description: constructor
  //
  // @param none
  //--------------------------------------------------------------------------
  TaxResponseFormatter();

  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::~TaxResponseFormatter
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~TaxResponseFormatter();

  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::formatResponse
  //
  // Description: Prepare a TaxRequest response tagged suitably for client
  //
  // @param taxTrx - a valid TaxTrx
  //--------------------------------------------------------------------------
  void formatResponse(TaxTrx& taxTrx);

protected:
  // Nothing

private:

  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::getTotalItinTax
  //
  // Description: Get an itineraries total tax
  //
  // @param itin - itinerary to be accummulated
  // @param noDec - no of decimals to be determined
  // @param ticketingDate - transaction ticketing date
  //--------------------------------------------------------------------------
  MoneyAmount getTotalItinTax(const Itin* itin, uint16_t& noDec, const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::getPassengerTotalTax
  //
  // Description: Get passenger total tax for the given itinerary
  //
  // @param itin - itinerary to be accummulated
  // @param paxType - passenger type to be accummulated
  //--------------------------------------------------------------------------
  MoneyAmount getPassengerTotalTax(const Itin* itin, const PaxType* paxType);

  //--------------------------------------------------------------------------
  // @function TaxResponseFormatter::addTaxDetails
  //
  // Description: Add passenger detail tax information to the payload
  //
  // @param itin - itinerary to be accummulated
  // @param paxType - passenger type to be accummulated
  // @param construct - XMLConstruct to use
  // @param ticketingDate - transaction ticketing date
  //--------------------------------------------------------------------------
  void addTaxDetails(const Itin* itin,
                     const PaxType* paxType,
                     XMLConstruct& construct,
                     const DateTime& ticketingDate);

}; // End class TaxResponseFormatter

} // End namespace tse

