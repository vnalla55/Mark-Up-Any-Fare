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
#include "Pricing/StructuredFareRulesUtils.h"
#include "Rules/SubjectObserved.h"

#include <memory>

namespace tse
{
using BaseObserverType = IObserver<Indicator, const RuleItemInfo*, DateTime, int16_t, LocCode>;

class MaxStayApplicationObserver : public MaxStayApplicationObserverType
{
public:
  MaxStayApplicationObserver(SubjectObserved<BaseObserverType>* maxStayApp)
    : _maxStayApp(maxStayApp)
  {
    _maxStayApp->registerObserver(this);
  }

  void notify(Indicator indicator,
              const RuleItemInfo* minStayRestriction,
              DateTime date,
              int16_t startNVBTravelSeg,
              LocCode) override;

  void update(FareUsage& fu) override;
  ~MaxStayApplicationObserver() override { _maxStayApp->unregisterObserver(this); }

protected:
  SubjectObserved<BaseObserverType>* _maxStayApp = nullptr;
  const RuleItemInfo* _maxStayRule = nullptr;
  int16_t _segmentPosition = 0;
  DateTime _maxStayDate = DateTime();
};

void
MaxStayApplicationObserver::notify(Indicator /*not use*/,
                                   const RuleItemInfo* maxStayRestriction,
                                   DateTime date,
                                   int16_t segmentPosition,
                                   LocCode /*not use*/)
{
  setNotified();
  _maxStayRule = maxStayRestriction;
  _segmentPosition = segmentPosition;
  _maxStayDate = date;
}

void
MaxStayApplicationObserver::update(FareUsage& fu)
{
  fu.maxStayDate() = _maxStayDate;
}

class SFRMaxStayObserver : public MaxStayApplicationObserver
{
public:
  SFRMaxStayObserver(SubjectObserved<BaseObserverType>* maxStayApp)
    : MaxStayApplicationObserver(maxStayApp)
  {
  }

  void notify(Indicator indicator,
              const RuleItemInfo* maxStayRestriction,
              DateTime date,
              int16_t startNVBTravelSeg,
              LocCode location) override;

  void update(FareUsage& fu) override;

private:
  LocCode _location = LocCode();
  Indicator _indicator = Indicator();

};

void
SFRMaxStayObserver::notify(Indicator indicator,
                           const RuleItemInfo* minStayRestriction,
                           DateTime date,
                           int16_t startNVBTravelSeg,
                           LocCode location)
{
  MaxStayApplicationObserver::notify(
      indicator, minStayRestriction, date, startNVBTravelSeg, location);
  _location = location;
  _indicator = indicator;
}

void
SFRMaxStayObserver::update(FareUsage& fu)
{
  MaxStayApplicationObserver::update(fu);
  fu.createStructuredRuleDataIfNonexistent();

  auto& maxStayMap = fu.getStructuredRuleData()._maxStayMostRestrictiveFCData;
  if (_indicator == RuleConst::RETURN_TRV_COMMENCE_IND)
  {
    structuredFareRulesUtils::updateMaxStayTrvCommenceData(
        maxStayMap, _segmentPosition, _maxStayDate, _location);
  }
  else if (_indicator == RuleConst::RETURN_TRV_COMPLETE_IND)
  {
    structuredFareRulesUtils::updateMaxStayTrvCompleteData(
        maxStayMap, _segmentPosition, _maxStayDate, _location);
  }
}
// Specialization for MaxStay Application

template <>
std::unique_ptr<MaxStayApplicationObserverType>
MaxStayApplicationObserverType::create(ObserverType observerType,
                                       DataHandle&,
                                       SubjectObserved<BaseObserverType>* minStayApp)
{
  switch (observerType)
  {
  case MAX_STAY:
    return std::unique_ptr<MaxStayApplicationObserverType>(
        new MaxStayApplicationObserver(minStayApp));
  case MAX_STAY_SFR:
    return std::unique_ptr<MaxStayApplicationObserverType>(new SFRMaxStayObserver(minStayApp));
  default:
    break;
  }
  return std::unique_ptr<MaxStayApplicationObserverType>(nullptr);
}

} // namespace tse
