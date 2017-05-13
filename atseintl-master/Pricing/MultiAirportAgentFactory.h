#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/ShoppingTrx.h"

#include <string>
#include <vector>

namespace tse
{
class MultiAirportAgent;
class ShoppingTrx;

class MultiAirportAgentFactory
{
public:
  typedef std::pair<std::string, std::string> OriginDestinationPair;
  typedef std::vector<OriginDestinationPair> OriginDestinationVec;

  MultiAirportAgent* getAgent(const std::string& s, ShoppingTrx& trx);

protected:
  void generateCityMap(const std::string&);
  bool reqIsQualified();
  bool checkOrigAndDest();
  const LocCode& getOrigin(const int&) const;
  const LocCode& getDestination(const int&) const;
  void setAgentArguments(const OriginDestinationVec::const_iterator& it, const bool& atOrigin);
  OriginDestinationVec::const_iterator findInAirportContainer(const std::string& s) const;

  ShoppingTrx* _trx;
  OriginDestinationVec _cityAirportContainer;
  LocCode _city;
  LocCode _airport;
  bool _atOrigin;

private:
  friend class MultiAirportAgentFactoryTest;
};
}

