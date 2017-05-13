// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "DBAccess/SQLQuery.h"

namespace tse
{
class ServicesSubGroup;

class QueryGetServicesSubGroup : public SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetServicesSubGroup(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {}

  QueryGetServicesSubGroup(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {
  }

  virtual ~QueryGetServicesSubGroup() {}

  virtual const char* getQueryName() const override;

  void findServicesSubGroup(std::vector<ServicesSubGroup*>& servicesSubGroup,
                            const ServiceGroup& serviceGroup,
                            const ServiceGroup& serviceSubGroup);

  static void initialize();

  const QueryGetServicesSubGroup& operator=(const QueryGetServicesSubGroup& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  }

  const QueryGetServicesSubGroup& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  }

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
} // tse

