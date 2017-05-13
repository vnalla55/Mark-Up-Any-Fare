#pragma once

#include "DBAccess/FareFocusDaytimeApplDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusDaytimeApplInfo
{
 public:
  FareFocusDaytimeApplInfo()
    : _dayTimeApplItemNo(0)
  {
  }

  ~FareFocusDaytimeApplInfo()
  {
    for (std::vector<FareFocusDaytimeApplDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& dayTimeApplItemNo() { return _dayTimeApplItemNo; }
  const uint64_t& dayTimeApplItemNo() const { return _dayTimeApplItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareFocusDaytimeApplDetailInfo*>& details() { return _details; }
  const std::vector<FareFocusDaytimeApplDetailInfo*>& details() const { return _details; }

  bool operator==(const FareFocusDaytimeApplInfo& rhs) const
  {
    bool result(_dayTimeApplItemNo == rhs._dayTimeApplItemNo
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

  static void dummyData(FareFocusDaytimeApplInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._dayTimeApplItemNo = 111222;
    obj._createDate = current;
    obj._expireDate = current;
    FareFocusDaytimeApplDetailInfo* detail(new FareFocusDaytimeApplDetailInfo);
    FareFocusDaytimeApplDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _dayTimeApplItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:
  uint64_t _dayTimeApplItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareFocusDaytimeApplDetailInfo*> _details;
};

}// tse

