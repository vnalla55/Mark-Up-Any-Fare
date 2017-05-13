//----------------------------------------------------------------------------
//
//      File: MileageModelMap.h
//      Description: Create and interpret Data Model mappings for a Mileage
//                   request.
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
#include "DataModel/MileageTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/DataModelMap.h"

#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{

class MileageModelMap : public DataModelMap
{
public:
  MileageModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~MileageModelMap();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function MileageModelMap::classMapEntry
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
  // @function MileageModelMap::saveMapEntry
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
    void (MileageModelMap::*func)(const xercesc::Attributes&); // store function
    void (MileageModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  MileageTrx* _mileageTrx = nullptr;
  MileageTrx::MileageItem* _mileageItem = nullptr;

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeMileageInformation
  //
  // Description: This is the equivalent of a document start for a Mileage
  //              request xml document.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeMileageInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveMileageInformation
  //
  // Description: This is the equivalent of a document end for a Mileage
  //              request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveMileageInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeAgentInformation
  //
  // Description: This is the equivalent of a start element for a Mileage
  //              AGI element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveAgentInformation
  //
  // Description: This is the equivalent of a element end for a Mileage
  //              AGI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Mileage
  //              BIL element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Mileage
  //              BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeOptionInformation
  //
  // Description: This is the equivalent of a start element for a Mileage
  //              OPT element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeOptionInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveOptionInformation
  //
  // Description: This is the equivalent of a element end for a Mileage
  //              OPT element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveOptionInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeItemInformation
  //
  // Description: This is the equivalent of a start element for a Mileage
  //              WNI element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeItemInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveItemInformation
  //
  // Description: This is the equivalent of a element end for a Mileage
  //              WNI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveItemInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeProcOptsInformation
  //
  // Description: This is the equivalent of a start element for a Mileage
  //              PRO element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeProcOptsInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveProcOptsInformation
  //
  // Description: This is the equivalent of a element end for a Mileage
  //              PRO element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveProcOptsInformation();

  //--------------------------------------------------------------------------
  // @function MileageModelMap::storeDiagInformation
  //
  // Description: Convenience method for mapping PCC sent XML diagnostic
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeDiagInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function MileageModelMap::saveDiagInformation
  //
  // Description: Convenience method for saving a populated DIG
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveDiagInformation();

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);
};

} // End namespace tse

