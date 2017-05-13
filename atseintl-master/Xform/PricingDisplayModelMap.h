//----------------------------------------------------------------------------
//
//      File: PricingDisplayModelMap.h
//      Description: Create and interpret Data Model mappings for a Pricing
//                   Display request.
//      Created: January 29, 2005
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

#include "Common/Config/ConfigMan.h"
#include "DataModel/Billing.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
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
class Itin;
class PricingTrx;

class PricingDisplayModelMap : public DataModelMap
{
public:
  PricingDisplayModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~PricingDisplayModelMap();

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::classMapEntry
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
  // @function PricingDisplayModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName) override;

private:
  struct Mapping
  {
    void (PricingDisplayModelMap::*func)(const xercesc::Attributes&); // store function
    void (PricingDisplayModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  PricingTrx* _pricingTrx = nullptr;
  PaxType* _paxType = nullptr;
  AirSeg* _wddAirSeg = nullptr;
  Itin* _itin = nullptr;
  std::string _ruleCategories;

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::storePricingDisplayInformation
  //
  // Description: This is the equivalent of a document start for a Pricing
  //              Display request xml document.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storePricingDisplayInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::savePricingDisplayInformation
  //
  // Description: This is the equivalent of a document end for a Pricing
  //              Display request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void savePricingDisplayInformation();

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::storeAgentInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              Display AGI element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::saveAgentInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              Display AGI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              Display BIL element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              Display BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::storeOptionInformation
  //
  // Description: This is the equivalent of a start element for a Pricing
  //              Display OPT element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeOptionInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::saveOptionInformation
  //
  // Description: This is the equivalent of a element end for a Pricing
  //              Display OPT element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveOptionInformation();

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::storeProcOptsInformation
  //
  // Description: This is the equivalent of a start element for a Pricing Display
  //              PRO element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeProcOptsInformation(const xercesc::Attributes& attrs) {}

  //--------------------------------------------------------------------------
  // @function PricingDisplayModelMap::saveProcOptsInformation
  //
  // Description: This is the equivalent of a element end for a Pricing Display
  //              PRO element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveProcOptsInformation() {}

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);
};

} // End namespace tse

