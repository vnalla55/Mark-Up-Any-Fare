#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusDisplayCatType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusDisplayCatTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    DISPLAYCATTYPEITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , DISPLAYCATTYPE

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.DISPLAYCATTYPEITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.DISPLAYCATTYPE ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("DISPLAYCATTYPEITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSDISPLAYCATTYPE",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSDISPLAYCATTYPEDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.DISPLAYCATTYPEITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusDisplayCatTypeInfo* mapRow(Row* row, FareFocusDisplayCatTypeInfo* infoPrev)
  {
    FareFocusDisplayCatTypeInfo* info(nullptr);
    uint64_t displayCatTypeItemNo(row->getLong(DISPLAYCATTYPEITEMNO));
    if (infoPrev && displayCatTypeItemNo == infoPrev->displayCatTypeItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusDisplayCatTypeInfo;
      info->displayCatTypeItemNo() = row->getLong(DISPLAYCATTYPEITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    if (!row->isNull(DISPLAYCATTYPE))
      info->displayCatType().push_back(row->getChar(DISPLAYCATTYPE));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusDisplayCatTypeHistoricalSQLStatement
    : public QueryGetFareFocusDisplayCatTypeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.DISPLAYCATTYPEITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusDisplayCatTypeSQLStatement
    : public QueryGetFareFocusDisplayCatTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.DISPLAYCATTYPEITEMNO, ORDERNO");
  }
};

} // tse

