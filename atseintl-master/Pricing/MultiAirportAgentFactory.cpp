#include "Pricing/MultiAirportAgentFactory.h"

#include "Common/Logger.h"
#include "Pricing/MultiAirportAgent.h"

namespace tse
{
namespace
{
Logger
logger("atseintl.PricingOrchestrator.MultiAirportAgentFactory");
}

MultiAirportAgent*
MultiAirportAgentFactory::getAgent(const std::string& s, ShoppingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "getAgent, " << s);
  _trx = &trx;
  generateCityMap(s);

  MultiAirportAgent* agent = nullptr;
  if (reqIsQualified())
  {
    agent = _trx->dataHandle().create<MultiAirportAgent>();
    agent->init(_trx, _city, _airport, _atOrigin);
    LOG4CXX_DEBUG(logger,
                  "agent created, _city = " << _city << ", _airport = " << _airport
                                            << ", _atOrigin = " << _atOrigin);
  }
  return agent;
}

bool
MultiAirportAgentFactory::reqIsQualified()
{
  if (_trx->isAltDates())
    return false;

  if (_trx->isSimpleTrip())
    return checkOrigAndDest();
  else
    return false;
}

bool
MultiAirportAgentFactory::checkOrigAndDest()
{
  OriginDestinationVec::const_iterator origIt = findInAirportContainer(getOrigin(0)); // origin leg0
  OriginDestinationVec::const_iterator destIt =
      findInAirportContainer(getDestination(0)); // destination leg0
  OriginDestinationVec::const_iterator endIt = _cityAirportContainer.end();

  if (origIt == endIt && destIt == endIt)
    return false; // not origin nor dest on the list
  else if (origIt < destIt) // airport is at the origin
    setAgentArguments(origIt, true);
  else
    // airport is at the destination
    setAgentArguments(destIt, false);

  return true;
}

MultiAirportAgentFactory::OriginDestinationVec::const_iterator
MultiAirportAgentFactory::findInAirportContainer(const std::string& s) const
{
  OriginDestinationVec::const_iterator it = _cityAirportContainer.begin();
  for (; it != _cityAirportContainer.end(); ++it)
    if ((*it).first == s)
      break;
  return it;
}

void
MultiAirportAgentFactory::setAgentArguments(const OriginDestinationVec::const_iterator& it,
                                            const bool& atOrigin)
{
  _city = (*it).first;
  _airport = (*it).second;
  _atOrigin = atOrigin;
}

const LocCode&
MultiAirportAgentFactory::getOrigin(const int& legId) const
{
  return _trx->legs()[legId].sop()[0].itin()->travelSeg().front()->boardMultiCity();
}

const LocCode&
MultiAirportAgentFactory::getDestination(const int& legId) const
{
  return _trx->legs()[legId].sop()[0].itin()->travelSeg().back()->offMultiCity();
}

void
MultiAirportAgentFactory::generateCityMap(const std::string& s)
{
  using namespace boost;
  // s like "LON*LGW|NYC*LGA"
  typedef const tokenizer<char_separator<char> > tokenizer;
  char_separator<char> separatorPipe("|");
  tokenizer tokens(s, separatorPipe);
  tokenizer::const_iterator tokenI = tokens.begin();
  const tokenizer::const_iterator tokenE = tokens.end();
  for (; tokenI != tokenE; ++tokenI)
  {
    const std::string cxrList = tokenI->data();
    char_separator<char> separatorAsterix("*");
    tokenizer tokensList(cxrList, separatorAsterix);
    tokenizer::const_iterator it = tokensList.begin();
    tokenizer::const_iterator itEnd = tokensList.end();

    std::vector<std::string> strVec;
    for (int i = 0; i < 2 && it != itEnd; ++i, ++it)
      strVec.push_back(it->data());

    if (strVec.size() == 2)
    {
      const LocCode city = strVec[0];
      const LocCode airport = strVec[1];
      _cityAirportContainer.push_back(std::make_pair(city, airport));
    }
    else
      LOG4CXX_ERROR(logger, "incorrect list element: " << cxrList);
  }
}

} // tse
