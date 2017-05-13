//----------------------------------------------------------------------------
//
//      File: TaxOTAModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: Septemer, 2006
//      Authors: Hitha Alex
//
//  Copyright Sabre 2006
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
#include "Common/XMLChString.h"
#include "DataModel/PaxType.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/DataModelMap.h"

#include <map>
#include <stack>
#include <string>
#include <tuple>

namespace tse
{
class Agent;
class AirSeg;
class Fare;
class FarePath;
class Itin;
class TravelSeg;

using FareUsageMap = std::map<int, std::tuple<std::vector<TravelSeg*>, Fare*>>;
using AgentCache = std::map<LocCode, Agent*>; // agent reuse cache

namespace xray
{
class JsonMessage;
}

class TaxOTAModelMap : public DataModelMap
{
  friend class TaxOTAModelMapTest;

public:
  TaxOTAModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : DataModelMap(config, dataHandle, trx)
  {
  }

  virtual ~TaxOTAModelMap();

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
  // @function TaxOTAModelMap::classMapEntry
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
  void saveMapEntry(std::string& tagName) override {}
  void saveMapEntry(std::string& tagName, std::string& text) override;

  DataModelMap* getModelMap() { return this; }

private:
  struct Mapping
  {
    void (TaxOTAModelMap::*func)(const std::string&, const xercesc::Attributes&); // store function
    void (TaxOTAModelMap::*trxFunc)(const std::string&, const std::string&); // Trx interaction func
    MemberMap members; // associative members
  };

  TaxTrx* _taxTrx = nullptr;
  Itin* _itin = nullptr;
  AirSeg* _airSeg = nullptr;
  PaxType* _paxType = nullptr;
  FarePath* _farePath = nullptr;

  // Order passenger input
  uint16_t _paxInputOrder = 0;

  int _fareComponentNumber = 0;
  bool _fareCompDefinedAtLeastOnce = false;
  bool _fareCompNotDefinedAtLeastOnce = false;
  double _equivAmount = 0;
  MoneyAmount _totalNUCAmount = 0;
  FareUsageMap _fareUsageMap;
  AgentCache _agentCache;

  bool _intlItin = false;

  std::string _xrayMessageId;
  std::string _xrayConversationId;

  DateTime convertOTADate(const char* inDate);

  //--------------------------------------------------------------------------
  // @functions  TaxOTAModelMap::storeTaxInformation
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
  // @function TaxOTAModelMap::saveTaxInformation
  //
  // Description: This is the equivalent of a document end for a Tax
  //              request xml document.
  //
  // @param tagName
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxInformation(const std::string& tagName, const std::string& text);

  //--------------------------------------------------------------------------
  // @function TaxOTAModelMap::*
  //
  // Description: Convenience methods for mapping XML
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param tagName - value to be stored away
  // @param position - index mechanism for value assignment
  // @return void
  //--------------------------------------------------------------------------

