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

#include "DataModel/FareUsage.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/StructuredFareRulesUtils.h"
#include "Rules/SubjectObserved.h"
#include "Rules/UpdaterObserver.h"

namespace tse
{
using BaseObserverType =
    IObserver<NotificationType, uint16_t, const DateTime&, const LocCode, const int>;

struct TravelRestrictionsData
{
  const DateTime* _date = nullptr;
  uint16_t _segOrder = 0;
  LocCode _location = LocCode();
  int _time = -1;
};

class TravelCommenceObserver : public TravelRestrictionsObserverType<FareUsage>
{
public:
  TravelCommenceObserver(SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle);
  void notify(NotificationType notiType,
              uint16_t nvaStartSegOrder,
              const DateTime& mustCommence,
              const LocCode location,
              const int) override;
  void update(FareUsage& fu) override;
  ~TravelCommenceObserver() override;

protected:
  DataHandle& _dataHandle;
  SubjectObserved<BaseObserverType>* _tvlRestApp = nullptr;
  std::vector<TravelRestrictionsData> _tvlRestrData;
};

TravelCommenceObserver::TravelCommenceObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                               DataHandle& dataHandle)
  : _dataHandle(dataHandle), _tvlRestApp(tvlRestApp)
{
  _tvlRestApp->registerObserver(this);
}

TravelCommenceObserver::~TravelCommenceObserver()
{
  _tvlRestApp->unregisterObserver(this);
}

void
TravelCommenceObserver::notify(NotificationType notiType,
                               uint16_t nvaStartSegOrder,
                               const DateTime& mustCommence,
                               const LocCode location,
                               const int time)
{
  if (notiType == NotificationType::TRAVEL_MUST_COMMENCE)
  {
    setNotified();
    _tvlRestrData.push_back({&mustCommence, nvaStartSegOrder, location, time});
  }
}

void
TravelCommenceObserver::update(FareUsage& fu)
{
  for (auto& data : _tvlRestrData)
    fu.addNVAData(_dataHandle, data._segOrder, data._date);
}

class TravelCompleteObserver : public TravelRestrictionsObserverType<FareUsage>
{
public:
  TravelCompleteObserver(SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle);
  void notify(NotificationType notiType,
              uint16_t nvaStopSegOrder,
              const DateTime& mustBeComplete,
              const LocCode,
              const int) override;
  void update(FareUsage& fu) override;
  ~TravelCompleteObserver() override;

protected:
  DataHandle& _dataHandle;
  SubjectObserved<BaseObserverType>* _tvlRestApp = nullptr;
  TravelRestrictionsData _tvlRestrData;
};

TravelCompleteObserver::TravelCompleteObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                               DataHandle& dataHandle)
  : _dataHandle(dataHandle), _tvlRestApp(tvlRestApp)
{
  _tvlRestApp->registerObserver(this);
}

TravelCompleteObserver::~TravelCompleteObserver()
{
  _tvlRestApp->unregisterObserver(this);
}

void
TravelCompleteObserver::notify(NotificationType notiType,
                               uint16_t nvaStopSegOrder,
                               const DateTime& mustBeComplete,
                               const LocCode location,
                               const int time)
{
  if (notiType == NotificationType::TRAVEL_MUST_COMPLETE)
  {
    setNotified();
    _tvlRestrData._date = &mustBeComplete;
    _tvlRestrData._segOrder = nvaStopSegOrder;
    _tvlRestrData._location = location;
    _tvlRestrData._time = time;
  }
}

void
TravelCompleteObserver::update(FareUsage& fu)
{
  fu.addNVAData(_dataHandle, _tvlRestrData._segOrder, _tvlRestrData._date);
}

class SFRTravelCommenceObserver : public TravelCommenceObserver
{
public:
  SFRTravelCommenceObserver(SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle)
    : TravelCommenceObserver(tvlRestApp, dataHandle)
  {
  }

  void update(FareUsage& fu) override;
  ~SFRTravelCommenceObserver() override = default;
};

void
SFRTravelCommenceObserver::update(FareUsage& fu)
{
  TravelCommenceObserver::update(fu);

  fu.createStructuredRuleDataIfNonexistent();

  MaxStayMap& maxStayMap = fu.getStructuredRuleData()._maxStayMostRestrictiveFCData;

  for (auto& data : _tvlRestrData)
  {
    structuredFareRulesUtils::updateMaxStayTrvCommenceData(
        maxStayMap,
        data._segOrder,
        structuredFareRulesUtils::getDateWithEarliestTime(*data._date, data._time),
        data._location);
  }
}

