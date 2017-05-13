//----------------------------------------------------------------------------
//          File:           QueryGetMinFareDefaultLogicSQLStatement.h
//          Description:    QueryGetMinFareDefaultLogicSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetMinFareDefaultLogic.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMinFareDefaultLogicSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareDefaultLogicSQLStatement() {};
  virtual ~QueryGetMinFareDefaultLogicSQLStatement() {};

  enum ColumnIndexes
  {
    GOVERNINGCARRIER = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NMLHIPTARIFFCATIND,
    NMLCTMTARIFFCATIND,
    VENDOR,
    USERAPPLTYPE,
    USERAPPL,
    DIRECTIONALIND,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    DOMAPPL,
    DOMEXCEPTIND,
    DOMLOCTYPE,
    DOMLOC,
    NMLFARECOMPAREIND,
    NMLMPMBEFORERTGIND,
    NMLRTGBEFOREMPMIND,
    NMLHIPRESTRCOMPIND,
    NMLHIPUNRESTRCOMPIND,
    NMLHIPRBDCOMPIND,
    NMLHIPSTOPCOMPIND,
    NMLHIPORIGIND,
    NMLHIPORIGNATIONIND,
    NMLHIPFROMINTERIND,
    NMLHIPDESTIND,
    NMLHIPDESTNATIONIND,
    NMLHIPTOINTERIND,
    SPCLHIPTARIFFCATIND,
    SPCLHIPRULETRFIND,
    SPCLHIPFARECLASSIND,
    SPCLHIP1STCHARIND,
    SPCLHIPSTOPCOMPIND,
    SPCLHIPSPCLONLYIND,
    SPCLHIPLOCTYPE,
    SPCLHIPLOC,
    SPCLHIPORIGIND,
    SPCLHIPORIGNATIONIND,
    SPCLHIPFROMINTERIND,
    SPCLHIPDESTIND,
    SPCLHIPDESTNATIONIND,
    SPECIALPROCESSNAME,
    SPCLHIPTOINTERIND,
    NMLCTMRESTRCOMPIND,
    NMLCTMUNRESTRCOMPIND,
    NMLCTMRBDCOMPIND,
    NMLCTMSTOPCOMPIND,
    NMLCTMORIGIND,
    NMLCTMDESTNATIONIND,
    NMLCTMTOINTERIND,
    SPCLCTMTARIFFCATIND,
    SPCLCTMRULETRFIND,
    SPCLCTMFARECLASSIND,
    SPCLSAME1STCHARFBIND2,
    SPCLCTMSTOPCOMPIND,
    SPCLCTMMKTCOMP,
    SPCLCTMORIGIND,
    SPCLCTMDESTNATIONIND,
    SPCLCTMTOINTERIND,
    CPMEXCL,
    DOMFARETYPEEXCEPT,
    NMLHIPORIGNATIONTKTPT,
    NMLHIPORIGNATIONSTOPPT,
    NMLHIPSTOPOVERPT,
    NMLHIPTICKETEDPT,
    SPCLHIPORIGNATIONTKTPT,
    SPCLHIPORIGNATIONSTOPPT,
    SPCLHIPSTOPOVERPT,
    SPCLHIPTICKETEDPT,
    NMLHIPNOINTERTOINTER,
    SPCLHIPNOINTERTOINTER,
    FARETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select dl.GOVERNINGCARRIER,dl.VERSIONDATE,dl.SEQNO,dl.CREATEDATE,EXPIREDATE,EFFDATE,"
        " DISCDATE,NMLHIPTARIFFCATIND,NMLCTMTARIFFCATIND,VENDOR,USERAPPLTYPE,USERAPPL,"
        " DIRECTIONALIND,LOC1TYPE,LOC1,LOC2TYPE,LOC2,DOMAPPL,DOMEXCEPTIND,DOMLOCTYPE,"
        " DOMLOC,NMLFARECOMPAREIND,NMLMPMBEFORERTGIND,NMLRTGBEFOREMPMIND,NMLHIPRESTRCOMPIND,"
        " NMLHIPUNRESTRCOMPIND,NMLHIPRBDCOMPIND,NMLHIPSTOPCOMPIND,NMLHIPORIGIND,"
        " NMLHIPORIGNATIONIND,NMLHIPFROMINTERIND,NMLHIPDESTIND,NMLHIPDESTNATIONIND,"
        " NMLHIPTOINTERIND,SPCLHIPTARIFFCATIND,SPCLHIPRULETRFIND,SPCLHIPFARECLASSIND,"
        " SPCLHIP1STCHARIND,SPCLHIPSTOPCOMPIND,SPCLHIPSPCLONLYIND,SPCLHIPLOCTYPE,SPCLHIPLOC,"
        " SPCLHIPORIGIND,SPCLHIPORIGNATIONIND,SPCLHIPFROMINTERIND,SPCLHIPDESTIND,"
        " SPCLHIPDESTNATIONIND,SPECIALPROCESSNAME,SPCLHIPTOINTERIND,NMLCTMRESTRCOMPIND,"
        " NMLCTMUNRESTRCOMPIND,NMLCTMRBDCOMPIND,NMLCTMSTOPCOMPIND,NMLCTMORIGIND,"
        " NMLCTMDESTNATIONIND,NMLCTMTOINTERIND,SPCLCTMTARIFFCATIND,SPCLCTMRULETRFIND,"
        " SPCLCTMFARECLASSIND,SPCLSAME1STCHARFBIND2,SPCLCTMSTOPCOMPIND,SPCLCTMMKTCOMP,"
        " SPCLCTMORIGIND,SPCLCTMDESTNATIONIND,SPCLCTMTOINTERIND,CPMEXCL,DOMFARETYPEEXCEPT,"
        " NMLHIPORIGNATIONTKTPT,NMLHIPORIGNATIONSTOPPT,NMLHIPSTOPOVERPT,NMLHIPTICKETEDPT,"
        " SPCLHIPORIGNATIONTKTPT,SPCLHIPORIGNATIONSTOPPT,SPCLHIPSTOPOVERPT,"
        " SPCLHIPTICKETEDPT,NMLHIPNOINTERTOINTER,SPCLHIPNOINTERTOINTER,dls.FARETYPE");

    //		        this->From("=MINFAREDEFAULTLOGIC dl LEFT OUTER JOIN =MINFAREDEFAULTLOGICSEG dls"
    //		                   "     USING (GOVERNINGCARRIER,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("GOVERNINGCARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=MINFAREDEFAULTLOGIC",
                             "dl",
                             "LEFT OUTER JOIN",
                             "=MINFAREDEFAULTLOGICSEG",
                             "dls",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("dl.GOVERNINGCARRIER = %1q "
                " and %cd <= EXPIREDATE");

    this->OrderBy("GOVERNINGCARRIER,SEQNO,FARETYPE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MinFareDefaultLogic*
  mapRowToMinFareDefaultLogic(Row* row, MinFareDefaultLogic* dlPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode govCxr = row->getString(GOVERNINGCARRIER);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    MinFareDefaultLogic* dl;

    if (dlPrev != nullptr && dlPrev->vendor() == vendor && dlPrev->governingCarrier() == govCxr &&
        dlPrev->versionDate() == versionDate && dlPrev->seqNo() == seqNo &&
        dlPrev->createDate() == createDate)
    { // Add to Prev
      dl = dlPrev;
    }
    else
    { // Time for a new Parent
      dl = new tse::MinFareDefaultLogic;
      dl->vendor() = vendor;
      dl->governingCarrier() = govCxr;
      dl->versionDate() = versionDate;
      dl->seqNo() = seqNo;
      dl->createDate() = createDate;
      dl->expireDate() = row->getDate(EXPIREDATE);
      dl->effDate() = row->getDate(EFFDATE);
      dl->discDate() = row->getDate(DISCDATE);
      dl->nmlHipTariffCatInd() = row->getInt(NMLHIPTARIFFCATIND);
      dl->nmlCtmTariffCatInd() = row->getInt(NMLCTMTARIFFCATIND);
      dl->userApplType() = row->getChar(USERAPPLTYPE);
      dl->userAppl() = row->getString(USERAPPL);

      std::string dir = row->getString(DIRECTIONALIND);
      if (dir == "F")
        dl->directionalInd() = FROM;
      else if (dir == "W")
        dl->directionalInd() = WITHIN;
      else if (dir == "O")
        dl->directionalInd() = ORIGIN;
      else if (dir == "X")
        dl->directionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        dl->directionalInd() = BETWEEN;

      LocKey* loc = &dl->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &dl->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      dl->domAppl() = row->getChar(DOMAPPL);
      dl->domExceptInd() = row->getChar(DOMEXCEPTIND);

      loc = &dl->domLoc();
      loc->locType() = row->getChar(DOMLOCTYPE);
      loc->loc() = row->getString(DOMLOC);

      dl->nmlFareCompareInd() = row->getChar(NMLFARECOMPAREIND);
      dl->nmlMpmBeforeRtgInd() = row->getChar(NMLMPMBEFORERTGIND);
      dl->nmlRtgBeforeMpmInd() = row->getChar(NMLRTGBEFOREMPMIND);
      dl->nmlHipRestrCompInd() = row->getChar(NMLHIPRESTRCOMPIND);
      dl->nmlHipUnrestrCompInd() = row->getChar(NMLHIPUNRESTRCOMPIND);
      dl->nmlHipRbdCompInd() = row->getChar(NMLHIPRBDCOMPIND);
      dl->nmlHipStopCompInd() = row->getChar(NMLHIPSTOPCOMPIND);
      dl->nmlHipOrigInd() = row->getChar(NMLHIPORIGIND);
      dl->nmlHipOrigNationInd() = row->getChar(NMLHIPORIGNATIONIND);
      dl->nmlHipFromInterInd() = row->getChar(NMLHIPFROMINTERIND);
      dl->nmlHipDestInd() = row->getChar(NMLHIPDESTIND);
      dl->nmlHipDestNationInd() = row->getChar(NMLHIPDESTNATIONIND);
      dl->nmlHipToInterInd() = row->getChar(NMLHIPTOINTERIND);
      dl->nmlHipExemptInterToInter() = row->getChar(NMLHIPNOINTERTOINTER);
      dl->spclHipTariffCatInd() = row->getChar(SPCLHIPTARIFFCATIND);
      dl->spclHipRuleTrfInd() = row->getChar(SPCLHIPRULETRFIND);
      dl->spclHipFareClassInd() = row->getChar(SPCLHIPFARECLASSIND);
      dl->spclHip1stCharInd() = row->getChar(SPCLHIP1STCHARIND);
      dl->spclHipStopCompInd() = row->getChar(SPCLHIPSTOPCOMPIND);
      dl->spclHipSpclOnlyInd() = row->getChar(SPCLHIPSPCLONLYIND);

      loc = &dl->spclHipLoc();
      loc->locType() = row->getChar(SPCLHIPLOCTYPE);
      loc->loc() = row->getString(SPCLHIPLOC);

      dl->spclHipOrigInd() = row->getChar(SPCLHIPORIGIND);
      dl->spclHipOrigNationInd() = row->getChar(SPCLHIPORIGNATIONIND);
      dl->spclHipFromInterInd() = row->getChar(SPCLHIPFROMINTERIND);
      dl->spclHipDestInd() = row->getChar(SPCLHIPDESTIND);
      dl->spclHipDestNationInd() = row->getChar(SPCLHIPDESTNATIONIND);
      dl->specialProcessName() = row->getString(SPECIALPROCESSNAME);
      dl->spclHipToInterInd() = row->getChar(SPCLHIPTOINTERIND);
      dl->spclHipExemptInterToInter() = row->getChar(SPCLHIPNOINTERTOINTER);
      dl->nmlCtmRestrCompInd() = row->getChar(NMLCTMRESTRCOMPIND);
      dl->nmlCtmUnrestrCompInd() = row->getChar(NMLCTMUNRESTRCOMPIND);
      dl->nmlCtmRbdCompInd() = row->getChar(NMLCTMRBDCOMPIND);
      dl->nmlCtmStopCompInd() = row->getChar(NMLCTMSTOPCOMPIND);
      dl->nmlCtmOrigInd() = row->getChar(NMLCTMORIGIND);
      dl->nmlCtmDestNationInd() = row->getChar(NMLCTMDESTNATIONIND);
      dl->nmlCtmToInterInd() = row->getChar(NMLCTMTOINTERIND);
      dl->spclCtmTariffCatInd() = row->getChar(SPCLCTMTARIFFCATIND);
      dl->spclCtmRuleTrfInd() = row->getChar(SPCLCTMRULETRFIND);
      dl->spclCtmFareClassInd() = row->getChar(SPCLCTMFARECLASSIND);
      dl->spclSame1stCharFBInd2() = row->getChar(SPCLSAME1STCHARFBIND2);
      dl->spclCtmStopCompInd() = row->getChar(SPCLCTMSTOPCOMPIND);
      dl->spclCtmMktComp() = row->getChar(SPCLCTMMKTCOMP);
      dl->spclCtmOrigInd() = row->getChar(SPCLCTMORIGIND);
      dl->spclCtmDestNationInd() = row->getChar(SPCLCTMDESTNATIONIND);
      dl->spclCtmToInterInd() = row->getChar(SPCLCTMTOINTERIND);
      dl->cpmExcl() = row->getChar(CPMEXCL);
      dl->domFareTypeExcept() = row->getChar(DOMFARETYPEEXCEPT);
      dl->nmlHipOrigNationTktPt() = row->getChar(NMLHIPORIGNATIONTKTPT);
      dl->nmlHipOrigNationStopPt() = row->getChar(NMLHIPORIGNATIONSTOPPT);
      dl->nmlHipStopoverPt() = row->getChar(NMLHIPSTOPOVERPT);
      dl->nmlHipTicketedPt() = row->getChar(NMLHIPTICKETEDPT);
      dl->spclHipOrigNationTktPt() = row->getChar(SPCLHIPORIGNATIONTKTPT);
      dl->spclHipOrigNationStopPt() = row->getChar(SPCLHIPORIGNATIONSTOPPT);
      dl->spclHipStopoverPt() = row->getChar(SPCLHIPSTOPOVERPT);
      dl->spclHipTicketedPt() = row->getChar(SPCLHIPTICKETEDPT);
    } // New Parent

    // Check for Child
    if (!row->isNull(FARETYPE))
    {
      FareTypeAbbrev fta = row->getString(FARETYPE);
      dl->fareTypes().push_back(fta);
    }

    return dl;
  } // mapRowToMinFareDefaultLogic()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareDefaultLogicHistoricalSQLStatement
    : public QueryGetMinFareDefaultLogicSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("(dl.GOVERNINGCARRIER = %1q or dl.GOVERNINGCARRIER = '')"
                " and (dl.VENDOR = %2q or dl.VENDOR = '')"
                " and %3n <= dl.EXPIREDATE"
                " and (%4n >= dl.CREATEDATE or %5n >= dl.EFFDATE)");
    this->OrderBy("GOVERNINGCARRIER desc, VENDOR desc, SEQNO desc, VERSIONDATE desc, CREATEDATE "
                  "desc, FARETYPE desc");
  }
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllMinFareDefaultLogicSQLStatement
    : public QueryGetMinFareDefaultLogicSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("GOVERNINGCARRIER,SEQNO,FARETYPE");
  }
}; // class QueryGetAllMinFareDefaultLogicSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllMinFareDefaultLogicHistoricalSQLStatement
    : public QueryGetMinFareDefaultLogicSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("GOVERNINGCARRIER desc, VENDOR desc, SEQNO desc, VERSIONDATE desc, CREATEDATE "
                  "desc, FARETYPE desc");
  }
}; // class QueryGetAllMinFareDefaultLogicHistoricalSQLStatement
} // tse

