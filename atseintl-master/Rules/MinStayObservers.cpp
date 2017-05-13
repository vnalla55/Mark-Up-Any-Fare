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

#include "Rules/UpdaterObserver.h"

#include "DataModel/FareUsage.h"
#include "Rules/SubjectObserved.h"

#include <memory>

namespace tse
{
using BaseObserverType = IObserver<const RuleItemInfo*, DateTime, int16_t, LocCode>;

class MinStayApplicationObserver : public MinStayApplicationObserverType
{
public:
  MinStayApplicationObserver(SubjectObserved<BaseObserverType>* minStayApp)
    : _minStayApp(minStayApp)
  {
    _minStayApp->registerObserver(this);
  }

  void notify(const RuleItemInfo* minStayRestriction,
              DateTime date,
              int16_t startNVBTravelSeg,
              LocCode) override;

  void update(FareUsage& fu) override;
  ~MinStayApplicationObserver() override { _minStayApp->unregisterObserver(this); }

protected:
  SubjectObserved<BaseObserverType>* _minStayApp = nullptr;
  const RuleItemInfo* _minStayRule = nullptr;
  int16_t _startNVBTravelSeg = 0;
  DateTime _minStayDate = DateTime();
};

class SFRMinStayObserver : public MinStayApplicationObserver
{
public:
  SFRMinStayObserver(SubjectObserved<BaseObserverType>* minStayApp)
    : MinStayApplicationObserver(minStayApp)
  {
  }

  void notify(const RuleItemInfo* minStayRestriction,
              DateTime date,
              int16_t startNVBTravelSeg,
              LocCode location) override;

  void update(FareUsage& fu) override;

private:
  LocCode _location = LocCode();
};

void
MinStayApplicationObserver::notify(const RuleItemInfo* minStayRestriction,
                                       DateTime date,
                                       int16_t startNVBTravelSeg,
                                       LocCode /*location*/)
{
  setNotified();
  _minStayRule = minStayRestriction;
  _startNVBTravelSeg = startNVBTravelSeg;
  _minStayDate = date;
}

void
MinStayApplicationObserver::update(FareUsage& fu)
{
  fu.startNVBTravelSeg() = _startNVBTravelSeg;
  fu.minStayDate() = _minStayDate;
}

void
SFRMinStayObserver::notify(const RuleItemInfo* minStayRestriction,
                                      DateTime date,
                                      int16_t startNVBTravelSeg,
                                      LocCode location)
{
  MinStayApplicationObserver::notify(minStayRestriction, date, startNVBTravelSeg, location);
  _location = location;
}

void
SFRMinStayObserver::update(FareUsage& fu)
{
  MinStayApplicationObserver::update(fu);
  fu.createStructuredRuleDataIfNonexistent();
  auto& structuredRuleData = fu.getStructuredRuleData();
  structuredRuleData._minStayLocation = _location;
  structuredRuleData._minStayDate = _minStayDate;
  structuredRuleData._minStaySegmentOrder = _startNVBTravelSeg;
}

// Specialization for MinStay Application

template <>
std::unique_ptr<MinStayApplicationObserverType>
MinStayApplicationObserverType::create(ObserverType observerType,
                                       DataHandle&,
                                       SubjectObserved<BaseObserverType>* minStayApp)
{
  switch (observerType)
  {
  case MIN_STAY:
    return std::unique_ptr<MinStayApplicationObserverType>(
        new MinStayApplicationObserver(minStayApp));
  case MIN_STAY_SFR:
    return std::unique_ptr<MinStayApplicationObserverType>(
        new SFRMinStayObserver(minStayApp));
  default:
    break;
  }
  return std::unique_ptr<MinStayApplicationObserverType>(nullptr);
}

} // namespace tse
