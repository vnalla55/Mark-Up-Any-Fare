#pragma once

#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetServicesDescriptionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetServicesDescriptionSQLStatement() {}
  virtual ~QueryGetServicesDescriptionSQLStatement() {}

  enum ColumnIndexes
  {
    VALUE = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DESCRIPTION,
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VALUE, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE, DESCRIPTION");

    this->From("=SERVICESDESCRIPTION");

    this->Where("VALUE= %1q ");
    this->OrderBy("VALUE, CREATEDATE ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static ServicesDescription* mapRowToServicesDescription(Row* row)
  {
    ServicesDescription* servicesDescription = new ServicesDescription;

    servicesDescription->value() = row->getString(VALUE);
    servicesDescription->createDate() = row->getDate(CREATEDATE);
    servicesDescription->expireDate() = row->getDate(EXPIREDATE);
    servicesDescription->effDate() = row->getDate(EFFDATE);
    servicesDescription->discDate() = row->getDate(DISCDATE);
    servicesDescription->description() = row->getString(DESCRIPTION);

    return servicesDescription;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetServicesDescriptionHistoricalSQLStatement
    : public QueryGetServicesDescriptionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VALUE= %1q "
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
    this->OrderBy("VALUE, CREATEDATE");
  }
};
} // tse

