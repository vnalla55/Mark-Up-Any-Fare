#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusAccountCd.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusAccountCdSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    ACCOUNTCDITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , ACCOUNTCD
    , ACCOUNTCDTYPE

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.ACCOUNTCDITEMNO, CREATEDATE, EXPIREDATE, ACCOUNTCD, ACCOUNTCDTYPE ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("ACCOUNTCDITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSACCOUNTCD",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSACCOUNTCDDETAIL",
                             "c",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.ACCOUNTCDITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusAccountCdInfo* mapRow(Row* row, FareFocusAccountCdInfo* infoPrev)
  {
    FareFocusAccountCdInfo* info(nullptr);
    uint64_t accountCdItemNo(row->getLong(ACCOUNTCDITEMNO));
    if (infoPrev && accountCdItemNo == infoPrev->accountCdItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusAccountCdInfo;
      info->accountCdItemNo() = row->getLong(ACCOUNTCDITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareFocusAccountCdDetailInfo* detail(new FareFocusAccountCdDetailInfo);
    detail->accountCd() = row->getString(ACCOUNTCD);
    detail->accountCdType() = row->getChar(ACCOUNTCDTYPE);
    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusAccountCdHistoricalSQLStatement
    : public QueryGetFareFocusAccountCdSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.ACCOUNTCDITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusAccountCdSQLStatement
    : public QueryGetFareFocusAccountCdSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.ACCOUNTCDITEMNO, ORDERNO");
  }
};

} // tse

