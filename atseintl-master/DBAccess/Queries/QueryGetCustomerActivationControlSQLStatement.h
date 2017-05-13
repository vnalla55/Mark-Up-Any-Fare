//----------------------------------------------------------------------------
//          File:           QueryGetCustomerActivationControlSQLStatement.h
//          Description:    QueryGetCustomerActivationControlSQLStatement
//          Created:        05/25/2012
//          Authors:        Jayanthi Shyam Mohan
//
//          Updates:
//
//     © 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/FallbackUtil.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/Queries/QueryGetCustomerActivationControl.h"
#include "DBAccess/Row.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCustomerActivationControlSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCustomerActivationControlSQLStatement() {};
  virtual ~QueryGetCustomerActivationControlSQLStatement() {};

  enum ColumnIndexes
  {
    PROJCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    PSEUDOCITY,
    PROJDESC,
    PROJACTIVATIONIND,
    CRSUSERAPPL,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select a.PROJCODE,a.VERSIONDATE,a.SEQNO,a.CREATEDATE,a.EXPIREDATE, "
        "       a.EFFDATE,a.DISCDATE,a.PSEUDOCITY,a.PROJDESC,a.PROJACTIVATIONIND,a.CRSUSERAPPL");

    this->From("=CUSTOMERACTIVATIONCONTROL a ");
    this->Where("a.PROJCODE = %1q and %cd <= a.EXPIREDATE ");
    this->OrderBy("a.PROJCODE, a.VERSIONDATE, a.SEQNO, a.CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CustomerActivationControl* mapRowToCustomerActivationControl(Row* row)
  {
    CustomerActivationControl* custActivationCntrl = new CustomerActivationControl;

    custActivationCntrl->projCode() = row->getString(PROJCODE);
    custActivationCntrl->versionDate() = row->getDate(VERSIONDATE);
    custActivationCntrl->seqNo() = row->getLong(SEQNO);
    custActivationCntrl->createDate() = row->getDate(CREATEDATE);

    if (!row->isNull(EXPIREDATE))
      custActivationCntrl->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(EFFDATE))
      custActivationCntrl->effDate() = row->getDate(EFFDATE);
    if (!row->isNull(DISCDATE))
      custActivationCntrl->discDate() = row->getDate(DISCDATE);
    if (!row->isNull(PSEUDOCITY))
      custActivationCntrl->pseudoCity() = row->getString(PSEUDOCITY);
    if (!row->isNull(PROJDESC))
      custActivationCntrl->projDesc() = row->getString(PROJDESC);
    if (!row->isNull(PROJACTIVATIONIND))
      custActivationCntrl->projActvInd() = row->getChar(PROJACTIVATIONIND);
    if (!row->isNull(CRSUSERAPPL))
      custActivationCntrl->crsUserAppl() = row->getString(CRSUSERAPPL);

    return custActivationCntrl;
  }

