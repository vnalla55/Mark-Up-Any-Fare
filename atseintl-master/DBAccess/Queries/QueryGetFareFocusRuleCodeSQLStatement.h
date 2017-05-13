#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRuleCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusRuleCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    RULECDITEMNO
    , CREATEDATE
    , EXPIREDATE
    , RULECD
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.RULECDITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.RULECD ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("RULECDITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSRULECODE",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSRULECODEDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.RULECDITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusRuleCodeInfo* mapRow(Row* row, FareFocusRuleCodeInfo* infoPrev)
  {
    FareFocusRuleCodeInfo* info(nullptr);
    uint64_t ruleCdItemNo(row->getLong(RULECDITEMNO));
    if (infoPrev && ruleCdItemNo == infoPrev->ruleCdItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusRuleCodeInfo;
      info->ruleCdItemNo() = row->getLong(RULECDITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    info->ruleCd().push_back(row->getString(RULECD));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusRuleCodeHistoricalSQLStatement
    : public QueryGetFareFocusRuleCodeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.RULECDITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusRuleCodeSQLStatement
    : public QueryGetFareFocusRuleCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.RULECDITEMNO, ORDERNO");
  }
};

} // tse


