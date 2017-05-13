#pragma once

#include "Common/FallbackUtil.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerCalc.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackHandlingFee);

template <typename QUERYCLASS>
class QueryGetFareRetailerCalcSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    FARERETAILERCALCITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , ORDERNO
    , CALCULATIONTYPECD
    , FARECALCIND
    , PERCENT1
    , PERCENTNODEC1
    , PERCENTMIN1
    , PERCENTMAX1
    , AMOUNT1
    , AMOUNTNODEC1
    , AMOUNTCURRENCY1
    , AMOUNTMIN1
    , AMOUNTMAX1
    , PERCENT2
    , PERCENTNODEC2
    , PERCENTMIN2
    , PERCENTMAX2
    , AMOUNT2
    , AMOUNTNODEC2
    , AMOUNTCURRENCY2
    , AMOUNTMIN2
    , AMOUNTMAX2
    , HIDEOPTNCD
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    if (fallback::fixed::fallbackHandlingFee())
    {
      Command("select p.FARERETAILERCALCITEMNO, p.CREATEDATE, p.EXPIREDATE,"
              "       c.ORDERNO, CALCULATIONTYPECD, FARECALCIND, PERCENT1, PERCENTNODEC1, PERCENTMIN1, PERCENTMAX1,"
              "       AMOUNT1, AMOUNTNODEC1, AMOUNTCURRENCY1, AMOUNTMIN1, AMOUNTMAX1,"
              "       PERCENT2, PERCENTNODEC2, PERCENTMIN2, PERCENTMAX2,"
              "       AMOUNT2, AMOUNTNODEC2, AMOUNTCURRENCY2, AMOUNTMIN2, AMOUNTMAX2 ");
    }
    else
    {
      Command("select p.FARERETAILERCALCITEMNO, p.CREATEDATE, p.EXPIREDATE,"
              "       c.ORDERNO, CALCULATIONTYPECD, FARECALCIND, PERCENT1, PERCENTNODEC1, PERCENTMIN1, PERCENTMAX1,"
              "       AMOUNT1, AMOUNTNODEC1, AMOUNTCURRENCY1, AMOUNTMIN1, AMOUNTMAX1,"
              "       PERCENT2, PERCENTNODEC2, PERCENTMIN2, PERCENTMAX2,"
              "       AMOUNT2, AMOUNTNODEC2, AMOUNTCURRENCY2, AMOUNTMIN2, AMOUNTMAX2, HIDEOPTNCD ");
    }

    std::vector<std::string> joinFields;
    joinFields.reserve(1);
    joinFields.push_back("FARERETAILERCALCITEMNO");
    std::string from;
    this->generateJoinString("=FARERETAILERCALC",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FARERETAILERCALCDETAIL",
                             "c",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.FARERETAILERCALCITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareRetailerCalcInfo* mapRow(Row* row, FareRetailerCalcInfo* infoPrev)
  {
    FareRetailerCalcInfo* info(nullptr);
    uint64_t fareRetailerCalcItemNo(row->getLong(FARERETAILERCALCITEMNO));
    if (infoPrev && fareRetailerCalcItemNo == infoPrev->fareRetailerCalcItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareRetailerCalcInfo;
      info->fareRetailerCalcItemNo() = row->getLong(FARERETAILERCALCITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareRetailerCalcDetailInfo* detail(new FareRetailerCalcDetailInfo);
    detail->orderNo() = row->getInt(ORDERNO);
    detail->calculationTypeCd() = row->getString(CALCULATIONTYPECD);
    detail->fareCalcInd() = row->getChar(FARECALCIND);
    // nullable below
    if (!row->isNull(PERCENTNODEC1))
    {
      detail->percentNoDec1() = row->getInt(PERCENTNODEC1);
      if (!row->isNull(PERCENT1))
        detail->percent1() = QUERYCLASS::adjustDecimal(row->getInt(PERCENT1), detail->percentNoDec1());
      if (!row->isNull(PERCENTMIN1))
        detail->percentMin1() = QUERYCLASS::adjustDecimal(row->getInt(PERCENTMIN1), detail->percentNoDec1());
      if (!row->isNull(PERCENTMAX1))
        detail->percentMax1() = QUERYCLASS::adjustDecimal(row->getInt(PERCENTMAX1), detail->percentNoDec1());
    }

    if (!row->isNull(AMOUNTCURRENCY1))
      detail->amountCurrency1() = row->getString(AMOUNTCURRENCY1);
    if (!row->isNull(AMOUNTNODEC1))
    {
      detail->amountNoDec1() = row->getInt(AMOUNTNODEC1);
      if (!row->isNull(AMOUNT1))
        detail->amount1() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNT1), detail->amountNoDec1());
      if (!row->isNull(AMOUNTMIN1))
        detail->amountMin1() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNTMIN1), detail->amountNoDec1());
      if (!row->isNull(AMOUNTMAX1))
        detail->amountMax1() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNTMAX1), detail->amountNoDec1());
    }

    if (!row->isNull(PERCENTNODEC2))
    {
      detail->percentNoDec2() = row->getInt(PERCENTNODEC2);
      if (!row->isNull(PERCENT2))
        detail->percent2() = QUERYCLASS::adjustDecimal(row->getInt(PERCENT2), detail->percentNoDec2());
      if (!row->isNull(PERCENTMIN2))
        detail->percentMin2() = QUERYCLASS::adjustDecimal(row->getInt(PERCENTMIN2), detail->percentNoDec2());
      if (!row->isNull(PERCENTMAX2))
        detail->percentMax2() = QUERYCLASS::adjustDecimal(row->getInt(PERCENTMAX2), detail->percentNoDec2());
    }

    if (!row->isNull(AMOUNTCURRENCY2))
      detail->amountCurrency2() = row->getString(AMOUNTCURRENCY2);
    if (!row->isNull(AMOUNTNODEC2))
    {
      detail->amountNoDec2() = row->getInt(AMOUNTNODEC2);
      if (!row->isNull(AMOUNT2))
        detail->amount2() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNT2), detail->amountNoDec2());
      if (!row->isNull(AMOUNTMIN2))
        detail->amountMin2() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNTMIN2), detail->amountNoDec2());
      if (!row->isNull(AMOUNTMAX2))
        detail->amountMax2() = QUERYCLASS::adjustDecimal(row->getInt(AMOUNTMAX2), detail->amountNoDec2());
    }

    if (!fallback::fixed::fallbackHandlingFee()
        && !row->isNull(HIDEOPTNCD))
    {
      detail->hideOptnCd() = row->getChar(HIDEOPTNCD);
    }

    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareRetailerCalcHistoricalSQLStatement
    : public QueryGetFareRetailerCalcSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.FARERETAILERCALCITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareRetailerCalcSQLStatement
    : public QueryGetFareRetailerCalcSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.FARERETAILERCALCITEMNO, ORDERNO");
  }
};

} // tse


