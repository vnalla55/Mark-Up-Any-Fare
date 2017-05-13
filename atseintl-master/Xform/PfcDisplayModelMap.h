//----------------------------------------------------------------------------
//
//      File: PfcDisplayModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: May, 2008
//      Authors: Piotr Lach
//
//  Copyright Sabre 2008
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
#include "DataModel/Billing.h"
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

class PfcDisplayModelMap : public DataModelMap
{
public:
  PfcDisplayModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~PfcDisplayModelMap();

  //--------------------------------------------------------------------------
  // @function PfcDisplayModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function PfcDisplayModelMap::classMapEntry
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
  // @function PfcDisplayModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName) override {}
  void saveMapEntry(std::string& tagName, std::string& text) override;
  DataModelMap* getModelMap() { return this; }

private:
  struct Mapping
  {
    void (PfcDisplayModelMap::*func)(const std::string&,
                                     const xercesc::Attributes&); // store function
    void (PfcDisplayModelMap::*trxFunc)(const std::string&,
                                        const std::string&); // Trx interaction func
    MemberMap members; // associative members
  };

  TaxTrx* _taxTrx = nullptr;

  //--------------------------------------------------------------------------
  // @functions  PfcDisplayModelMap::storeTaxInformation
  //
  // Description: This is the equivalent of a document start for a Tax
  //              request xml document.
  //
  // @param tagName - No meaning here
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storePfcDisplayInformation(const std::string& tagName, const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PfcDisplayModelMap::saveTaxInformation
  //
  // Description: This is the equivalent of a document end for a Tax
  //              request xml document.
  //
  // @param tagName
  // @return void
  //--------------------------------------------------------------------------
  void savePfcDisplayInformation(const std::string& tagName, const std::string& text);

  void storeAgentInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveAgentInformation(const std::string& tagName, const std::string& text);

  void storeBillingInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveBillingInformation(const std::string& tagName, const std::string& text);

  void storeProcOptsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveProcOptsInformation(const std::string& tagName, const std::string& text);

  void storeSegmentInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveSegmentInformation(const std::string& tagName, const std::string& text);

  void storeDynamicConfigOverride(const std::string& tagName, const xercesc::Attributes& attrs);
};
} // End namespace tse
