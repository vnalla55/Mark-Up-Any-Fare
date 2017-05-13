/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include <utility>
#include <vector>

namespace tse
{

template <typename ObserverType>
class SubjectObserved
{
public:
  void
  registerObserver(ObserverType* observer)
  {
    _observersList.push_back(observer);
  }

  void
  unregisterObserver(ObserverType* observer)
  {
    _observersList.erase(std::remove(_observersList.begin(), _observersList.end(), observer),
                         _observersList.end());
  }

  template <typename... TypesToNotify>
  void notifyObservers(TypesToNotify&&... parametersToUpdate) const
  {
    for (auto observer : _observersList)
    {
      observer->notify(std::forward<TypesToNotify>(parametersToUpdate)...);
    }
  }
  virtual ~SubjectObserved() = default;

private:
  std::vector<ObserverType*> _observersList;
};

}
