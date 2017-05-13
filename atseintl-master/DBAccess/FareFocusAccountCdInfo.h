#pragma once

#include "DBAccess/FareFocusAccountCdDetailInfo.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusAccountCdInfo
{
 public:
  FareFocusAccountCdInfo()
    : _accountCdItemNo(0)
  {
  }

  ~FareFocusAccountCdInfo()
  {
    for (std::vector<FareFocusAccountCdDetailInfo*>::const_iterator it(_details.begin()),
                                                                   itend(_details.end());
         it != itend;
         ++it)
    {
      delete *it;
    }
  }

  uint64_t& accountCdItemNo() { return _accountCdItemNo; }
  uint64_t accountCdItemNo() const { return _accountCdItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareFocusAccountCdDetailInfo*>& details() { return _details; }
  const std::vector<FareFocusAccountCdDetailInfo*>& details() const { return _details; }

  bool operator==(const FareFocusAccountCdInfo& rhs) const
  {
    bool result(_accountCdItemNo == rhs._accountCdItemNo
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

  static void dummyData(FareFocusAccountCdInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._accountCdItemNo = 1111111122223333;
    obj._createDate = current;
    obj._expireDate = current;
    FareFocusAccountCdDetailInfo* detail(new FareFocusAccountCdDetailInfo);
    FareFocusAccountCdDetailInfo::dummyData(*detail);
    obj._details.push_back(detail);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _accountCdItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _details);
  }

 private:
  uint64_t _accountCdItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareFocusAccountCdDetailInfo*> _details;
};

}// tse

