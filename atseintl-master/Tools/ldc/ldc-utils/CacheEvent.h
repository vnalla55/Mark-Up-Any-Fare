#ifndef CacheEvent__H
#define CacheEvent__H

#include <set>
#include <string>
#include <time.h>

struct CacheEventItem
{
  std::string table;
  std::string key;
  std::string origKey;
  time_t localtime;

  friend bool operator<(const CacheEventItem& lhs, const CacheEventItem& rhs)
  {
    if (lhs.table != rhs.table)
    {
      return (lhs.table < rhs.table);
    }
    else
    {
      return (lhs.key < rhs.key);
    }
  }
};

class CacheEvent
{
public:
  CacheEvent(std::string hostArg,
             std::string userArg,
             std::string passwordArg,
             std::string databaseArg,
             std::string portArg,
             std::string tableArg);

  bool load() { return false;  }

  std::set<CacheEventItem, std::less<CacheEventItem> > collection;
  std::set<std::string, std::less<std::string> > entities;

private:
  std::string host;
  std::string user;
  std::string password;
  std::string database;
  std::string table;
  std::string query;
  unsigned int port;
  bool upperCaseEntity;
  bool trimLastPipe;
  bool cutSpaces;
};

#endif