private:
  // Override these functions to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCustomerActivationControlHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCustomerActivationControlHistoricalSQLStatement
    : public QueryGetCustomerActivationControlSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("   a.PROJCODE = %1q "
        "   and %2n <= a.EXPIREDATE"
        "   and %3n >= a.CREATEDATE");
    this->OrderBy("a.PROJCODE, a.VERSIONDATE, a.SEQNO, a.CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetMultiHostActivationAppl
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMultiHostActivationApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMultiHostActivationApplSQLStatement() {};
  virtual ~QueryGetMultiHostActivationApplSQLStatement() {};

  enum ColumnIndexes
  {
    PROJCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    MULTIHOSTCARRIER,
    ACTIVATIONDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select b.PROJCODE,b.VERSIONDATE,b.SEQNO,b.CREATEDATE,b.MULTIHOSTCARRIER, "
                  "       b.ACTIVATIONDATE");

    this->From("=MULTIHOSTACTIVATIONAPPL b");
    this->Where("    b.PROJCODE = %1q "
                "and b.VERSIONDATE = %2n "
                "and b.SEQNO = %3n "
                "and b.CREATEDATE = %4n ");

    this->OrderBy("b.PROJCODE, b.VERSIONDATE, b.SEQNO, b.CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void
  mapRowToMultiHostActivationAppl(Row* row, CustomerActivationControl* customerActivationControl)
  {

    CustomerActivationControl::MultiHostActivation* mhActv =
        new CustomerActivationControl::MultiHostActivation;

    std::string projCode = row->getString(PROJCODE);
    DateTime versionDate = row->getDate(VERSIONDATE);
    int64_t seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    if (customerActivationControl != nullptr && customerActivationControl->projCode() == projCode &&
        customerActivationControl->versionDate() == versionDate &&
        customerActivationControl->seqNo() == seqNo &&
        customerActivationControl->createDate() == createDate)
    {
      if (!row->isNull(MULTIHOSTCARRIER))
        mhActv->mhCxr() = row->getString(MULTIHOSTCARRIER);

      if (!row->isNull(ACTIVATIONDATE))
        mhActv->mhCxrActDate() = row->getDate(ACTIVATIONDATE);

      customerActivationControl->multiHostActivation().push_back(mhActv);
    }
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMultiHostActivationApplHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMultiHostActivationApplHistoricalSQLStatement
    : public QueryGetMultiHostActivationApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetCarrierActivationAppl
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCarrierActivationApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierActivationApplSQLStatement() {};
  virtual ~QueryGetCarrierActivationApplSQLStatement() {};

  enum ColumnIndexes
  {
    PROJCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    CARRIER,
    ACTIVATIONDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select c.PROJCODE,c.VERSIONDATE,c.SEQNO,c.CREATEDATE,c.CARRIER, "
                  "       c.ACTIVATIONDATE");

    this->From("=CARRIERACTIVATIONAPPL c");
    this->Where("    c.PROJCODE = %1q "
                "and c.VERSIONDATE = %2n "
                "and c.SEQNO = %3n "
                "and c.CREATEDATE = %4n ");

    this->OrderBy("c.PROJCODE, c.VERSIONDATE, c.SEQNO, c.CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void
  mapRowToCarrierActivationAppl(Row* row, CustomerActivationControl* customerActivationControl)
  {

    CustomerActivationControl::CarrierActivation* cxrActv =
        new CustomerActivationControl::CarrierActivation;

    std::string projCode = row->getString(PROJCODE);
    DateTime versionDate = row->getDate(VERSIONDATE);
    int64_t seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    if (customerActivationControl != nullptr && customerActivationControl->projCode() == projCode &&
        customerActivationControl->versionDate() == versionDate &&
        customerActivationControl->seqNo() == seqNo &&
        customerActivationControl->createDate() == createDate)
    {
      if (!row->isNull(CARRIER))
        cxrActv->cxr() = row->getString(CARRIER);

      if (!row->isNull(ACTIVATIONDATE))
        cxrActv->cxrActDate() = row->getDate(ACTIVATIONDATE);

      customerActivationControl->cxrActivation().push_back(cxrActv);
    }
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCarrierActivationApplHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCarrierActivationApplHistoricalSQLStatement
    : public QueryGetCarrierActivationApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetGeoLocationAppl
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGeoLocationApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGeoLocationApplSQLStatement() {};
  virtual ~QueryGetGeoLocationApplSQLStatement() {};

  enum ColumnIndexes
  {
    PROJCODE = 0,
    SEQNO,
    VERSIONDATE,
    CREATEDATE,
    LOCTYPE,
    LOC,
    ACTIVATIONDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select d.PROJCODE,d.SEQNO,d.VERSIONDATE,d.CREATEDATE,d.LOCTYPE, "
                  "       d.LOC,d.ACTIVATIONDATE");

    this->From("=GEOLOCATIONAPPL d");
    this->Where("    d.PROJCODE = %1q "
                "and d.VERSIONDATE = %2n "
                "and d.SEQNO = %3n "
                "and d.CREATEDATE = %4n ");

    this->OrderBy("d.PROJCODE, d.SEQNO, d.VERSIONDATE, d.CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template

    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void
  mapRowToGeoLocationAppl(Row* row, CustomerActivationControl* customerActivationControl)
  {
    CustomerActivationControl::GeoActivation* geoActv =
        new CustomerActivationControl::GeoActivation;

    std::string projCode = row->getString(PROJCODE);
    DateTime versionDate = row->getDate(VERSIONDATE);
    int64_t seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    if (customerActivationControl != nullptr && customerActivationControl->projCode() == projCode &&
        customerActivationControl->versionDate() == versionDate &&
        customerActivationControl->seqNo() == seqNo &&
        customerActivationControl->createDate() == createDate)
    {
      if (!row->isNull(LOCTYPE))
        geoActv->locType() = row->getChar(LOCTYPE);
      if (!row->isNull(LOC))
        geoActv->loc() = row->getString(LOC);
      if (!row->isNull(ACTIVATIONDATE))
        geoActv->locActivationDate() = row->getDate(ACTIVATIONDATE);

      customerActivationControl->geoActivation().push_back(geoActv);
    }
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetGeoLocationApplHistoricalSQLStatement
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGeoLocationApplHistoricalSQLStatement
    : public QueryGetGeoLocationApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override {}
};

} // tse
