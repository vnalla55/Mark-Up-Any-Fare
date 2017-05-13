#pragma once

#include "DBAccess/FareFocusRoutingDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusRoutingInfo
{
 public:
  FareFocusRoutingInfo()
    : _routingItemNo(0)
  {
  }

  ~FareFocusRoutingInfo()
  {
    for (std::vector<FareFocusRoutingDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& routingItemNo() { return _routingItemNo; }
  uint64_t routingItemNo() const { return _routingItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareFocusRoutingDetailInfo*>& details() { return _details; }
  const std::vector<FareFocusRoutingDetailInfo*>& details() const { return _details; }

  bool operator==(const FareFocusRoutingInfo& rhs) const
  {
    bool result(_routingItemNo == rhs._routingItemNo
                && _createDate == rhs._createDate
                && _expireDate == rhs._expireDate
                && _details.size() == rhs._details.size());
    if (result)
    {
      for (size_t i = 0; i <  _details.size(); ++i)
      {
        result = result && *_details[i] == *rhs._details[i];
        if (!result)
        {
          break;
        }
      }
    }
    return result;
  }

  static void dummyData(FareFocusRoutingInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._routingItemNo = 1111111122223333;
    obj._createDate = current;
    obj._expireDate = current;
    FareFocusRoutingDetailInfo* detail(new FareFocusRoutingDetailInfo);
    FareFocusRoutingDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _routingItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:
  uint64_t _routingItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareFocusRoutingDetailInfo*> _details;
};

}// tse

