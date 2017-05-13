//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Rules/IObserver.h"

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <memory>

namespace tse
{
template <typename ObserverType>
class SubjectObserved;
class FareUsage;
class RuleItemInfo;
class DataHandle;

enum ObserverType
{ ADVANCE_TICKETING_SFR,
  ADVANCE_RESERVATION_SFR,
  MIN_STAY,
  MIN_STAY_SFR,
  MAX_STAY,
  MAX_STAY_SFR,
  TRV_REST_COMMENCE,
  TRV_REST_COMMENCE_SFR,
  TRV_REST_COMPLETE,
  TRV_REST_COMPLETE_SFR };

template <typename UpdatedType, typename... TypesToNotify>
class UpdaterObserver : public IObserver<TypesToNotify...>
{
public:
  void updateIfNotified(UpdatedType& ref)
  {
    if (_isNotified)
      update(ref);
  }

  ~UpdaterObserver() override = default;

  virtual void update(UpdatedType& ref) = 0;

  // Factory Method should be specialized for each observer type in .cpp file

  static std::unique_ptr<UpdaterObserver<UpdatedType, TypesToNotify...>>
  create(ObserverType, DataHandle&, SubjectObserved<IObserver<TypesToNotify...>>*);

protected:
  void setNotified() { _isNotified = true; }

private:
  bool _isNotified = false;
};

using MinStayApplicationObserverType =
    UpdaterObserver<FareUsage, const RuleItemInfo*, DateTime, int16_t, LocCode>;
using MaxStayApplicationObserverType =
    UpdaterObserver<FareUsage, Indicator, const RuleItemInfo*, DateTime, int16_t, LocCode>;
template <typename UpdatedType>
using TravelRestrictionsObserverType = UpdaterObserver<UpdatedType,
                                                       NotificationType,
                                                       uint16_t,
                                                       const DateTime&,
                                                       const LocCode,
                                                       const int>;
template <typename UpdatedType>
using AdvanceResAndTktObserverType =
    UpdaterObserver<UpdatedType, NotificationType, const DateTime&>;
}
