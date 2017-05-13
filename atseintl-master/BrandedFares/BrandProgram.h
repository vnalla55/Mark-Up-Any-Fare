//-------------------------------------------------------------------
//
//  File:        BrandProgram.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class BrandInfo;
class Diag894Collector;
class FareMarket;

class BrandProgram : boost::noncopyable
{
public:
  using LegToDirectionMap = std::map<uint16_t, Direction>;
  using OnDPair = std::pair<LocCode, LocCode>;

  using BrandsData = std::vector<BrandInfo*>;
  using AccountCodes = std::vector<std::string>;

  ProgramID& programID() { return _programID; }
  const ProgramID& programID() const { return _programID; }

  ProgramCode& programCode() { return _programCode; }
  const ProgramCode& programCode() const { return _programCode; }

  ProgramName& programName() { return _programName; }
  const ProgramName& programName() const { return _programName; }

  std::string& programDescription() { return _programDescription; }
  const std::string& programDescription() const { return _programDescription; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  std::vector<PaxTypeCode>& passengerType() { return _paxTypeCode; }
  const std::vector<PaxTypeCode>& passengerType() const { return _paxTypeCode; }

  AlphaCode& direction() { return _direction; }
  const AlphaCode& direction() const { return _direction; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  GlobalDirection globalDirection() const { return _globalDirection; }

  SequenceNumber& sequenceNo() { return _sequenceNo; }
  const SequenceNumber sequenceNo() const { return _sequenceNo; }

  DateTime& effectiveDate() { return _effectiveDate; }
  const DateTime& effectiveDate() const { return _effectiveDate; }

  DateTime& discontinueDate() { return _discontinueDate; }
  const DateTime& discontinueDate() const { return _discontinueDate; }

  BrandsData& brandsData() { return _brandsData; }
  const BrandsData& brandsData() const { return _brandsData; }

  AccountCodes& accountCodes() { return _accountCodes; }
  const AccountCodes& accountCodes() const { return _accountCodes; }

  LocCode& originLoc() { return _originLoc; }
  const LocCode& originLoc() const { return _originLoc; }

  std::set<LocCode>& originsRequested() { return _originsRequested; }
  const std::set<LocCode>& originsRequested() const { return _originsRequested; }

  std::set<LocCode>& destinationsRequested() { return _destinationsRequested; }
  const std::set<LocCode>& destinationsRequested() const { return _destinationsRequested; }

  SystemCode& systemCode() { return _systemCode; }
  const SystemCode& systemCode() const { return _systemCode; }

  BrandSource& dataSource() { return _dataSource; }
  const BrandSource& dataSource() const { return _dataSource; }

  LegToDirectionMap& getMutableLegToDirectionMap() { return _legToDirectionMap; }
  const LegToDirectionMap& getLegToDirectionMap() const { return _legToDirectionMap; }

  std::set<OnDPair>& getMutableAvailabilityOnOnD() { return _availabilityOnOnD; }
  const std::set<OnDPair>& getAvailabilityOnOnD() const { return _availabilityOnOnD; }

  //calculates program direction for given fare market. returns true if calculation
  //was possible, false otherwise. in multi-itinerary request it is possible
  //that program and fare market have nothing in common (program from one itin and
  //fm from the other).
  bool calculateDirectionality(const FareMarket& fareMarket,
                               Direction& direction,
                               Diag894Collector* diag894 = nullptr) const;
private:
  ProgramID _programID;
  ProgramCode _programCode;
  ProgramName _programName; // Program Text in DB
  std::string _programDescription;
  VendorCode _vendorCode;
  std::vector<PaxTypeCode> _paxTypeCode;
  GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
  AlphaCode _direction;
  SequenceNumber _sequenceNo = 0;
  DateTime _effectiveDate;
  DateTime _discontinueDate;
  BrandsData _brandsData;
  AccountCodes _accountCodes;
  LocCode _originLoc;
  std::set<LocCode> _originsRequested;
  std::set<LocCode> _destinationsRequested;
  SystemCode _systemCode = ' ';
  BrandSource _dataSource = BrandSource::BRAND_SOURCE_S8;
  LegToDirectionMap _legToDirectionMap;
  std::set<OnDPair> _availabilityOnOnD;
};

} // tse

