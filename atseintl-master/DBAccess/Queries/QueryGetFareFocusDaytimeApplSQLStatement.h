#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusDaytimeAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusDaytimeApplSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    DAYTIMEAPPLITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , ORDERNO
    , STARTTIME
    , STOPTIME
    , TODAPPL
    , DOW
    , DAYTIMENEG
    , STARTDATE
    , STOPDATE
    , NUMBEROFCOLUMNS
  };


  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.DAYTIMEAPPLITEMNO, p.CREATEDATE, p.EXPIREDATE, c.ORDERNO, c.STARTTIME, c.STOPTIME, c.TODAPPL, c.DOW , c.DAYTIMENEG, c.STARTDATE, c.STOPDATE  ");
    std::vector<std::string> joinFields;
    //joinFields.reserve(4);
    joinFields.push_back("DAYTIMEAPPLITEMNO");
    std::string from;

    this->generateJoinString("=FAREFOCUSDAYTIMEAPPL",
                                 "p",
                                 "LEFT OUTER JOIN",
                                 "=FAREFOCUSDAYTIMEAPPLDETAIL",
                                 "c",
                                 joinFields,
                                 from);

    this->From(from);

    this->Where("p.DAYTIMEAPPLITEMNO = %1n"
                " and %2n <= EXPIREDATE");

    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusDaytimeApplInfo* mapRow(Row* row, FareFocusDaytimeApplInfo* infoPrev)
  {
    FareFocusDaytimeApplInfo* info(nullptr);
    uint64_t dayTimeApplItemNo(row->getLong(DAYTIMEAPPLITEMNO));
    if (infoPrev && dayTimeApplItemNo == infoPrev->dayTimeApplItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusDaytimeApplInfo;
      info->dayTimeApplItemNo() = row->getLong(DAYTIMEAPPLITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareFocusDaytimeApplDetailInfo* detail(new FareFocusDaytimeApplDetailInfo);
    if (!row->isNull(ORDERNO))
      detail->orderNo() = row->getInt(ORDERNO);
    if (!row->isNull(STARTTIME))
      detail->startTime() = row->getInt(STARTTIME);
    if (!row->isNull(STOPTIME))
      detail->stopTime() = row->getInt(STOPTIME);
    if (!row->isNull(TODAPPL))
      detail->todAppl() = row->getChar(TODAPPL);
    if (!row->isNull(DOW))
      detail->dow() = row->getString(DOW);
    if (!row->isNull(DAYTIMENEG))
      detail->dayTimeNeg() = row->getChar(DAYTIMENEG);
    if (!row->isNull(STARTDATE))
      detail->startDate() = row->getDate(STARTDATE);
    if (!row->isNull(STOPDATE))
      detail->stopDate() = row->getDate(STOPDATE);
    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusDaytimeApplHistoricalSQLStatement
    : public QueryGetFareFocusDaytimeApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.DAYTIMEAPPLITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusDaytimeApplSQLStatement
    : public QueryGetFareFocusDaytimeApplSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.DAYTIMEAPPLITEMNO, ORDERNO");
  }
};

} // tse

