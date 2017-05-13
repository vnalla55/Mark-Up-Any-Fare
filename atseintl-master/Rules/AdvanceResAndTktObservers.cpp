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
#include "Common/DateTime.h"
#include "Pricing/StructuredFareRulesUtils.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/UpdaterObserver.h"

namespace tse
{
using AdvanceResAndTktBaseObserverType = IObserver<NotificationType, const DateTime&>;
using AdvanceResAndTktSubject = SubjectObserved<AdvanceResAndTktBaseObserverType>;

template <typename T>
class SFRAdvanceReservationObserver : public AdvanceResAndTktObserverType<T>
{
public:
  SFRAdvanceReservationObserver(AdvanceResAndTktSubject* advanceResApp)
    : _advanceResApp(advanceResApp)
  {
    _advanceResApp->registerObserver(this);
  }

  ~SFRAdvanceReservationObserver() { _advanceResApp->unregisterObserver(this); }

  void notify(NotificationType notificationType, const DateTime& lastDateToBook) override
  {
    if (notificationType == LAST_DATE_TO_BOOK)
    {
      _lastDateToBook.setWithEarlier(lastDateToBook);
      this->setNotified();
    }
  }

  void update(T& object) override
  {
    object.createStructuredRuleDataIfNonexistent();
    object.getStructuredRuleData().setAdvanceReservationWithEarlier(
        structuredFareRulesUtils::getDateWithEarliestTime(_lastDateToBook));
  }

private:
  DateTime _lastDateToBook = DateTime::openDate();
  AdvanceResAndTktSubject* _advanceResApp = nullptr;
};

template <typename T>
class SFRAdvanceTicketingObserver : public AdvanceResAndTktObserverType<T>
{
public:
  SFRAdvanceTicketingObserver(AdvanceResAndTktSubject* advanceTktApp)
    : _advanceTktApp(advanceTktApp)
  {
    _advanceTktApp->registerObserver(this);
  }

  ~SFRAdvanceTicketingObserver() { _advanceTktApp->unregisterObserver(this); }

  void notify(NotificationType notificationType, const DateTime& latestTktDate) override
  {
    if (notificationType == LATEST_TKT_DAY)
    {
      _latestTktDate.setWithEarlier(latestTktDate);
      this->setNotified();
    }
  }

  void update(T& object) override
  {
    object.createStructuredRuleDataIfNonexistent();
    object.getStructuredRuleData().setAdvanceTicketingWithEarlier(
        structuredFareRulesUtils::getDateWithEarliestTime(_latestTktDate));
  }

private:
  DateTime _latestTktDate = DateTime::openDate();
  AdvanceResAndTktSubject* _advanceTktApp = nullptr;
};

template <>
std::unique_ptr<AdvanceResAndTktObserverType<PaxTypeFare>>
AdvanceResAndTktObserverType<PaxTypeFare>::create(ObserverType observerType,
                                                  DataHandle&,
                                                  AdvanceResAndTktSubject* subjectObserved)
{
  if (observerType == ADVANCE_RESERVATION_SFR)
  {
    return std::unique_ptr<AdvanceResAndTktObserverType<PaxTypeFare>>(
        new SFRAdvanceReservationObserver<PaxTypeFare>(subjectObserved));
  }

  if (observerType == ADVANCE_TICKETING_SFR)
  {
    return std::unique_ptr<AdvanceResAndTktObserverType<PaxTypeFare>>(
        new SFRAdvanceTicketingObserver<PaxTypeFare>(subjectObserved));
  }

  return nullptr;
}

template <>
std::unique_ptr<AdvanceResAndTktObserverType<FareUsage>>
AdvanceResAndTktObserverType<FareUsage>::create(ObserverType observerType,
                                                DataHandle&,
                                                AdvanceResAndTktSubject* subjectObserved)
{
  if (observerType == ADVANCE_RESERVATION_SFR)
  {
    return std::unique_ptr<AdvanceResAndTktObserverType<FareUsage>>(
        new SFRAdvanceReservationObserver<FareUsage>(subjectObserved));
  }

  if (observerType == ADVANCE_TICKETING_SFR)
  {
    return std::unique_ptr<AdvanceResAndTktObserverType<FareUsage>>(
        new SFRAdvanceTicketingObserver<FareUsage>(subjectObserved));
  }

  return nullptr;
}
}
