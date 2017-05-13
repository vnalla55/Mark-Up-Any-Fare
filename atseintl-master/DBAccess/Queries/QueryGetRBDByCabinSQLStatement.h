#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetRBDByCabin.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetRBDByCabinSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR
    , CARRIER
    , SEQNO
    , CREATEDATE
    , EXPIREDATE
    , EFFDATE
    , DISCDATE
    , AREAIND
    , LOC1
    , LOC1TYPE
    , LOC2
    , LOC2TYPE
    , FIRSTTKTDATE
    , LASTTKTDATE
    , FLIGHTNO1
    , FLIGHTNO2
    , EQUIPMENT
    // child
    , CABIN
    , BOOKINGCODE1
    , BOOKINGCODE2
    , BOOKINGCODE3
    , BOOKINGCODE4
    , BOOKINGCODE5
    , BOOKINGCODE6
    , BOOKINGCODE7
    , BOOKINGCODE8
    , BOOKINGCODE9
    , BOOKINGCODE10
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select rbd.VENDOR, rbd.CARRIER, rbd.SEQNO, rbd.CREATEDATE, "
                  "EXPIREDATE, EFFDATE, DISCDATE, AREAIND, "
                  "LOC1, LOC1TYPE, LOC2, LOC2TYPE, FIRSTTKTDATE, LASTTKTDATE, "
                  "FLIGHTNO1, FLIGHTNO2, EQUIPMENT, "
                  "CABIN, BOOKINGCODE1, BOOKINGCODE2, BOOKINGCODE3, BOOKINGCODE4, "
                  "BOOKINGCODE5, BOOKINGCODE6, BOOKINGCODE7, BOOKINGCODE8, "
                  "BOOKINGCODE9, BOOKINGCODE10 "
                  "from RBDBYCABIN rbd "
                  "LEFT OUTER JOIN RBDBYCABINSEG rbdseg ON "
                  "rbdseg.SEQNO = rbd.SEQNO and rbdseg.VENDOR = rbd.VENDOR "
                  "and rbdseg.CARRIER = rbd.CARRIER and rbdseg.CREATEDATE = rbd.CREATEDATE ");

    this->Where("rbd.VENDOR = %1q and rbd.CARRIER = %2q and VALIDITYIND = 'Y' "
                "and EXPIREDATE >= %cd and DISCDATE >= %cd and EFFDATE <= DISCDATE");

    this->OrderBy("rbd.VENDOR, rbd.CARRIER, rbd.SEQNO, rbd.CREATEDATE, ORDERNO");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static RBDByCabinInfo* mapRow(Row* row,
                                RBDByCabinInfo* infoPrev)
  {
    RBDByCabinInfo* info(nullptr);
    VendorCode vendor(row->getString(VENDOR));
    CarrierCode carrier(row->getString(CARRIER));
    uint64_t sequenceNo(row->getLong(SEQNO));
    DateTime createDate(row->getDate(CREATEDATE));
    if (infoPrev
        && sequenceNo == infoPrev->sequenceNo()
        && vendor == infoPrev->vendor()
        && carrier == infoPrev->carrier()
        && createDate == infoPrev->createDate())
    {
      info = infoPrev;
    }
    else
    {
      info = new RBDByCabinInfo;
      info->vendor() = vendor;
      info->carrier() = carrier;
      info->sequenceNo() = sequenceNo;
      info->createDate() = createDate;
      if (!row->isNull(EXPIREDATE))
      {
        info->expireDate() = row->getDate(EXPIREDATE);
      }
      if (!row->isNull(EFFDATE))
      {
        info->effDate() = row->getDate(EFFDATE);
      }
      if (!row->isNull(DISCDATE))
      {
        info->discDate() = row->getDate(DISCDATE);
      }
      if (!row->isNull(AREAIND))
      {
        info->globalDir() = row->getString(AREAIND);
      }
      if (!row->isNull(LOC1))
      {
        info->locKey1().loc() = row->getString(LOC1);
      }
      if (!row->isNull(LOC1TYPE))
      {
        info->locKey1().locType() = row->getChar(LOC1TYPE);
      }
      if (!row->isNull(LOC2))
      {
        info->locKey2().loc() = row->getString(LOC2);
      }
      if (!row->isNull(LOC2TYPE))
      {
        info->locKey2().locType() = row->getChar(LOC2TYPE);
      }
      if (!row->isNull(FIRSTTKTDATE))
      {
        info->firstTicketDate() = row->getDate(FIRSTTKTDATE);
      }
      if (!row->isNull(LASTTKTDATE))
      {
        info->lastTicketDate() = row->getDate(LASTTKTDATE);
      }
      if (!row->isNull(FLIGHTNO1))
      {
        info->flightNo1() = atoi(row->getString(FLIGHTNO1));
      }
      if (!row->isNull(FLIGHTNO2))
      {
        info->flightNo2() = atoi(row->getString(FLIGHTNO2));
      }
      if (!row->isNull(EQUIPMENT))
      {
        info->equipmentType() = row->getString(EQUIPMENT);
      }
    }
    char cabin(' ');
    if (!row->isNull(CABIN))
    {
      cabin = row->getChar(CABIN);
    }
    if (cabin != ' ')
    {
      if (!row->isNull(BOOKINGCODE1))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE1)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE2))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE2)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE3))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE3)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE4))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE4)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE5))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE5)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE6))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE6)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE7))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE7)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE8))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE8)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE9))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE9)] = cabin;
      }
      if (!row->isNull(BOOKINGCODE10))
      {
        info->bookingCodeCabinMap()[row->getString(BOOKINGCODE10)] = cabin;
      }
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetRBDByCabinHistoricalSQLStatement
  : public QueryGetRBDByCabinSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    this->Where("rbd.VENDOR = %1q and rbd.CARRIER = %2q and VALIDITYIND = 'Y' "
                "and EXPIREDATE >= %3n and rbd.CREATEDATE <= %4n");

    this->OrderBy("rbd.VENDOR, rbd.CARRIER, rbd.SEQNO, rbd.CREATEDATE, ORDERNO");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllRBDByCabinSQLStatement
  : public QueryGetRBDByCabinSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= rbd.CREATEDATE "
                "and %cd <= EXPIREDATE "
                "and VALIDITYIND = 'Y'");

    this->OrderBy("rbd.VENDOR, rbd.CARRIER, rbd.SEQNO, rbd.CREATEDATE, ORDERNO");
  }

};

} // tse