class SFRTravelCompleteObserver : public TravelCompleteObserver
{
public:
  SFRTravelCompleteObserver(SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle)
    : TravelCompleteObserver(tvlRestApp, dataHandle)
  {
  }

  void update(FareUsage& fu) override;
  ~SFRTravelCompleteObserver() override = default;
};

void
SFRTravelCompleteObserver::update(FareUsage& fu)
{
  TravelCompleteObserver::update(fu);

  fu.createStructuredRuleDataIfNonexistent();

  MaxStayMap& maxStayMap = fu.getStructuredRuleData()._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap,
      _tvlRestrData._segOrder,
      structuredFareRulesUtils::getDateWithEarliestTime(*_tvlRestrData._date, _tvlRestrData._time),
      _tvlRestrData._location);
}

// Specialization for TravelRestrictions Application

template <>
std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>
TravelRestrictionsObserverType<FareUsage>::create(ObserverType observerType,
                                                  DataHandle& dataHandle,
                                                  SubjectObserved<BaseObserverType>* tvlRestApp)
{
  switch (observerType)
  {
  case TRV_REST_COMMENCE:
    return std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>(
        new TravelCommenceObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMMENCE_SFR:
    return std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>(
        new SFRTravelCommenceObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMPLETE:
    return std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>(
        new TravelCompleteObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMPLETE_SFR:
    return std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>(
        new SFRTravelCompleteObserver(tvlRestApp, dataHandle));
  default:
    return std::unique_ptr<TravelRestrictionsObserverType<FareUsage>>(nullptr);
  }
}

class PaxTypeFareTravelCommenceObserver : public TravelRestrictionsObserverType<PaxTypeFare>
{
public:
  PaxTypeFareTravelCommenceObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                    DataHandle& dataHandle);
  void notify(NotificationType notiType,
              uint16_t nvaStartSegOrder,
              const DateTime& mustCommence,
              const LocCode location,
              const int) override;
  void update(PaxTypeFare& paxTypeFare) override;
  ~PaxTypeFareTravelCommenceObserver() override;

protected:
  DataHandle& _dataHandle;
  SubjectObserved<BaseObserverType>* _tvlRestApp = nullptr;
  std::vector<TravelRestrictionsData> _tvlRestrData;
};

PaxTypeFareTravelCommenceObserver::PaxTypeFareTravelCommenceObserver(
    SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle)
  : _dataHandle(dataHandle), _tvlRestApp(tvlRestApp)
{
  _tvlRestApp->registerObserver(this);
}

PaxTypeFareTravelCommenceObserver::~PaxTypeFareTravelCommenceObserver()
{
  _tvlRestApp->unregisterObserver(this);
}

void
PaxTypeFareTravelCommenceObserver::notify(NotificationType notiType,
                                          uint16_t nvaStartSegOrder,
                                          const DateTime& mustCommence,
                                          const LocCode location,
                                          const int time)
{
  if (notiType == NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMMENCE)
  {
    setNotified();
    _tvlRestrData.push_back({&mustCommence, nvaStartSegOrder, location, time});
  }
}

void
PaxTypeFareTravelCommenceObserver::update(PaxTypeFare& paxTypeFare)
{
  for (auto& data : _tvlRestrData)
    paxTypeFare.addNVAData(_dataHandle, data._segOrder, data._date);
}

class PaxTypeFareTravelCompleteObserver : public TravelRestrictionsObserverType<PaxTypeFare>
{
public:
  PaxTypeFareTravelCompleteObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                    DataHandle& dataHandle);
  void notify(NotificationType notiType,
              uint16_t nvaStopSegOrder,
              const DateTime& mustBeComplete,
              const LocCode,
              const int) override;
  void update(PaxTypeFare& paxTypeFare) override;
  ~PaxTypeFareTravelCompleteObserver() override;

protected:
  DataHandle& _dataHandle;
  SubjectObserved<BaseObserverType>* _tvlRestApp = nullptr;
  TravelRestrictionsData _tvlRestrData;
};

PaxTypeFareTravelCompleteObserver::PaxTypeFareTravelCompleteObserver(
    SubjectObserved<BaseObserverType>* tvlRestApp, DataHandle& dataHandle)
  : _dataHandle(dataHandle), _tvlRestApp(tvlRestApp)
{
  _tvlRestApp->registerObserver(this);
}

PaxTypeFareTravelCompleteObserver::~PaxTypeFareTravelCompleteObserver()
{
  _tvlRestApp->unregisterObserver(this);
}

void
PaxTypeFareTravelCompleteObserver::notify(NotificationType notiType,
                                          uint16_t nvaStopSegOrder,
                                          const DateTime& mustBeComplete,
                                          const LocCode location,
                                          const int time)
{
  if (notiType == NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE)
  {
    setNotified();
    _tvlRestrData._date = &mustBeComplete;
    _tvlRestrData._segOrder = nvaStopSegOrder;
    _tvlRestrData._location = location;
    _tvlRestrData._time = time;
  }
}

void
PaxTypeFareTravelCompleteObserver::update(PaxTypeFare& paxTypeFare)
{
  paxTypeFare.addNVAData(_dataHandle, _tvlRestrData._segOrder, _tvlRestrData._date);
}

class SFRPaxTypeFareTravelCommenceObserver : public PaxTypeFareTravelCommenceObserver
{
public:
  SFRPaxTypeFareTravelCommenceObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                       DataHandle& dataHandle)
    : PaxTypeFareTravelCommenceObserver(tvlRestApp, dataHandle)
  {
  }

  void update(PaxTypeFare& paxTypeFare) override;
};

