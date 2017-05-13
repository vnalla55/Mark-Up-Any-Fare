//----------------------------------------------------------------------------
//
//      File: TaxInfoModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: Dec, 2008
//      Authors: Jakub Kubica
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

#include "Common/Config/ConfigMan.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/DataModelMap.h"

#include <xercesc/sax2/Attributes.hpp>

namespace tse
{

class TaxInfoModelMap : public DataModelMap
{
public:
  TaxInfoModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~TaxInfoModelMap();

  //--------------------------------------------------------------------------
  // @function TaxInfoModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function TaxInfoModelMap::classMapEntry
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
  // @function TaxInfoModelMap::saveMapEntry
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
  TaxTrx* _taxTrx = nullptr;
  TaxInfoItem* _actualTaxInfoItem = nullptr;

  struct Mapping
  {
    void (TaxInfoModelMap::*func)(const std::string&, const xercesc::Attributes&); // store function
    void (TaxInfoModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  void isTransactionOK();

  void storeTaxCommonInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveTaxCommonInformation();

  void storeAgentInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveAgentInformation();

  void storeBillingInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveBillingInformation();

  void storeTaxInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveTaxInformation();

  void storeProcOptsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveProcOptsInformation();

  void storeCountryInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveCountryInformation();

  void storeAirportInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveAirportInformation();

  void storeDynamicConfigOverride(const std::string& tagName, const xercesc::Attributes& attrs);

  const bool convertTimeHHMM(const char* inDate, int32_t& hour, int32_t& min);
};
} // namespace tse
