//----------------------------------------------------------------------------
//  File:           PrintBaseSQL.cpp
//  Description:    PrintBaseSQL
//  Created:        10/4/2007
// Authors:
//
//  Updates:
//
// 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/PrintBaseSQL.h"

namespace tse
{
log4cxx::LoggerPtr
PrintBaseSQL::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PrintBaseSQL"));
boost::mutex PrintBaseSQL::_mutex;

void
PrintBaseSQL::dump() const
{
  if (IS_DEBUG_ENABLED(_logger))
  {
    std::ostringstream os;
    dump(os);

    std::string str = os.str();
    LOG4CXX_DEBUG(_logger, str);
  }
}

void
PrintBaseSQL::dump(std::ostream& os) const
{
  boost::lock_guard<boost::mutex> g(_mutex);
  const std::set<std::string>& classNames = SQLQuery::getQueryClassNames();
  std::set<std::string>::const_iterator i = classNames.begin();
  std::set<std::string>::const_iterator end = classNames.end();
  os << "Registered baseSQL" << std::endl;
  while (i != end)
  {
    os << *i << std::endl;
    os << SQLQuery::getBaseSQLForClass(*i) << std::endl << std::endl;
    i++;
  }
}
}
