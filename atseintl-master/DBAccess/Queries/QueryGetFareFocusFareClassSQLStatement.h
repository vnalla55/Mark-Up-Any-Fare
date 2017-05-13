#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusFareClass.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusFareClassSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    FARECLASSITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , FARECLASS

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.FARECLASSITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.FARECLASS ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("FARECLASSITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSFARECLASS",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSFARECLASSDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.FARECLASSITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusFareClassInfo* mapRow(Row* row, FareFocusFareClassInfo* infoPrev)
  {
    FareFocusFareClassInfo* info(nullptr);
    int fareClassItemNo(row->getLong(FARECLASSITEMNO));
    if (infoPrev && fareClassItemNo == infoPrev->fareClassItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusFareClassInfo;
      info->fareClassItemNo() = row->getLong(FARECLASSITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    info->fareClass().push_back(row->getString(FARECLASS));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusFareClassHistoricalSQLStatement
    : public QueryGetFareFocusFareClassSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.FARECLASSITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusFareClassSQLStatement
    : public QueryGetFareFocusFareClassSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.FARECLASSITEMNO, ORDERNO");
  }
};

} // tse

