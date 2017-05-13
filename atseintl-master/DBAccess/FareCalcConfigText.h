#pragma once

#include "DBAccess/Flattenizable.h"

#include <map>
#include <string>

namespace tse
{

class FareCalcConfigText
{
public:
  friend class QueryGetMsgText;

  FareCalcConfigText() {}
  ~FareCalcConfigText() {}

  enum TextAppl
  {
    WPA_RO_INDICATOR = 1,
    WPA_NO_MATCH_NO_FARE,
    WPA_NO_MATCH_VERIFY_BOOKING_CLASS,
    WPA_NO_MATCH_REBOOK,
    WPA_NO_MATCH_BOOKING_CLASS,
    WP_RO_INDICATOR,
    WP_NO_MATCH_NO_FARE,
    WP_NO_MATCH_VERIFY_BOOKING_CLASS,
    WP_NO_MATCH_REBOOK,
    WP_NO_MATCH_BOOKING_CLASS,
    MAX_TEXT_APPL
  };

  typedef std::map<TextAppl, std::string> FCCTextMap;

  bool getApplMsg(TextAppl appl, std::string& msg) const
  {
    FCCTextMap::const_iterator i = _fccTextMap.find(appl);
    bool ret = (i != _fccTextMap.end());
    if (ret)
    {
      msg = i->second;
    }
    return ret;
  }

  FCCTextMap& fccTextMap() { return _fccTextMap; }

  bool operator==(const FareCalcConfigText& rhs) const
  {
    return ((_fccTextMap == rhs._fccTextMap));
  }

  static void dummyData(FareCalcConfigText& obj)
  {
    obj._fccTextMap[WPA_RO_INDICATOR] = "aaaaaaaa";
    obj._fccTextMap[WP_RO_INDICATOR] = "bbbbbbbb";
  }

private:
  FCCTextMap _fccTextMap;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE_ENUM_KEY_MAP(archive, _fccTextMap);
  }

private:
};

} // namespace tse

