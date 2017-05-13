// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputPointOfSale.h"
#include <log4cxx/helpers/objectptr.h>
#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class Itin;
class ServiceFeesGroup;
class TravelSeg;
class OCFees;
}

namespace tax
{
class InputRequest;
class InputGeo;
class InputItin;

class OptionalServicesBuilder
{
  friend class OptionalServicesBuilderTest;

  typedef std::map<const tse::TravelSeg*, std::vector<tax::InputGeo*> > TravelSegGeoItems;
  typedef std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > OptionalServicesRefs;
  static const std::string ATPOCINC_NAME;

  class OCFeesSegsChecker
  {
    std::set<const tse::OCFees::OCFeesSeg* > _elements;
  public:
    bool
    shouldAdd(const tse::OCFees::OCFeesSeg* ocFeesSeg)
    {
      auto result = _elements.insert(ocFeesSeg);
      return result.second;
    }
  };

  class OCFeesChecker // to remove with monetaryDiscountTaxesForAllSegments
  {
    std::set<const tse::OCFees* > _elements;
  public:
    bool
    shouldAdd(const tse::OCFees* ocFees)
    {
      auto result = _elements.insert(ocFees);
      return result.second;
    }
  };

public:
  OptionalServicesBuilder(tse::PricingTrx& trx,
                          InputRequest& inputRequest,
                          tax::InputItin& itin,
                          TravelSegGeoItems& items,
                          const tse::FarePath& tseFarePath,
                          OptionalServicesRefs& optionalServicesMapping,
                          std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> >& ocTaxInclIndMap);
  virtual ~OptionalServicesBuilder();

  void addOptionalServices();
  void addBaggage();

  static log4cxx::LoggerPtr _logger;

private:
  OptionalServicesBuilder(const OptionalServicesBuilder& builder);
  OptionalServicesBuilder& operator=(const OptionalServicesBuilder& builder);

  void buildMappings(const tse::OCFees& ocFees);
  void addGeoPathMappings();
  void addToTaxInclIndMap(tse::OCFees& ocFees, int index);
  void addToTaxInclIndMap(tse::OCFees& ocFees, size_t segIndex, int index);
  void addOCFees(tse::OCFees& ocFees);
  int addOCFees(tse::OCFees& ocFees, size_t segIndex);
  void addServiceFeesGroup(tse::ServiceFeesGroup& group);
  type::OptionalServiceTag
  getOptionalServiceTag(const tse::Indicator& fltTktMerchInd) const;
  bool checkTaxInclInd(const tse::OCFees& ocFees) const;

  tse::PricingTrx& _trx;
  InputRequest& _inputRequest;
  InputItin& _itin;
  TravelSegGeoItems& _items;
  const tse::FarePath& _tseFarePath;
  OptionalServicesRefs& _optionalServicesMapping;
  std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> >& _ocTaxInclIndMap;
  bool _geoPathMappingsAdded;
  OCFeesChecker _ocFeesChecker;
  OCFeesSegsChecker _ocFeesSegsChecker; // to remove with monetaryDiscountTaxesForAllSegments
};

} // end of tax
