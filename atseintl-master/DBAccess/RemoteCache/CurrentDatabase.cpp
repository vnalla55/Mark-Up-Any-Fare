#include "DBAccess/RemoteCache/CurrentDatabase.h"

#include "DBAccess/ORACLEAdapter.h"

#include <mutex>
#include <set>

namespace tse
{

namespace RemoteCache
{

namespace
{

enum
{
  NONHISTORICAL
  , HISTORICAL
};

std::set<std::string> nonhistoricalSet;
std::set<std::string> historicalSet;
std::string nonhistoricalString;
std::string historicalString;
bool nonhistoricalInTransition(false);
bool historicalInTransition(false);

std::string removeSuffix(const std::string& database)
{
  std::string transformed(database);
  std::size_t underscore(transformed.find('_'));
  if (underscore != std::string::npos)
  {
    transformed.erase(underscore);
  }
  return transformed;
}

std::mutex mutex;

}// namespace

void recordCurrentDatabase(const std::string& database,
                           bool isHistorical,
                           int numberConnections)
{
  std::unique_lock<std::mutex> lock(mutex);
  std::set<std::string>& set(isHistorical ? historicalSet : nonhistoricalSet);
  if (0 >= numberConnections)
  {
    set.erase(database);
  }
  else
  {
    set.insert(database);
  }
  std::string& str(isHistorical ? historicalString : nonhistoricalString);
  str.clear();
  bool& inTransition(isHistorical ? historicalInTransition : nonhistoricalInTransition);
  inTransition = false;
  for (const auto& name : set)
  {
    if (!str.empty())
    {
      str.append(",");
      inTransition = true;
    }
    str.append(removeSuffix(name));
  }
}

std::string getCurrentDatabase(bool isHistorical,
                               bool& inTransition)
{
  std::unique_lock<std::mutex> lock(mutex);
  typedef std::map<std::string, int> Connections;
  const Connections& connections(ORACLEAdapter::dbConnections());
  for (int i = 0; i < 2; ++i)
  {
    const std::set<std::string>& set(0 == i ? nonhistoricalSet : historicalSet);
    std::string& string(0 == i ? nonhistoricalString : historicalString);
    bool& transition(0 == i ? nonhistoricalInTransition : historicalInTransition);
    string.clear();
    transition = false;
    for (const auto& database : set)
    {
      Connections::const_iterator it(connections.find(database));
      if (it != connections.end() && it->second > 0)
      {
        if (!string.empty())
        {
          string.append(",");
          transition = true;
        }
        string.append(removeSuffix(it->first));
      }
    }
  }
  if (isHistorical)
  {
    if (!historicalString.empty())
    {
      inTransition = historicalInTransition;
      return historicalString;
    }
    else
    {
      inTransition = nonhistoricalInTransition;
      return nonhistoricalString;
    }
  }
  else
  {
    if (!nonhistoricalString.empty())
    {
      inTransition = nonhistoricalInTransition;
      return nonhistoricalString;
    }
    else
    {
      inTransition = historicalInTransition;
      return historicalString;
    }
  }
}

}// RemoteCache

}// tse
