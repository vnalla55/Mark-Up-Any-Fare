//----------------------------------------------------------------------------
//
//      File: TaxModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: November 10, 2004
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

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/PaxType.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/DataModelMap.h"

#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{
class AirSeg;
class FarePath;
class Itin;

class TaxModelMap : public DataModelMap
{
public:
  friend class TaxModelMapTest;

  TaxModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~TaxModelMap();

  //--------------------------------------------------------------------------
  // @function TaxModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function TaxModelMap::classMapEntry
  //
  // Description: Multi purpose method.
  //     1.  tagName can be a class tag name
  //     2.  tagName can be a member tag name
  //     3.  tagName can be a data value
  //
  // @param tagName - value to be evaluated
  // @param atts - vector of attributes
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool classMapEntry(std::string& tagName, const xercesc::Attributes& atts) override;

  //--------------------------------------------------------------------------
  // @function TaxModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName, std::string& text) override;

private:
  struct Mapping
  {
    void (TaxModelMap::*func)(const xercesc::Attributes&); // store function
    void (TaxModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  TaxTrx* _taxTrx = nullptr;
  Itin* _itin = nullptr;
  AirSeg* _airSeg = nullptr;
  PaxType* _paxType = nullptr;
  FarePath* _farePath = nullptr;

  //--------------------------------------------------------------------------
  // @function TaxModelMap::storeTaxInformation
  //
  // Description: This is the equivalent of a document start for a Tax
  //              request xml document.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeTaxInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxModelMap::saveTaxInformation
  //
  // Description: This is the equivalent of a document end for a Tax
  //              request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxInformation();

  //--------------------------------------------------------------------------
  // @function TaxModelMap::storeItineraryInformation
  //
  // Description: Convenience method for mapping ITN sent XML summary
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------
  void storeItineraryInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxModelMap::saveItineraryInformation
  //
  // Description: Convenience method for closing out a populated
  //              Itin object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveItineraryInformation();

  //--------------------------------------------------------------------------
  // @function TaxModelMap::storePassengerInformation
  //
  // Description: Convenience method for mapping PXI sent XML passenger
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------
  void storePassengerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxModelMap::savePassengerInformation
  //
  // Description: Convenience method for closing out a populated
  //              object
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePassengerInformation();

  //--------------------------------------------------------------------------
  // @function TaxModelMap::storeFlightInformation
  //
  // Description: Convenience method for mapping FLI sent XML flight
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------
  void storeFlightInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxModelMap::saveFlightInformation
  //
  // Description: Convenience method for closing out a populated
  //              AirSeg object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFlightInformation();

  //--------------------------------------------------------------------------
  // @function TaxModelMap::storeBookingClass
  //
  // Description: Convenience method for mapping BCC sent XML flight
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------
  void storeBookingClass(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxModelMap::saveBookingClass
  //
  // Description: Convenience method for closing out a populated
  //              object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveBookingClass();

  //--------------------------------------------------------------------------
  // @function PricingModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              BIL element.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);
  //--------------------------------------------------------------------------
  // @function PricingModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);
};

} // End namespace tse

