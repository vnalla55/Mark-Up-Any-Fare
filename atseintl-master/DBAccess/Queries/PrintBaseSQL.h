//----------------------------------------------------------------------------
//          File:           PrintBaseSQL.h
//          Description:    Print the registered _baseSQL for verification
//          Created:        10/4/2007
// Authors:
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

#include <boost/thread/mutex.hpp>

namespace tse
{
class PrintBaseSQL : public tse::SQLQuery
{
public:
  PrintBaseSQL() : SQLQuery() {};
  virtual ~PrintBaseSQL() {};
  void dump() const;
  void dump(std::ostream& os) const;

private:
  static log4cxx::LoggerPtr _logger;
  static boost::mutex _mutex;
}; // class PrintBaseSQL
} // namespace tse

