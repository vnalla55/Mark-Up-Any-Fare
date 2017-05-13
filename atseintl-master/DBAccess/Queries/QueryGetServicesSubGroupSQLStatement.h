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

#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetServicesSubGroupSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetServicesSubGroupSQLStatement() {}
  virtual ~QueryGetServicesSubGroupSQLStatement() {}

  enum ColumnIndexes
  {
    SVCGROUP = 0,
    SVCSUBGROUP,
    CREATEDATE,
    DEFINITION
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SVCGROUP, SVCSUBGROUP, CREATEDATE, DEFINITION");
    this->From("=SERVICESSUBGROUP");
    this->Where("SVCGROUP = %1q and SVCSUBGROUP = %2q");
    this->OrderBy("CREATEDATE DESC");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static ServicesSubGroup* mapRowToServicesSubGroup(Row* row)
  {
    ServicesSubGroup* servicesSubGroup = new ServicesSubGroup;

    servicesSubGroup->serviceGroup() = row->getString(SVCGROUP);
    servicesSubGroup->serviceSubGroup() = row->getString(SVCSUBGROUP);
    servicesSubGroup->createDate() = row->getDate(CREATEDATE);
    servicesSubGroup->definition() = row->getString(DEFINITION);

    return servicesSubGroup;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
}

