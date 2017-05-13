//-------------------------------------------------------------------
//
//  File:        BrandDataRetriever.h
//  Created:     January 11, 2007
//  Authors:     Marco Cartolano
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

#include "BrandingService/BrandResponseItem.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/BrandedCarrier.h"
#include "DBAccess/Loc.h"
#include "FareDisplay/FDConsts.h"


namespace tse
{

/**
*   @class BrandDataRetriever
*   Responsible for collecting all brand data from the database and translate them
*   into a booking code/brand code map
*/

class Group;
class BrandedFareApp;
class FareDisplayTrx;

class BrandDataRetriever
{
  friend class BrandDataRetrieverTest;

public:
  BrandDataRetriever(FareDisplayTrx& trx);
  virtual ~BrandDataRetriever();

  /**
   * Invokes the main data handle call to collect information from the db.
   */
  void getBrandData(std::vector<Group*>& groups);

private:
  Indicator _userApplType;
  UserApplCode _userAppl;
  CarrierCode _carrier;
  FareDisplayTrx& _trx;

  std::vector<BrandedFareApp*> _brandedFareAppData;
  std::vector<BrandedFareApp*> _brandedFareAppMatches;
  std::map<BookingCode, BrandCode> _brandMap;

  std::vector<BrandResponseItem*> _brandResponseItemVec;
  std::map<std::string, BrandCode> _brandIncludeFareBasisCodeMap;
  std::multimap<std::string, BrandCode> _brandExcludeFareBasisCodeMMap;

  /**
   * Retrieves brand data from database and also provides one single interface
   * for all dbaccess call.
   */
  bool
  getData(const Indicator& userApplType, const UserApplCode& userAppl, const CarrierCode& carrier);

  /**
   * Selects data records based on directionality and geo location.
   */
  bool selectData();

  /**
   * Matches the data record directionality and geo location against the
   * origin and destination of the transaction.
   */
  bool isMatchLocation(const BrandedFareApp* const& brandedFareApp,
                       const Loc& origin,
                       const Loc& destination);

  /**
   * Initializes brand group and group header.
   */
  void initializeGroup(std::vector<Group*>& groups);

  /**
   * Determine the brand of each valid fare.
   */
  void determineBrands();

  //====================================================================
  // Methods to support Airline Branding Service
  //====================================================================

  /**
   * Retrieves brand data calling brand service
   */
  bool getDataFromService();

  /**
   * Selects data returned by brand service
   */
  bool selectDataFromService();

  /**
   * Initializes brand group and group header.
   */
  void initializeGroupFromService(std::vector<Group*>& groups);

  /**
   * Determine the brand of each valid fare.
   */
  void determineBrandsFromService();

  /**
   * Matches the data record directionality and geo location against the
   * origin and destination of the transaction.
   */
  bool isMatchFareBasisCode(const std::string& brandFareBasisCode,
                            const std::string& ptfFareBasisCode,
                            bool& exactMatch);

  /**
   * Send 'request' to brand service and return 'response'
   */

  std::string callBrandingService(std::string request);
};

} // namespace tse

