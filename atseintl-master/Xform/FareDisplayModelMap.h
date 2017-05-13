//----------------------------------------------------------------------------
//
//      File: FareDisplayModelMap.h
//      Description: Create and interpret Data Model mappings for Fare Display
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
#include "DataModel/ERDFareComp.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/Trx.h"
#include "Xform/DataModelMap.h"
#include "Xform/RequestXmlValidator.h"

#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{
class AirSeg;
class ERDFareComp;
class ERDFltSeg;
class XMLChString;

static constexpr Indicator TRUE_IND = 'T';
static constexpr Indicator CHILD_IND = 'C';
static constexpr Indicator INFANT_IND = 'I';
typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

class FareDisplayModelMap : public DataModelMap
{
  friend class FareDisplayModelMapTest;

public:
  FareDisplayModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~FareDisplayModelMap();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::verifyLocation
  //
  // Description: Verifies location. If location is not valid, throws an exception
  //
  // @param none
  // @return char* - Verified location
  //--------------------------------------------------------------------------
  const char* verifyLocation(const char* loc);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::classMapEntry
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
  // @function FareDisplayModelMap::saveMapEntry
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
    void (FareDisplayModelMap::*func)(const xercesc::Attributes&); // store function
    void (FareDisplayModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  FareDisplayTrx* _fareDisplayTrx = nullptr;
  AirSeg* _airSeg = nullptr;
  ERDFareComp* _erdFareComp = nullptr;
  ERDFltSeg* _erdFltSeg = nullptr;
  HPUSection _hpuSection;
  PaxTypeCode _paxType;
  uint16_t _paxTypeNumber = 0;
  PaxTypeCode _requestedPaxType;
  PaxTypeCode _actualPaxType;
  bool _inDts = false;
  bool _ticketDatePresent = false;
  bool _departureDatePresent = false;
  RequestXmlValidator _reqXmlValidator;
  bool _isMcpPartition = false;

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::nullStoreFunction
  //
  // Description: Used when no parsing action is needed for an element
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void nullStoreFunction(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::nullSaveFunction
  //
  // Description: Used when no parsing action is needed for an element
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void nullSaveFunction();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeFareDisplayInformation
  //
  // Description: This is the equivalent of a document start for a Fare
  //              Display request xml document.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeFareDisplayInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveFareDisplayInformation
  //
  // Description: This is the equivalent of a document end for a Fare
  //              Display request xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveFareDisplayInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeAgentInformation
  //
  // Description: This is the equivalent of a start element for a Fare
  //              Display AGI element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveAgentInformation
  //
  // Description: This is the equivalent of a element end for a Fare
  //              Display AGI element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeBillingInformation
  //
  // Description: This is the equivalent of a start element for a Fare
  //              Display BIL element.
  //
  // @param attrs   - Attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeCommonBillingInformation
  //
  // Description: This is the equivalent of a start element for a Fare
  //              Display BIL element for common elements
  //
  // @param attrs   - Attribute list
  // @param billing - Billing Information
  // @return void
  //--------------------------------------------------------------------------
  void
  storeCommonBillingInformation(const int i, const xercesc::Attributes& attrs, Billing& billing);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveBillingInformation
  //
  // Description: This is the equivalent of a element end for a Fare
  //              Display BIL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storePreferredCarrier
  //
  // Description: This is the equivalent of a start element for a Fare
  //              Display PCL element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storePreferredCarrier(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::savePreferredCarrier
  //
  // Description: This is the equivalent of a element end for a Fare
  //              Display PCL element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void savePreferredCarrier();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeProcessingOptions
  //
  // Description: This is the equivalent of a start element for a Fare
  //              Display PRO element.
  //
  // @param position - No meaning here
  // @return void
  //--------------------------------------------------------------------------
  void storeProcessingOptions(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveProcessingOptions
  //
  // Description: This is the equivalent of a element end for a Fare
  //              Display PRO element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveProcessingOptions();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storePassengerInformation
  //
  // Description: Convenience method for mapping PCC sent XML passenger
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePassengerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::savePassengerInformation
  //
  // Description: Convenience method for saving a populated travel
  //              segment to the trx vector.
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePassengerInformation();
  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeDiagInformation
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
  // @function FareDisplayModelMap::saveDiagInformation
  //
  // Description: Convenience method for saving a populated DIG
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveDiagInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveRuleCategoryInformation
  //
  // Description: Convenience method for saving a populated rule category
  //              information to options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveRuleCategoryInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeRuleCategoryInformation
  //
  // Description: Convenience method for mapping RCG sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeRuleCategoryInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveAlphaCodeInformation
  //
  // Description: Convenience method for saving a populated alpha code
  //              information to options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveAlphaCodeInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeAlphaCodeInformation
  //
  // Description: Convenience method for mapping ACD sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeAlphaCodeInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveCombinabilityCodeInformation
  //
  // Description: Convenience method for saving a populated combinability code
  //              information to options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCombinabilityCodeInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeCombinabilityCodeInformation
  //
  // Description: Convenience method for mapping CCD sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCombinabilityCodeInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveSecondaryMarketsAndCarrierInformation
  //
  // Description: Convenience method for saving a populated secondary markets
  //              and carrier information to the request.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSecondaryMarketsAndCarrierInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeSecondaryMarketsAndCarrierInformation
  //
  // Description: Convenience method for mapping SMC sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSecondaryMarketsAndCarrierInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveCAT25Information
  //
  // Description: Convenience method for saving a populated CAT25
  //              information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCAT25Information();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeCAT25Information
  //
  // Description: Convenience method for mapping C25 sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCAT25Information(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveCAT35Information
  //
  // Description: Convenience method for saving a populated CAT35
  //              information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCAT35Information();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeCAT35Information
  //
  // Description: Convenience method for mapping C35 sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCAT35Information(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveDFIInformation
  //
  // Description: Convenience method for saving a populated discounted
  //              fare information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveDFIInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeDFIInformation
  //
  // Description: Convenience method for mapping DFI sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeDFIInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeECNInformation
  //
  // Description: Convenience method for mapping Construction sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeECNInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveOAOInformation
  //
  // Description: Convenience method for saving a populated discounted
  //              fare information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveOAOInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeOAOInformation
  //
  // Description: Convenience method for mapping DFI sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeOAOInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveDAOInformation
  //
  // Description: Convenience method for saving a populated discounted
  //              fare information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveDAOInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeDAOInformation
  //
  // Description: Convenience method for mapping DFI sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeDAOInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveCALInformation
  //
  // Description: Convenience method for saving a populated discounted
  //              fare information to the options.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCALInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeCALInformation
  //
  // Description: Convenience method for mapping CAL sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCALInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveSEGInformation
  //
  // Description: Convenience method for saving a travel segment
  //              information to the ERD segment.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSEGInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeSEGInformation
  //
  // Description: Convenience method for mapping SEG sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSEGInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeSUMInformation
  //
  // Description: Convenience method for mapping ERD sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSUMInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeERDInformation
  //
  // Description: Convenience method for mapping ERD sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeERDInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeHPUInformation
  //
  // Description: Method for mapping HPU sent XML information
  //              into the Data Model.  Must have a valid mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeHPUInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::storeDTSInformation
  //
  // Description: Convenience method for mapping DTS sent XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeDTSInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveDTSInformation
  //
  // Description: This is the equivalent of a element end for a Fare
  //              Display DTS element.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveDTSInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::setInclusionDefaults
  //
  // Description: Set inclusion code defaults based on input qualifiers
  //
  // @param request - a reference to the transactions request
  // @param options - a reference to the transactions options
  // @param dataHandle - a reference to the transactions dataHandle
  // @return true if present, false otherwise
  //--------------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // adds preferred carrier in the carrier list and removes duplicates.
  //-----------------------------------------------------------------------
  void addPreferredCarrier(const CarrierCode& cxr);
  /**
   * @@WARNING
   * This is a hard coding for FP
   * This is done only for LR ( ref: PL 10627)
   * In FF this will be driven by DataBase.
   */

  void setMultiTransportCity();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap:: isValidPaxType
  //
  // Description: Validates pax type codes input
  //
  // @param paxTypeCodes - a reference to the vector of pax type codes
  // @param dataHandle - a reference to the transactions dataHandle
  // @return true if valid pax types, false otherwise
  //--------------------------------------------------------------------------
  bool isValidPaxType(const std::vector<PaxTypeCode>& paxTypeCodes, DataHandle& dataHandle);

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap:: setCAL_FareCompDepartureDate
  //
  // Description: Sets departureDate to the first travel Date of the
  //                    Pricing Unit.
  //
  //--------------------------------------------------------------------------
  void setCAL_FareCompDepartureDate();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap:: saveERDInformation
  //
  // Description: Sets unique fare basis and cat35 fare basis.
  //
  //--------------------------------------------------------------------------
  void saveERDInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::saveHPUInformation
  //
  // Description: Sets asl source PCC if ORG is not used
  //
  //--------------------------------------------------------------------------
  void saveHPUInformation();

  //--------------------------------------------------------------------------
  // @function FareDisplayModelMap::parseSegmentNumbers
  //
  // Description: Parses string of segment numbers into vector of integers
  //
  // @param string - a reference to string with slash separated segment numbers (e.x. 1/2/3)
  // @param result - a reference to vector where results will be stored
  //--------------------------------------------------------------------------
  void parseSegmentNumbers(const std::string& string, std::vector<uint16_t>& result);

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);

  void checkParsingRetailerCodeError(bool isPRM, uint8_t rcqCount);

  void parseRetailerCode(const XMLChString& xmlValue, FareDisplayRequest* request, uint8_t& rcqCount);

  static const std::string BAD_GLOBAL;

  CarrierCode checkAndSwapCarrier(const CarrierCode& carrier);
};
} // End namespace tse