void
SFRPaxTypeFareTravelCommenceObserver::update(PaxTypeFare& paxTypeFare)
{
  PaxTypeFareTravelCommenceObserver::update(paxTypeFare);

  paxTypeFare.createStructuredRuleDataIfNonexistent();
  auto& maxStayMap = paxTypeFare.getStructuredRuleData()._maxStayMap;

  for (auto& data : _tvlRestrData)
  {
    structuredFareRulesUtils::updateMaxStayTrvCommenceData(
        maxStayMap,
        data._segOrder,
        structuredFareRulesUtils::getDateWithEarliestTime(*data._date, data._time),
        data._location);
  }
}

class SFRPaxTypeFareTravelCompleteObserver : public PaxTypeFareTravelCompleteObserver
{
public:
  SFRPaxTypeFareTravelCompleteObserver(SubjectObserved<BaseObserverType>* tvlRestApp,
                                       DataHandle& dataHandle)
    : PaxTypeFareTravelCompleteObserver(tvlRestApp, dataHandle)
  {
  }

  void update(PaxTypeFare& paxTypeFare) override;
};

void
SFRPaxTypeFareTravelCompleteObserver::update(PaxTypeFare& paxTypeFare)
{
  PaxTypeFareTravelCompleteObserver::update(paxTypeFare);

  paxTypeFare.createStructuredRuleDataIfNonexistent();
  auto& maxStayMap = paxTypeFare.getStructuredRuleData()._maxStayMap;

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap,
      _tvlRestrData._segOrder,
      structuredFareRulesUtils::getDateWithEarliestTime(*_tvlRestrData._date, _tvlRestrData._time),
      _tvlRestrData._location);
}

// Specialization for TravelRestrictions Application

template <>
std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>
TravelRestrictionsObserverType<PaxTypeFare>::create(ObserverType observerType,
                                                    DataHandle& dataHandle,
                                                    SubjectObserved<BaseObserverType>* tvlRestApp)
{
  switch (observerType)
  {
  case TRV_REST_COMMENCE:
    return std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>(
        new PaxTypeFareTravelCommenceObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMMENCE_SFR:
    return std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>(
        new SFRPaxTypeFareTravelCommenceObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMPLETE:
    return std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>(
        new PaxTypeFareTravelCompleteObserver(tvlRestApp, dataHandle));
  case TRV_REST_COMPLETE_SFR:
    return std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>(
        new SFRPaxTypeFareTravelCompleteObserver(tvlRestApp, dataHandle));
  default:
    return std::unique_ptr<TravelRestrictionsObserverType<PaxTypeFare>>(nullptr);
  }
}

} // namespace tse
