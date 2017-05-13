//----------------------------------------------------------------------------
//
//      File: TaxDisplayModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: August, 2007
//      Authors: Dean Van Decker
//
//  Copyright Sabre 2007
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

class TaxDisplayModelMap : public DataModelMap
{
public:
  TaxDisplayModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~TaxDisplayModelMap();

  //--------------------------------------------------------------------------
  // @function TaxDisplayModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function TaxDisplayModelMap::classMapEntry
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
  // @function TaxDisplayModelMap::saveMapEntry
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
    void (TaxDisplayModelMap::*func)(const std::string&,
                                     const xercesc::Attributes&); // store function
    void (TaxDisplayModelMap::*trxFunc)(const std::string&,
                                        const std::string&); // Trx interaction func
    MemberMap members; // associative members
  };

  TaxTrx* _taxTrx = nullptr;

  //--------------------------------------------------------------------------
  // @functions  TaxDisplayModelMap::storeTaxInformation
  //
  // Description: This is the equivalent of a document start for a Tax
  //              request xml document.
  //
  // @param tagName - No meaning here
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeTaxInformation(const std::string& tagName, const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxDisplayModelMap::saveTaxInformation
  //
  // Description: This is the equivalent of a document end for a Tax
  //              request xml document.
  //
  // @param tagName
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxInformation(const std::string& tagName, const std::string& text);

  //--------------------------------------------------------------------------
  // @function TaxDisplayModelMap::*
  //
  // Description: Convenience methods for mapping XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param tagName - value to be stored away
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------
  void storeNationInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAirportInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTaxCodeInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTaxTypeInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeCarrierCodeInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeSequenceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeCategoryInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeHistoryDateInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeRetrievalDateInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeMenuInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeReissueInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTaxHelpInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeUSTaxHelpInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeCalculateTaxInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeSourceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTPAExtensionsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeUserInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeStationInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeBranchInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storePartitionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeSetAddressInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeServiceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeClientServiceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeParentServiceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAAACityInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAgentSineInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeActionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTransactionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeClientTransactionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeParentTransactionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeOfficeDesignatorInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeDynamicConfigOverride(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTxEntryInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeCarrierCodesInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeDetailLevelsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeRequestDateInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTravelDateInformation(const std::string& tagName, const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function TaxOTAModelMap::*
  //
  // Description: Convenience methods for closing out a populated
  //              object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveNationInformation(const std::string& tagName, const std::string& text);
  void saveAirportInformation(const std::string& tagName, const std::string& text);
  void saveTaxCodeInformation(const std::string& tagName, const std::string& text);
  void saveTaxTypeInformation(const std::string& tagName, const std::string& text);
  void saveCarrierCodeInformation(const std::string& tagName, const std::string& text);
  void saveSequenceInformation(const std::string& tagName, const std::string& text);
  void saveCategoryInformation(const std::string& tagName, const std::string& text);
  void saveHistoryDateInformation(const std::string& tagName, const std::string& text);
  void saveRetrievalDateInformation(const std::string& tagName, const std::string& text);
  void saveMenuInformation(const std::string& tagName, const std::string& text);
  void saveReissueInformation(const std::string& tagName, const std::string& text);
  void saveTaxHelpInformation(const std::string& tagName, const std::string& text);
  void saveUSTaxHelpInformation(const std::string& tagName, const std::string& text);
  void saveCalculateTaxInformation(const std::string& tagName, const std::string& text);
  void saveSourceInformation(const std::string& tagName, const std::string& text);
  void saveTPAExtensionsInformation(const std::string& tagName, const std::string& text);
  void saveUserInformation(const std::string& tagName, const std::string& text);
  void saveStationInformation(const std::string& tagName, const std::string& text);
  void saveBranchInformation(const std::string& tagName, const std::string& text);
  void savePartitionInformation(const std::string& tagName, const std::string& text);
  void saveSetAddressInformation(const std::string& tagName, const std::string& text);
  void saveServiceInformation(const std::string& tagName, const std::string& text);
  void saveClientServiceInformation(const std::string& tagName, const std::string& text);
  void saveParentServiceInformation(const std::string& tagName, const std::string& text);
  void saveAAACityInformation(const std::string& tagName, const std::string& text);
  void saveAgentSineInformation(const std::string& tagName, const std::string& text);
  void saveActionInformation(const std::string& tagName, const std::string& text);
  void saveTransactionInformation(const std::string& tagName, const std::string& text);
  void saveClientTransactionInformation(const std::string& tagName, const std::string& text);
  void saveParentTransactionInformation(const std::string& tagName, const std::string& text);
  void saveOfficeDesignatorInformation(const std::string& tagName, const std::string& text);
  void saveTxEntryInformation(const std::string& tagName, const std::string& text);
  void saveCarrierCodesInformation(const std::string& tagName, const std::string& text);
  void saveDetailLevelsInformation(const std::string& tagName, const std::string& text);
  void saveRequestDateInformation(const std::string& tagName, const std::string& text);
  void saveTravelDateInformation(const std::string& tagName, const std::string& text);
};

} // End namespace tse

