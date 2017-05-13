//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

namespace tse
{
enum NotificationType
{ LATEST_TKT_DAY,
  LAST_DATE_TO_BOOK,
  TRAVEL_MUST_COMPLETE,
  TRAVEL_MUST_COMMENCE,
  PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE,
  PAX_TYPE_FARE_TRAVEL_MUST_COMMENCE };

template <typename... TypesToNotify>
class IObserver
{
public:
  virtual void notify(TypesToNotify... parametersToUpdate) = 0;
  virtual ~IObserver() = default;
};
}
