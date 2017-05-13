//----------------------------------------------------------------------------
//          File:           QueryGetSalesRestrictionSQLStatement.h
//          Description:    QueryGetSalesRestrictionSQLStatement
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
#include "DBAccess/Queries/QueryGetSalesRestriction.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSalesRestrictionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSalesRestrictionSQLStatement() {};
  virtual ~QueryGetSalesRestrictionSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    UNAVAILTAG,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    SEGCNT,
    EARLIESTRESDATE,
    EARLIESTTKTDATE,
    LATESTRESDATE,
    LATESTTKTDATE,
    COUNTRYREST,
    RESIDENTREST,
    CARRIERCRSIND,
    OTHERCARRIER,
    VALIDATIONIND,
    CARRIERSEGIND,
    TVLAGENTSALEIND,
    TVLAGENTSELECTEDIND,
    FOPCASHIND,
    FOPCHECKIND,
    FOPCREDITIND,
    TKTISSMECH,
    FOPGTRIND,
    CURRCNTRYIND,
    CURR,
    TKTISSMAIL,
    TKTISSPTA,
    TKTISSSELF,
    TKTISSPTATKT,
    TKTISSAUTO,
    TKTISSSAT,
    TKTISSSATOCATO,
    TKTISSELECTRONIC,
    TKTISSSITI,
    TKTISSSOTO,
    TKTISSSITO,
    TKTISSSOTI,
    FAMILYGRPIND,
    EXTENDIND,
    ORDERNO,
    LOCAPPL,
    INHIBIT,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select sr.VENDOR,sr.ITEMNO,sr.CREATEDATE,UNAVAILTAG,EXPIREDATE,"
        "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,SEGCNT,EARLIESTRESDATE,"
        "       EARLIESTTKTDATE,LATESTRESDATE,LATESTTKTDATE,COUNTRYREST,"
        "       RESIDENTREST,CARRIERCRSIND,OTHERCARRIER,VALIDATIONIND,"
        "       CARRIERSEGIND,TVLAGENTSALEIND,TVLAGENTSELECTEDIND,FOPCASHIND,"
        "       FOPCHECKIND,FOPCREDITIND,TKTISSMECH,FOPGTRIND,CURRCNTRYIND,CURR,"
        "       TKTISSMAIL,TKTISSPTA,TKTISSSELF,TKTISSPTATKT,TKTISSAUTO,TKTISSSAT,"
        "       TKTISSSATOCATO,TKTISSELECTRONIC,TKTISSSITI,TKTISSSOTO,TKTISSSITO,"
        "       TKTISSSOTI,FAMILYGRPIND,EXTENDIND,ORDERNO,LOCAPPL,INHIBIT,"
        "       LOC1TYPE,(CASE when LOC1TYPE in ('I','H') then substr(LOC1,-7) else LOC1 end) LOC1,"
        "       LOC2TYPE,(CASE when LOC2TYPE in ('I','H') then substr(LOC2,-7) else LOC2 end) "
        "LOC2");

    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=SALESREST", "sr", "LEFT OUTER JOIN", "=LOCALE", "l", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("sr.VENDOR = %1q"
                "    and sr.ITEMNO = %2n"
                "    and %cd <= EXPIREDATE"
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, LOCAPPL, LOC1TYPE, LOC1, LOC2TYPE, LOC2, CREATEDATE, ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SalesRestriction* mapRowToSalesRestriction(Row* row, SalesRestriction* srPrev)
  { // Get Join (Determinant) fields
    VendorCode vendor = row->getString(VENDOR);
    uint32_t itemNo = static_cast<uint32_t>(row->getInt(ITEMNO));
    DateTime createDate = row->getDate(CREATEDATE);

    SalesRestriction* sr;

    // If Parent hasn't changed, add to Child (segs)
    if (srPrev != nullptr && srPrev->vendor() == vendor && srPrev->itemNo() == itemNo &&
        srPrev->createDate() == createDate)
    { // Create new Segment to add to Parent's vector
      sr = srPrev;
    }
    else
    { // Time for a new Parent
      sr = new tse::SalesRestriction;
      sr->vendor() = vendor;
      sr->itemNo() = itemNo;
      sr->createDate() = createDate;

      sr->expireDate() = row->getDate(EXPIREDATE);

      sr->unavailTag() = row->getChar(UNAVAILTAG);
      sr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
      sr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
      sr->segCnt() = row->getInt(SEGCNT);

      sr->earliestResDate() = row->getDate(EARLIESTRESDATE);
      sr->earliestTktDate() = row->getDate(EARLIESTTKTDATE);
      sr->latestResDate() = row->getDate(LATESTRESDATE);
      sr->latestTktDate() = row->getDate(LATESTTKTDATE);

      sr->countryRest() = row->getChar(COUNTRYREST);
      sr->residentRest() = row->getChar(RESIDENTREST);
      sr->carrierCrsInd() = row->getChar(CARRIERCRSIND);
      sr->otherCarrier() = row->getString(OTHERCARRIER);
      sr->validationInd() = row->getChar(VALIDATIONIND);
      sr->carrierSegInd() = row->getChar(CARRIERSEGIND);
      sr->tvlAgentSaleInd() = row->getChar(TVLAGENTSALEIND);
      sr->tvlAgentSelectedInd() = row->getChar(TVLAGENTSELECTEDIND);
      sr->fopCashInd() = row->getChar(FOPCASHIND);
      sr->fopCheckInd() = row->getChar(FOPCHECKIND);
      sr->fopCreditInd() = row->getChar(FOPCREDITIND);
      sr->fopGtrInd() = row->getChar(FOPGTRIND);
      sr->currCntryInd() = row->getChar(CURRCNTRYIND);
      sr->curr() = row->getString(CURR);
      sr->tktIssMail() = row->getChar(TKTISSMAIL);
      sr->tktIssPta() = row->getChar(TKTISSPTA);
      sr->tktIssMech() = row->getChar(TKTISSMECH);
      sr->tktIssSelf() = row->getChar(TKTISSSELF);
      sr->tktIssPtaTkt() = row->getChar(TKTISSPTATKT);
      sr->tktIssAuto() = row->getChar(TKTISSAUTO);
      sr->tktIssSat() = row->getChar(TKTISSSAT);
      sr->tktIssSatOcAto() = row->getChar(TKTISSSATOCATO);
      sr->tktIssElectronic() = row->getChar(TKTISSELECTRONIC);
      sr->tktIssSiti() = row->getChar(TKTISSSITI);
      sr->tktIssSoto() = row->getChar(TKTISSSOTO);
      sr->tktIssSito() = row->getChar(TKTISSSITO);
      sr->tktIssSoti() = row->getChar(TKTISSSOTI);
      sr->familyGrpInd() = row->getChar(FAMILYGRPIND);
      sr->extendInd() = row->getChar(EXTENDIND);
      sr->inhibit() = row->getChar(INHIBIT);
    }

    if (!row->isNull(ORDERNO))
    { // Got a Child!
      Locale* newLoc = new Locale;
      newLoc->orderNo() = row->getInt(ORDERNO);
      newLoc->locAppl() = row->getChar(LOCAPPL);

      LocKey* loc = &newLoc->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &newLoc->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);
      sr->locales().push_back(newLoc);
    }
    return sr;
  } // mapRowToSalesRestriction()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetSalesRestrictionSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSalesRestrictionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSalesRestrictionHistoricalSQLStatement
    : public QueryGetSalesRestrictionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "  select sh.VENDOR,sh.ITEMNO,sh.CREATEDATE,UNAVAILTAG,EXPIREDATE,"
        "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,SEGCNT,EARLIESTRESDATE,"
        "       EARLIESTTKTDATE,LATESTRESDATE,LATESTTKTDATE,COUNTRYREST,"
        "       RESIDENTREST,CARRIERCRSIND,OTHERCARRIER,VALIDATIONIND,"
        "       CARRIERSEGIND,TVLAGENTSALEIND,TVLAGENTSELECTEDIND,FOPCASHIND,"
        "       FOPCHECKIND,FOPCREDITIND,TKTISSMECH,FOPGTRIND,CURRCNTRYIND,CURR,"
        "       TKTISSMAIL,TKTISSPTA,TKTISSSELF,TKTISSPTATKT,TKTISSAUTO,TKTISSSAT,"
        "       TKTISSSATOCATO,TKTISSELECTRONIC,TKTISSSITI,TKTISSSOTO,TKTISSSITO,"
        "       TKTISSSOTI,FAMILYGRPIND,EXTENDIND,ORDERNO,LOCAPPL,INHIBIT,"
        "       LOC1TYPE,(CASE when LOC1TYPE in ('I','H') then substr(LOC1,-7) else LOC1 end) LOC1,"
        "       LOC2TYPE,(CASE when LOC2TYPE in ('I','H') then substr(LOC2,-7) else LOC2 end) "
        "LOC2");
    std::string fromh;
    std::vector<std::string> joinFieldsh;
    joinFieldsh.reserve(3);
    joinFieldsh.push_back("VENDOR");
    joinFieldsh.push_back("ITEMNO");
    joinFieldsh.push_back("CREATEDATE");
    this->generateJoinString(
        "=SALESRESTH", "sh", "LEFT OUTER JOIN", "=LOCALEH", "lh", joinFieldsh, fromh);
    partialStatement.From(fromh);
    partialStatement.Where("sh.VENDOR = %1q "
                           " and sh.ITEMNO = %2n"
                           " and sh.VALIDITYIND = 'Y'"
                           " and %3n <= sh.EXPIREDATE"
                           " and %4n >= sh.CREATEDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        "  select sr.VENDOR,sr.ITEMNO,sr.CREATEDATE,UNAVAILTAG,EXPIREDATE,"
        "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,SEGCNT,EARLIESTRESDATE,"
        "       EARLIESTTKTDATE,LATESTRESDATE,LATESTTKTDATE,COUNTRYREST,"
        "       RESIDENTREST,CARRIERCRSIND,OTHERCARRIER,VALIDATIONIND,"
        "       CARRIERSEGIND,TVLAGENTSALEIND,TVLAGENTSELECTEDIND,FOPCASHIND,"
        "       FOPCHECKIND,FOPCREDITIND,TKTISSMECH,FOPGTRIND,CURRCNTRYIND,CURR,"
        "       TKTISSMAIL,TKTISSPTA,TKTISSSELF,TKTISSPTATKT,TKTISSAUTO,TKTISSSAT,"
        "       TKTISSSATOCATO,TKTISSELECTRONIC,TKTISSSITI,TKTISSSOTO,TKTISSSITO,"
        "       TKTISSSOTI,FAMILYGRPIND,EXTENDIND,ORDERNO,LOCAPPL,INHIBIT,"
        "       LOC1TYPE,(CASE when LOC1TYPE in ('I','H') then substr(LOC1,-7) else LOC1 end) LOC1,"
        "       LOC2TYPE,(CASE when LOC2TYPE in ('I','H') then substr(LOC2,-7) else LOC2 end) "
        "LOC2");
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(3);
    joinFields.push_back("VENDOR");
    joinFields.push_back("ITEMNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=SALESREST", "sr", "LEFT OUTER JOIN", "=LOCALE", "l", joinFields, from);
    partialStatement.From(from);
    partialStatement.Where("sr.VENDOR = %5q"
                           "    and sr.ITEMNO = %6n"
                           "    and VALIDITYIND = 'Y'"
                           "    and %7n <= sr.EXPIREDATE"
                           "    and %8n >= sr.CREATEDATE");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, LOCAPPL, LOC1TYPE, LOC1, LOC2TYPE, LOC2, CREATEDATE, ORDERNO");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetSalesRestrictionHistoricalSQLStatement
} // tse
