//----------------------------------------------------------------------------
//
//      File: DataModelMap.h
//      Description: Create and interpret Data Model mappings
//      Created: March 22, 2004
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
#include "Common/TseConsts.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"

#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{
class PricingTrx;
class PricingOptions;
class Itin;

class DataModelMap
{
public:
  DataModelMap(const DataModelMap& dataModelMap) = delete;
  DataModelMap& operator=(const DataModelMap& dataModelMap) = delete;

  /* These methods are made public so that they can be called from
     a common ContentHandler, if any body takes the pain one day to
     refactor to have a common ContentHandler for all types of services.
     Right now TaxContentHandler is common for V2 and OTA requests
     and hence need these methods here.*/

  //--------------------------------------------------------------------------
  // @function DataModelMap::classMapEntry
  //
  // Description: Using the current mapping start any processing and store
  //              attribute values to trx object
  //
  // @param tagName - Element Name
  // @param atts - Element Arributes
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool classMapEntry(std::string& tagName, const xercesc::Attributes& atts) { return true; }

  //--------------------------------------------------------------------------
  // @function DataModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - Element name
  // @param text - Element data
  //--------------------------------------------------------------------------
  virtual void saveMapEntry(std::string& tagName) {}
  virtual void saveMapEntry(std::string& tagName, std::string& text) {}

  static std::string purgeBookingCodeOfNonAlpha(const std::string& bkc);

protected:
  DataModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx)
    : _config(config), _dataHandle(dataHandle), _trx(trx)
  {
  }

  virtual ~DataModelMap() = default;

  //--------------------------------------------------------------------------
  // @function DataModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  virtual bool createMap() = 0;

  ConfigMan& _config;

  DataHandle& _dataHandle;
  Trx*& _trx;

  // Basic mapping structure
  using MemberMap = std::map<unsigned int, int>;

  //--------------------------------------------------------------------------
  // @function DataModelMap::SDBMHash
  //
  // Description: Implementation of one of more popular hashing algorithms
  //
  // @param strValue - value to be hashed
  // @return unsigned int
  //--------------------------------------------------------------------------
  unsigned int SDBMHash(const std::string& strValue);
  unsigned int SDBMHash(const char* chrValue);

  // DataModel class mappings
  using ClassMap = std::map<unsigned int, void*>;
  using ClassMapIter = std::map<unsigned int, void*>::iterator;
  ClassMap _classMap;

  std::stack<void*> _currentMapEntryStack;
  void* _currentMapEntry = nullptr;

  //--------------------------------------------------------------------------
  // @function DataModelMap::convertDate
  //
  // Description: Take a char pointer to a date of form YYYY-MM-DD and
  //              return a DateTime
  //
  // @param inDate - pointer to a string of form YYYY-MM-DD
  // @return DateTime - success or failure
  //--------------------------------------------------------------------------
  const DateTime convertDate(const char* inDate);

  //--------------------------------------------------------------------------
  // @function DataModelMap::convertDateDDMMMYY
  //
  // Description: Take a char pointer to a date of form DDMMMYY and
  //              return a DateTime
  //
  // @param inDate - pointer to a string of form YYYY-MM-DD
  // @return DateTime - success or failure
  //--------------------------------------------------------------------------
  const DateTime convertDateDDMMMYY(const char* inDate);

  //--------------------------------------------------------------------------
  // @function DataModelMap::toUpper
  //
  // Description: Base upper case converter
  //
  // @param toConvert - string to be converted
  //--------------------------------------------------------------------------
  std::string& toUpper(std::string& toConvert) const;

  //--------------------------------------------------------------------------
  // @method checkFuturePricingCurrency
  //
  // Description: If a future date is used for the pricing request
  //              the nations currency must be checked to see if
  //              a new currency must be used.
  //
  // @param PricingTrx         - pricingTrx pointer
  // @param Itin*              - itinerary pointer
  // @param PricingOptions*    - options pointer
  //
  // @return void
  //--------------------------------------------------------------------------
  void checkFuturePricingCurrency(PricingTrx* pricingTrx, Itin* itin, PricingOptions* options);

  void handleDynamicConfig(MemberMap& members, const xercesc::Attributes& attrs);
};

} // End namespace tse

