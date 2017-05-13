//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayPrefSQLStatement.h
//          Description:    QueryGetFareDisplayPrefSQLStatement
//          Created:        11/02/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareDisplayPref.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDisplayPrefSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDisplayPrefSQLStatement() {};
  virtual ~QueryGetFareDisplayPrefSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    SSGGROUPNO,
    CREATEDATE,
    LOCKDATE,
    MEMONO,
    CREATORID,
    CREATORBUSINESSUNIT,
    SINGLECXRTEMPLATEID,
    MULTICXRTEMPLATEID,
    SHOWROUTINGS,
    DOUBLEFORROUNDTRIP,
    DISPLAYHALFROUNDTRIP,
    SAMEFAREBASISSAMELINE,
    RETURNDATEVALIDATION,
    NOFUTURESALESDATE,
    SINGLECARRIERSVCSCHED,
    MULTICARRIERSVCSCHED,
    TAXTEMPLATEID,
    ADDONTEMPLATEID,
    JOURNEYIND,
    VALIDATELOCALEFORPUBLFARE,
    APPLYDOWVALIDATIONTOOWFARES,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,CREATEDATE,"
                  "       LOCKDATE,MEMONO,CREATORID,CREATORBUSINESSUNIT,SINGLECXRTEMPLATEID,"
                  "       MULTICXRTEMPLATEID,SHOWROUTINGS,DOUBLEFORROUNDTRIP,"
                  "       DISPLAYHALFROUNDTRIP,SAMEFAREBASISSAMELINE,RETURNDATEVALIDATION,"
                  "       NOFUTURESALESDATE,SINGLECARRIERSVCSCHED,MULTICARRIERSVCSCHED,"
                  "       TAXTEMPLATEID,ADDONTEMPLATEID,JOURNEYIND,VALIDATELOCALEFORPUBLFARE,"
                  "       APPLYDOWVALIDATIONTOOWFARES");
    this->From(" =FAREDISPLAYPREF");
    this->Where("USERAPPLTYPE = %1q "
                "    and USERAPPL = %2q "
                "    and PSEUDOCITYTYPE = %3q "
                "    and PSEUDOCITY = %4q "
                "    and SSGGROUPNO = %5n ");

    if (DataManager::forceSortOrder())
      this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDisplayPref* mapRowToFareDisplayPref(Row* row)
  {
    tse::FareDisplayPref* fdp = new tse::FareDisplayPref;

    fdp->userApplType() = row->getChar(USERAPPLTYPE);
    fdp->userAppl() = row->getString(USERAPPL);
    fdp->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    fdp->pseudoCity() = row->getString(PSEUDOCITY);
    fdp->ssgGroupNo() = row->getInt(SSGGROUPNO);
    fdp->createDate() = row->getDate(CREATEDATE);
    fdp->lockDate() = row->getDate(LOCKDATE);
    fdp->memoNo() = row->getInt(MEMONO);
    fdp->creatorId() = row->getString(CREATORID);
    fdp->creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);
    fdp->singleCxrTemplateId() = row->getString(SINGLECXRTEMPLATEID);
    fdp->multiCxrTemplateId() = row->getString(MULTICXRTEMPLATEID);
    fdp->showRoutings() = row->getChar(SHOWROUTINGS);
    fdp->doubleForRoundTrip() = row->getChar(DOUBLEFORROUNDTRIP);
    fdp->displayHalfRoundTrip() = row->getChar(DISPLAYHALFROUNDTRIP);
    fdp->sameFareBasisSameLine() = row->getChar(SAMEFAREBASISSAMELINE);
    fdp->returnDateValidation() = row->getChar(RETURNDATEVALIDATION);
    fdp->noFutureSalesDate() = row->getChar(NOFUTURESALESDATE);
    fdp->singleCarrierSvcSched() = row->getChar(SINGLECARRIERSVCSCHED);
    fdp->multiCarrierSvcSched() = row->getChar(MULTICARRIERSVCSCHED);
    fdp->taxTemplateId() = row->getString(TAXTEMPLATEID);
    fdp->addOnTemplateId() = row->getString(ADDONTEMPLATEID);
    fdp->journeyInd() = row->getChar(JOURNEYIND);
    fdp->validateLocaleForPublFares() = row->getChar(VALIDATELOCALEFORPUBLFARE);
    fdp->applyDOWvalidationToOWFares() = row->getChar(APPLYDOWVALIDATIONTOOWFARES);

    return fdp;
  } // mapRowToFareDisplayPref()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDisplayPref
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDisplayPrefSQLStatement
    : public QueryGetFareDisplayPrefSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY, SSGGROUPNO");
  }
};
} // tse
