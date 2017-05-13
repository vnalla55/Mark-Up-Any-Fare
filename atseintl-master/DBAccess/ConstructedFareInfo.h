//-------------------------------------------------------------------
//
//  File:        ConstructedFareInfo.h
//  Created:     Feb 14, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents common data and members of
//               one ATPCO or SMF constructed fare
//
//  Copyright Sabre 2005
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

#include "DBAccess/ConstructedFareInfoFactory.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Flattenizable.h"

#include <memory>
#include <vector>

namespace tse
{
class ConstructedFareInfo;

namespace flattenizer
{
template <class ConstructedFareInfo>
inline void
flatten(Flattenizable::Archive& archive,
        const std::vector<std::shared_ptr<ConstructedFareInfo>>& v);
template <class ConstructedFareInfo>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<ConstructedFareInfo>>& v);
template <class ConstructedFareInfo>
inline void
calcmem(Flattenizable::Archive& archive,
        const std::vector<std::shared_ptr<ConstructedFareInfo>>& v);
}
}

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse
{

class ConstructedFareInfo
{
public:
  // public types
  // ====== =====

  enum ConstructionType
  {
    SINGLE_ORIGIN = 1,
    SINGLE_DESTINATION,
    DOUBLE_ENDED
  };

  // construction/destruction & assignment
  // ======================== = ==========

  ConstructedFareInfo();
  virtual ~ConstructedFareInfo();
  virtual const eConstructedFareInfoType objectType() const { return eConstructedFareInfo; }

  // main interface
  // ==== =========

  /**
   * This methods obtains a new ConstructedFareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual ConstructedFareInfo* clone(DataHandle& dataHandle) const;

  /**
   * This methods populates a given ConstructedFareInfo object to be
   * 'equal' to the current object
   *
   * @param cloneObj - object to populate
   */

  virtual void clone(ConstructedFareInfo& cloneObj) const;

  // constructed fare common data accessors
  // =========== ==== ====== ==== =========

  ConstructionType& constructionType() { return _constructionType; }
  const ConstructionType constructionType() const { return _constructionType; }

  LocCode& gateway1() { return _gateway1; }
  const LocCode& gateway1() const { return _gateway1; }

  LocCode& gateway2() { return _gateway2; }
  const LocCode& gateway2() const { return _gateway2; }

  MoneyAmount& specifiedFareAmount() { return _specifiedFareAmount; }
  const MoneyAmount specifiedFareAmount() const { return _specifiedFareAmount; }

  MoneyAmount& constructedNucAmount() { return _constructedNucAmount; }
  const MoneyAmount constructedNucAmount() const { return _constructedNucAmount; }

  MoneyAmount& constructedSecondNucAmount() { return _constructedSecondNucAmount; }
  const MoneyAmount constructedSecondNucAmount() const { return _constructedSecondNucAmount; }

  const unsigned int atpFareClassPriority() const { return _atpFareClassPriority; }
  unsigned int& atpFareClassPriority() { return _atpFareClassPriority; }

  FareInfo& fareInfo() { return *_fareInfo; }
  const FareInfo& fareInfo() const { return *_fareInfo; }

  // origin add-on data accessors
  // ====== ====== ==== =========

  AddonZone& origAddonZone() { return _origAddonZone; }
  const AddonZone origAddonZone() const { return _origAddonZone; }

  Footnote& origAddonFootNote1() { return _origAddonFootNote1; }
  const Footnote& origAddonFootNote1() const { return _origAddonFootNote1; }

  Footnote& origAddonFootNote2() { return _origAddonFootNote2; }
  const Footnote& origAddonFootNote2() const { return _origAddonFootNote2; }

  FareClassCode& origAddonFareClass() { return _origAddonFareClass; }
  const FareClassCode& origAddonFareClass() const { return _origAddonFareClass; }

  TariffNumber& origAddonTariff() { return _origAddonTariff; }
  const TariffNumber origAddonTariff() const { return _origAddonTariff; }

  RoutingNumber& origAddonRouting() { return _origAddonRouting; }
  const RoutingNumber& origAddonRouting() const { return _origAddonRouting; }

  MoneyAmount& origAddonAmount() { return _origAddonAmount; }
  const MoneyAmount origAddonAmount() const { return _origAddonAmount; }

  CurrencyCode& origAddonCurrency()
  {
    return _origAddonCurrency;
  };
  const CurrencyCode& origAddonCurrency() const
  {
    return _origAddonCurrency;
  };

  Indicator& origAddonOWRT() { return _origAddonOWRT; }
  const Indicator origAddonOWRT() const { return _origAddonOWRT; }

  // destination add-on data accessors
  // =========== ====== ==== =========

  AddonZone& destAddonZone() { return _destAddonZone; }
  const AddonZone destAddonZone() const { return _destAddonZone; }

  Footnote& destAddonFootNote1() { return _destAddonFootNote1; }
  const Footnote& destAddonFootNote1() const { return _destAddonFootNote1; }

  Footnote& destAddonFootNote2() { return _destAddonFootNote2; }
  const Footnote& destAddonFootNote2() const { return _destAddonFootNote2; }

  FareClassCode& destAddonFareClass() { return _destAddonFareClass; }
  const FareClassCode& destAddonFareClass() const { return _destAddonFareClass; }

  TariffNumber& destAddonTariff() { return _destAddonTariff; }
  const TariffNumber destAddonTariff() const { return _destAddonTariff; }

  RoutingNumber& destAddonRouting() { return _destAddonRouting; }
  const RoutingNumber& destAddonRouting() const { return _destAddonRouting; }

  MoneyAmount& destAddonAmount() { return _destAddonAmount; }
  const MoneyAmount destAddonAmount() const { return _destAddonAmount; }

  CurrencyCode& destAddonCurrency()
  {
    return _destAddonCurrency;
  };
  const CurrencyCode& destAddonCurrency() const
  {
    return _destAddonCurrency;
  };

  Indicator& destAddonOWRT() { return _destAddonOWRT; }
  const Indicator destAddonOWRT() const { return _destAddonOWRT; }

  virtual bool operator==(const ConstructedFareInfo& rhs) const
  {
    return ((_fareInfo == rhs._fareInfo ||
             (_fareInfo != nullptr && rhs._fareInfo != nullptr && *_fareInfo == *rhs._fareInfo)) &&
            (_constructionType == rhs._constructionType) && (_gateway1 == rhs._gateway1) &&
            (_gateway2 == rhs._gateway2) && (_specifiedFareAmount == rhs._specifiedFareAmount) &&
            (_constructedNucAmount == rhs._constructedNucAmount) &&
            (_constructedSecondNucAmount == rhs._constructedSecondNucAmount) &&
            (_atpFareClassPriority == rhs._atpFareClassPriority) &&
            (_origAddonZone == rhs._origAddonZone) &&
            (_origAddonFootNote1 == rhs._origAddonFootNote1) &&
            (_origAddonFootNote2 == rhs._origAddonFootNote2) &&
            (_origAddonFareClass == rhs._origAddonFareClass) &&
            (_origAddonTariff == rhs._origAddonTariff) &&
            (_origAddonRouting == rhs._origAddonRouting) &&
            (_origAddonAmount == rhs._origAddonAmount) &&
            (_origAddonCurrency == rhs._origAddonCurrency) &&
            (_destAddonZone == rhs._destAddonZone) &&
            (_destAddonFootNote1 == rhs._destAddonFootNote1) &&
            (_destAddonFootNote2 == rhs._destAddonFootNote2) &&
            (_destAddonFareClass == rhs._destAddonFareClass) &&
            (_destAddonTariff == rhs._destAddonTariff) &&
            (_destAddonRouting == rhs._destAddonRouting) &&
            (_destAddonAmount == rhs._destAddonAmount) &&
            (_destAddonCurrency == rhs._destAddonCurrency) &&
            (_origAddonOWRT == rhs._origAddonOWRT) && (_destAddonOWRT == rhs._destAddonOWRT));
  }

  static void dummyData(ConstructedFareInfo& obj) { obj.dummyData(); }

protected:
  // constructed fare common data
  // =========== ==== ====== ====

  ConstructionType _constructionType;
  LocCode _gateway1;
  LocCode _gateway2;
  MoneyAmount _specifiedFareAmount;
  MoneyAmount _constructedNucAmount;
  MoneyAmount _constructedSecondNucAmount;
  FareInfo* _fareInfo;
  unsigned int _atpFareClassPriority;

  // origin add-on data
  // ====== ====== ====

  AddonZone _origAddonZone;
  Footnote _origAddonFootNote1;
  Footnote _origAddonFootNote2;
  FareClassCode _origAddonFareClass;
  TariffNumber _origAddonTariff;
  RoutingNumber _origAddonRouting;
  MoneyAmount _origAddonAmount;
  CurrencyCode _origAddonCurrency;

  // destination add-on data
  // =========== ====== ====

  AddonZone _destAddonZone;
  Footnote _destAddonFootNote1;
  Footnote _destAddonFootNote2;
  FareClassCode _destAddonFareClass;
  TariffNumber _destAddonTariff;
  RoutingNumber _destAddonRouting;
  MoneyAmount _destAddonAmount;
  CurrencyCode _destAddonCurrency;
  Indicator _origAddonOWRT;
  Indicator _destAddonOWRT;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE_ENUM(archive, _constructionType);
    FLATTENIZE(archive, _gateway1);
    FLATTENIZE(archive, _gateway2);
    FLATTENIZE(archive, _specifiedFareAmount);
    FLATTENIZE(archive, _constructedNucAmount);
    FLATTENIZE(archive, _constructedSecondNucAmount);
    FLATTENIZE(archive, *_fareInfo);
    FLATTENIZE(archive, _atpFareClassPriority);
    FLATTENIZE(archive, _origAddonZone);
    FLATTENIZE(archive, _origAddonFootNote1);
    FLATTENIZE(archive, _origAddonFootNote2);
    FLATTENIZE(archive, _origAddonFareClass);
    FLATTENIZE(archive, _origAddonTariff);
    FLATTENIZE(archive, _origAddonRouting);
    FLATTENIZE(archive, _origAddonAmount);
    FLATTENIZE(archive, _origAddonCurrency);
    FLATTENIZE(archive, _destAddonZone);
    FLATTENIZE(archive, _destAddonFootNote1);
    FLATTENIZE(archive, _destAddonFootNote2);
    FLATTENIZE(archive, _destAddonFareClass);
    FLATTENIZE(archive, _destAddonTariff);
    FLATTENIZE(archive, _destAddonRouting);
    FLATTENIZE(archive, _destAddonAmount);
    FLATTENIZE(archive, _destAddonCurrency);
    FLATTENIZE(archive, _origAddonOWRT);
    FLATTENIZE(archive, _destAddonOWRT);
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const ConstructedFareInfo& obj)
  {
    obj.dumpObject(os);
    return os;
  }

protected:
  ConstructedFareInfo(bool init);
  void dumpConstructedFareInfo(std::ostream& os) const;
  virtual void dumpObject(std::ostream& os) const;
  virtual void dummyData();

private:
  void initialize();

  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  ConstructedFareInfo(ConstructedFareInfo& rhs);
  ConstructedFareInfo& operator=(const ConstructedFareInfo& rhs);

}; // End of class ConstructedFareInfo

} // End namespace tse

namespace tse
{
namespace flattenizer
{
template <>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<ConstructedFareInfo>>& v)
{
  size_t item_counter(0);
  archive.append(v.size());
  for (const auto& elem : v)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    archive.append(static_cast<size_t>(elem->objectType()));
    flatten(archive, *elem);
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}

template <>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<ConstructedFareInfo>>& v)
{
  size_t item_counter(0);
  size_t sz(0);
  archive.extract(sz);
  v.clear();

  while (sz--)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    size_t type;
    archive.extract(type);

    ConstructedFareInfo* info(
        ConstructedFareInfoFactory::create(static_cast<eConstructedFareInfoType>(type)));

    unflatten(archive, *info);
    v.push_back(std::shared_ptr<ConstructedFareInfo>(info));
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}

template <>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<ConstructedFareInfo>>& v)
{
  size_t item_counter(0);
  archive.addSize(sizeof(size_t));
  for (const auto& elem : v)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    archive.addSize(sizeof(size_t));
    calcmem(archive, *elem);
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}
} // namespace flattenizer
} // namespace tse
