#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusBookingCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusBookingCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    BOOKINGCDITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , BOOKINGCD

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.BOOKINGCDITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.BOOKINGCD ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("BOOKINGCDITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSBOOKINGCD",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSBOOKINGCDDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.BOOKINGCDITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusBookingCodeInfo* mapRow(Row* row, FareFocusBookingCodeInfo* infoPrev)
  {
    FareFocusBookingCodeInfo* info(nullptr);
    uint64_t bookingCodeItemNo(row->getLong(BOOKINGCDITEMNO));
    if (infoPrev && bookingCodeItemNo == infoPrev->bookingCodeItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusBookingCodeInfo;
      info->bookingCodeItemNo() = row->getLong(BOOKINGCDITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    info->bookingCode().push_back(row->getString(BOOKINGCD));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusBookingCodeHistoricalSQLStatement
    : public QueryGetFareFocusBookingCodeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.BOOKINGCDITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusBookingCodeSQLStatement
    : public QueryGetFareFocusBookingCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.BOOKINGCDITEMNO, ORDERNO");
  }
};

} // tse

