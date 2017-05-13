#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{
class FareTypeDesignator final
{
  friend class FareTypeDesignatorTest;

public:
  enum FareTypeDesignatorNew
  { FTD_UNKNOWN = 0,
    FTD_PREMIUM_FIRST = 10,
    FTD_FIRST = 15,
    FTD_PREMIUM_BUSINESS = 20,
    FTD_BUSINESS = 25,
    FTD_PREMIUM_ECONOMY = 30,
    FTD_ECONOMY = 35,
    FTD_EXCURSION = 40,
    FTD_ONEWAY_ADVANCE_PURCHASE = 50,
    FTD_ROUNDTRIP_ADVANCE_PURCHASE = 55,
    FTD_ONEWAY_INSTANT_PURCHASE = 60,
    FTD_ROUNDTRIP_INSTANT_PURCHASE = 65,
    FTD_SPECIAL = 70,
    FTD_ADDON = 80,
    FTD_PROMOTIONAL = 90 };
  // ctors
  bool operator==(const FareTypeDesignator& src) const
  {
    return _fareTypeDesignator == src._fareTypeDesignator;
  }
  bool operator!=(const FareTypeDesignator& src) const { return !((*this) == src); }
  // accessors
  int fareTypeDesig() const { return _fareTypeDesignator; }
  //
  bool isFTDUnknown() const { return _fareTypeDesignator == FTD_UNKNOWN; }
  bool isFTDEconomy() const { return _fareTypeDesignator == FTD_ECONOMY; }
  bool isFTDAddon() const { return _fareTypeDesignator == FTD_ADDON; }
  bool isFTDPromotional() const { return _fareTypeDesignator == FTD_PROMOTIONAL; }
  ///////////////////////////////
  void setFareTypeDesignator(int ftd);

  void setFTDUnknown() { _fareTypeDesignator = FTD_UNKNOWN; }
  void setFTDBusiness() { _fareTypeDesignator = FTD_BUSINESS; }
  void setFTDEconomy() { _fareTypeDesignator = FTD_ECONOMY; }
  void setFTDPromotional() { _fareTypeDesignator = FTD_PROMOTIONAL; }

  // serialization

  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _fareTypeDesignator); }

  static void dummyData(FareTypeDesignator& fd) { fd._fareTypeDesignator = FTD_FIRST; }
  ///////////////

  friend inline std::ostream& operator<<(std::ostream& os, const FareTypeDesignator& obj)
  {
    return os << "[" << obj._fareTypeDesignator << "]";
  }

private:
  int _fareTypeDesignator = FareTypeDesignatorNew::FTD_UNKNOWN;
};
} // namespace