  void storeProcessingOptionsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storePointOfSaleInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeSourceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTPAExtensionsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeUserInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeStationInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeBranchInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storePartitionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeSetAddressInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeServiceInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAAACityInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAgentSineInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeActionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeTransactionInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeItinerariesInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeItineraryInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeReservationItemsInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeItemInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeFlightSegmentInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeDepartureAirportInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeArrivalAirportInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeEquipmentInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeMarketingAirlineInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storeOperatingAirlineInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeAirFareInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void
  storePTCFareBreakdownInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storePassengerTypeInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeFareBasisCodeInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storePassengerFareInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeBaseFareInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeEquivalentFareInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeDiagInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void storeDynamicConfigOverride(const std::string& tagName, const xercesc::Attributes& attrs);
  //--------------------------------------------------------------------------
  // @function TaxOTAModelMap::*
  //
  // Description: Convenience methods for closing out a populated
  //              object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveProcessingOptionsInformation(const std::string& tagName, const std::string& text);
  void savePointOfSaleInformation(const std::string& tagName, const std::string& text);
  void saveSourceInformation(const std::string& tagName, const std::string& text);
  void saveTPAExtensionsInformation(const std::string& tagName, const std::string& text);
  void saveUserInformation(const std::string& tagName, const std::string& text);
  void saveStationInformation(const std::string& tagName, const std::string& text);
  void saveBranchInformation(const std::string& tagName, const std::string& text);
  void savePartitionInformation(const std::string& tagName, const std::string& text);
  void saveSetAddressInformation(const std::string& tagName, const std::string& text);
  void saveServiceInformation(const std::string& tagName, const std::string& text);
  void saveAAACityInformation(const std::string& tagName, const std::string& text);
  void saveAgentSineInformation(const std::string& tagName, const std::string& text);
  void saveActionInformation(const std::string& tagName, const std::string& text);
  void saveTransactionInformation(const std::string& tagName, const std::string& text);
  void saveItinerariesInformation(const std::string& tagName, const std::string& text);
  void saveItineraryInformation(const std::string& tagName, const std::string& text);
  void saveReservationItemsInformation(const std::string& tagName, const std::string& text);
  void saveItemInformation(const std::string& tagName, const std::string& text);
  void saveFlightSegmentInformation(const std::string& tagName, const std::string& text);
  void saveDepartureAirportInformation(const std::string& tagName, const std::string& text);
  void saveArrivalAirportInformation(const std::string& tagName, const std::string& text);
  void saveEquipmentInformation(const std::string& tagName, const std::string& text);
  void saveMarketingAirlineInformation(const std::string& tagName, const std::string& text);
  void saveOperatingAirlineInformation(const std::string& tagName, const std::string& text);
  void saveAirFareInformation(const std::string& tagName, const std::string& text);
  void savePTCFareBreakdownInformation(const std::string& tagName, const std::string& text);
  void savePassengerTypeInformation(const std::string& tagName, const std::string& text);
  void saveFareBasisCodeInformation(const std::string& tagName, const std::string& text);
  void savePassengerFareInformation(const std::string& tagName, const std::string& text);
  void saveBaseFareInformation(const std::string& tagName, const std::string& text);
  void saveEquivalentFareInformation(const std::string& tagName, const std::string& text);
  void saveDiagInformation(const std::string& tagName, const std::string& text);
  void saveFareUsage();
  void storeDiagParameters(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveDiagParameters(const std::string& tagName, const std::string& text);

  void storeDiagParameter(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveDiagParameter(const std::string& tagName, const std::string& text);
  void storeDiagnostic(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveDiagnostic(const std::string& tagName, const std::string& text);

  void storeHiddenStop(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveHiddenStop(const std::string& tagName, const std::string& text);

  void storeHiddenStops(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveHiddenStops(const std::string& tagName, const std::string& text);

  void storeFareBreakInfo(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveFareBreakInfo(const std::string& tagName, const std::string& text);

  void storeXrayInformation(const std::string& tagName, const xercesc::Attributes& attrs);
  void saveXrayInformation(const std::string& tagName, const std::string& text);

  std::string strUCase(tse::XMLChString& str);
  bool readXmlBool(tse::XMLChString& str);
  int getVersionPos(int pos);
  void addArunkSegmentForOTA(TaxTrx* trx,
                             Itin& itin,
                             const TravelSeg& travelSeg,
                             bool& isMissingArunkForOTA);
  void processMissingArunkSegForOTA(TaxTrx* trx,
                                    Itin* itin,
                                    const TravelSeg* travelSeg,
                                    bool& isMissingArunkForOTA);
  bool needToAddArunkSegment(TaxTrx* trx, Itin& itin, const TravelSeg& travelSeg);
  void setBoardCity(AirSeg& airSeg);
  void setOffCity(AirSeg& airSeg);
  void agentSetUp(Agent* agent);
  Agent* getAgent(LocCode& loc);
};

} // End namespace tse

