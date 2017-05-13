//----------------------------------------------------------------------------
//  (C) 2009, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSeasonalityDOW.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSeasonalityDOWSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    INHIBIT,
    IND_A,
    IND_B,
    IND_C,
    IND_D,
    IND_E,
    IND_F,
    IND_G,
    IND_H,
    IND_I,
    IND_J,
    IND_K,
    IND_L,
    IND_M,
    IND_N,
    IND_O,
    IND_P,
    IND_Q,
    IND_R,
    IND_S,
    IND_T,
    IND_U,
    IND_V,
    IND_W,
    IND_X,
    IND_Y,
    IND_Z
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, CREATEDATE, EXPIREDATE, "
                  "INHIBIT, IND_A, IND_B, IND_C, IND_D, IND_E, IND_F, "
                  "IND_G, IND_H, IND_I, IND_J, IND_K, IND_L, IND_M, "
                  "IND_N, IND_O, IND_P, IND_Q, IND_R, IND_S, IND_T, "
                  "IND_U, IND_V, IND_W, IND_X, IND_Y, IND_Z");
    this->From("=SEASONALITYDOW ");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    adjustBaseSQL();

    this->ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SeasonalityDOW* mapRowToSeasonalityDOW(Row* row)
  {
    SeasonalityDOW* seasonalityDOW = new SeasonalityDOW;

    seasonalityDOW->vendor() = row->getString(VENDOR);
    seasonalityDOW->itemNo() = row->getInt(ITEMNO);
    seasonalityDOW->createDate() = row->getDate(CREATEDATE);
    seasonalityDOW->expireDate() = row->getDate(EXPIREDATE);
    seasonalityDOW->inhibit() = row->getChar(INHIBIT);
    seasonalityDOW->indA() = row->getChar(IND_A);
    seasonalityDOW->indB() = row->getChar(IND_B);
    seasonalityDOW->indC() = row->getChar(IND_C);
    seasonalityDOW->indD() = row->getChar(IND_D);
    seasonalityDOW->indE() = row->getChar(IND_E);
    seasonalityDOW->indF() = row->getChar(IND_F);
    seasonalityDOW->indG() = row->getChar(IND_G);
    seasonalityDOW->indH() = row->getChar(IND_H);
    seasonalityDOW->indI() = row->getChar(IND_I);
    seasonalityDOW->indJ() = row->getChar(IND_J);
    seasonalityDOW->indK() = row->getChar(IND_K);
    seasonalityDOW->indL() = row->getChar(IND_L);
    seasonalityDOW->indM() = row->getChar(IND_M);
    seasonalityDOW->indN() = row->getChar(IND_N);
    seasonalityDOW->indO() = row->getChar(IND_O);
    seasonalityDOW->indP() = row->getChar(IND_P);
    seasonalityDOW->indQ() = row->getChar(IND_Q);
    seasonalityDOW->indR() = row->getChar(IND_R);
    seasonalityDOW->indS() = row->getChar(IND_S);
    seasonalityDOW->indT() = row->getChar(IND_T);
    seasonalityDOW->indU() = row->getChar(IND_U);
    seasonalityDOW->indV() = row->getChar(IND_V);
    seasonalityDOW->indW() = row->getChar(IND_W);
    seasonalityDOW->indX() = row->getChar(IND_X);
    seasonalityDOW->indY() = row->getChar(IND_Y);
    seasonalityDOW->indZ() = row->getChar(IND_Z);

    return seasonalityDOW;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetAllSeasonalityDOWSQLStatement : public QueryGetSeasonalityDOWSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE"
                " and VALIDITYIND = 'Y'");
  }
};

template <class QUERYCLASS>
class QueryGetSeasonalityDOWHistoricalSQLStatement
    : public QueryGetSeasonalityDOWSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'");
  }
};
}
