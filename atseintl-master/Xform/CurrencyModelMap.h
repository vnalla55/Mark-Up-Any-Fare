//----------------------------------------------------------------------------
//
//      File: CurrencyModelMap.h
//      Description: Create and interpret Data Model mappings for a Currency
//                   Conversion request.
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
#include "DataModel/CurrencyTrx.h"
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

class CurrencyModelMap : public DataModelMap
{
public:
  CurrencyModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~CurrencyModelMap();

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::classMapEntry
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
  // @function CurrencyModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName) override;

  DateTime getHistoricalDate(const char* dateStr);

private:
  struct Mapping
  {
    void (CurrencyModelMap::*func)(const xercesc::Attributes&); // store function
    void (CurrencyModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  CurrencyTrx* _currencyTrx = nullptr;

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::storeCurrencyInformation
  //
  // Description: This is the equivalent of a document start for a Currency
  //              conversion request xml document.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeCurrencyInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::saveCurrencyInformation
  //
  // Description: This is the equivalent of a document end for a Currency
  //              conversion request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveCurrencyInformation();

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::storeAgentInformation
  //
  // Description: This is the equivalent of a start element for a Currency
  //              conversion AGI element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::saveAgentInformation
  //
  // Description: This is the equivalent of a element end for a Currency
  //              conversion AGI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();
  void updateAgentInformation();

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Currency
  //              conversion BIL element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Currency
  //              conversion BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::storeOptionInformation
  //
  // Description: This is the equivalent of a start element for a Currency
  //              conversion OPT element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeOptionInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::saveOptionInformation
  //
  // Description: This is the equivalent of a element end for a Currency
  //              conversion OPT element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveOptionInformation();

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::storeProcOptsInformation
  //
  // Description: This is the equivalent of a start element for a Currency
  //              conversion PRO element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeProcOptsInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function CurrencyModelMap::saveProcOptsInformation
  //
  // Description: This is the equivalent of a element end for a Currency
  //              conversion PRO element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveProcOptsInformation();

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);
};
} // End namespace tse
