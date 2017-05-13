//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#pragma once

#include "DataModel/FreqFlyerStatusData.h"
#include "DataModel/FrequentFlyerRequest.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Service/Service.h"
#include "Util/FlatMap.h"

#include <locale>

namespace tse
{
struct Compare
{
  enum Type
  { FIRST_DIGIT = 1,
    SECOND_DIGIT = 2,
    TWO_ALPHA = 3 };

  Type determineType(const CarrierCode cxr) const
  {
    if (std::isdigit(cxr[1]))
      return SECOND_DIGIT;
    else if (std::isdigit(cxr[0]))
      return FIRST_DIGIT;
    return TWO_ALPHA;
  }

  bool operator()(const CarrierCode cxrA, const CarrierCode cxrB) const
  {
    Type typeA = determineType(cxrA);
    Type typeB = determineType(cxrB);

    if (typeA > typeB)
      return false;
    return typeA < typeB || cxrA < cxrB;
  }
};

struct FreqFlyerStatus;

class FrequentFlyerTrx : public Trx
{
public:
  using StatusList = std::vector<FreqFlyerStatusData>;
  using CarriersData = FlatMap<CarrierCode, StatusList, Compare>;
  void setRequest(FrequentFlyerRequest* request) { _request = request; }

  void setCxrs(const std::set<CarrierCode>& cxrs)
  {
    _cxrs.clear();
    for (const CarrierCode cxr : cxrs)
      _cxrs.insert(_cxrs.cend(), std::make_pair(cxr, StatusList()));
  }

  void setCxrData(const CarrierCode cxr, StatusList&& list)
  {
    _cxrs[cxr] = std::forward<StatusList>(list);
  }

  const CarriersData& getCxrs() const { return _cxrs; }

  bool process(Service& srv) override { return srv.process(*this); }

private:
  FrequentFlyerRequest* _request;
  CarriersData _cxrs;
};
} // tse namespace
