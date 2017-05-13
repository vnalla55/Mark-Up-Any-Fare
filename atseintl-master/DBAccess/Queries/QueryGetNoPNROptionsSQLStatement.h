//----------------------------------------------------------------------------
//          File:           QueryGetNoPNROptionsSQLStatement.h
//          Description:    QueryGetNoPNROptionsSQLStatement
//          Created:        1/11/2008
//          Authors:        Karolina Golebiewska
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
#include "DBAccess/Queries/QueryGetNoPNROptions.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNoPNROptionsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNoPNROptionsSQLStatement() {};
  virtual ~QueryGetNoPNROptionsSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    LOC1TYPE,
    LOC1,
    WQNOTPERMITTED,
    MAXNOOPTIONS,
    WQSORT,
    WQDUPLICATEAMOUNTS,
    FARELINEHEADERFORMAT,
    PASSENGERDETAILLINEFORMAT,
    FARELINEPTC,
    PRIMEPTCREFNO,
    SECONDARYPTCREFNO,
    FARELINEPTCBREAK,
    PASSENGERDETAILPTCBREAK,
    NEGPASSENGERTYPEMAPPING,
    NOMATCHOPTIONSDISPLAY,
    ALLMATCHTRAILERMESSAGE,
    MATCHINTEGRATEDTRAILER,
    ACCOMPANIEDTVLTRAILERMSG,
    RBDMATCHTRAILERMSG,
    RBDNOMATCHTRAILERMSG,
    RBDNOMATCHTRAILERMSG2,
    DISPLAYFARERULEWARNINGMSG,
    DISPLAYFARERULEWARNINGMSG2,
    DISPLAYFINALWARNINGMSG,
    DISPLAYFINALWARNINGMSG2,
    DISPLAYTAXWARNINGMSG,
    DISPLAYTAXWARNINGMSG2,
    DISPLAYPRIVATEFAREIND,
    DISPLAYNONCOCCURRENCYIND,
    DISPLAYTRUEPTCINFARELINE,
    APPLYROINFAREDISPLAY,
    ALWAYSMAPTOADTPSGRTYPE,
    NOMATCHRBDMESSAGE,
    NOMATCHNOFARESERRORMSG,
    TOTALNOOPTIONS,
    SEQNO,
    NODISPLAYOPTIONS,
    FARETYPEGROUP,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select c.USERAPPLTYPE, c.USERAPPL, c.LOC1TYPE, c.LOC1,"
        "       c.WQNOTPERMITTED, c.MAXNOOPTIONS, c.WQSORT,"
        "       c.WQDUPLICATEAMOUNTS, c.FARELINEHEADERFORMAT, c.PASSENGERDETAILLINEFORMAT,"
        "       c.FARELINEPTC, c.PRIMEPTCREFNO, c.SECONDARYPTCREFNO,"
        "       c.FARELINEPTCBREAK, c.PASSENGERDETAILPTCBREAK, c.NEGPASSENGERTYPEMAPPING,"
        "       c.NOMATCHOPTIONSDISPLAY, c.ALLMATCHTRAILERMESSAGE, c.MATCHINTEGRATEDTRAILER,"
        "       c.ACCOMPANIEDTVLTRAILERMSG, c.RBDMATCHTRAILERMSG, c.RBDNOMATCHTRAILERMSG,"
        "       c.RBDNOMATCHTRAILERMSG2, c.DISPLAYFARERULEWARNINGMSG, c.DISPLAYFARERULEWARNINGMSG2,"
        "       c.DISPLAYFINALWARNINGMSG, c.DISPLAYFINALWARNINGMSG2,"
        "       c.DISPLAYTAXWARNINGMSG, c.DISPLAYTAXWARNINGMSG2, c.DISPLAYPRIVATEFAREIND,"
        "       c.DISPLAYNONCOCCURRENCYIND, c.DISPLAYTRUEPTCINFARELINE,"
        "       c.APPLYROINFAREDISPLAY, c.ALWAYSMAPTOADTPSGRTYPE, c.NOMATCHRBDMESSAGE,"
        "       c.NOMATCHNOFARESERRORMSG, c.TOTALNOOPTIONS,"
        "       cs.SEQNO, cs.NODISPLAYOPTIONS, cs.FARETYPEGROUP");

    //		        this->From("=NOPNROPTIONS c LEFT OUTER JOIN =NOPNROPTIONSSEG cs "
    //		                   "                        USING
    //(USERAPPLTYPE,USERAPPL,LOC1TYPE,LOC1)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("USERAPPLTYPE");
    joinFields.push_back("USERAPPL");
    joinFields.push_back("LOC1TYPE");
    joinFields.push_back("LOC1");
    this->generateJoinString(
        "=NOPNROPTIONS", "c", "LEFT OUTER JOIN", "=NOPNROPTIONSSEG", "cs", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("c.USERAPPLTYPE = %1q "
                "   and c.USERAPPL = %2q ");
    this->OrderBy("c.USERAPPLTYPE desc, c.USERAPPL desc,"
                  "          c.LOC1TYPE desc, c.LOC1 desc, cs.SEQNO desc");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NoPNROptions* mapRowToNoPNROptions(Row* row, NoPNROptions* npoPrev)
  {
    Indicator userApplType = row->getChar(USERAPPLTYPE);
    UserApplCode userAppl = row->getString(USERAPPL);
    LocTypeCode locType1 = row->getChar(LOC1TYPE);
    LocCode loc1 = row->getString(LOC1);

    NoPNROptions* npo;

    // If Parent hasn't changed, add to Child (segs)
    if (npoPrev != nullptr && npoPrev->userApplType() == userApplType &&
        npoPrev->userAppl() == userAppl && npoPrev->loc1().locType() == locType1 &&
        npoPrev->loc1().loc() == loc1)
    {
      npo = npoPrev;
    }
    else
    { // Time for a new Parent
      npo = new tse::NoPNROptions;
      npo->userApplType() = userApplType;
      npo->userAppl() = userAppl;
      npo->loc1().locType() = locType1;
      npo->loc1().loc() = loc1;
      npo->wqNotPermitted() = row->getChar(WQNOTPERMITTED);
      npo->maxNoOptions() = row->getInt(MAXNOOPTIONS);
      npo->wqSort() = row->getChar(WQSORT);
      npo->wqDuplicateAmounts() = row->getChar(WQDUPLICATEAMOUNTS);
      npo->fareLineHeaderFormat() = row->getChar(FARELINEHEADERFORMAT);
      npo->passengerDetailLineFormat() = row->getChar(PASSENGERDETAILLINEFORMAT);
      npo->fareLinePTC() = row->getChar(FARELINEPTC);
      npo->primePTCRefNo() = row->getChar(PRIMEPTCREFNO);
      npo->secondaryPTCRefNo() = row->getChar(SECONDARYPTCREFNO);
      npo->fareLinePTCBreak() = row->getChar(FARELINEPTCBREAK);
      npo->passengerDetailPTCBreak() = row->getChar(PASSENGERDETAILPTCBREAK);
      npo->negPassengerTypeMapping() = row->getChar(NEGPASSENGERTYPEMAPPING);
      npo->noMatchOptionsDisplay() = row->getChar(NOMATCHOPTIONSDISPLAY);
      npo->allMatchTrailerMessage() = row->getChar(ALLMATCHTRAILERMESSAGE);
      npo->matchIntegratedTrailer() = row->getChar(MATCHINTEGRATEDTRAILER);
      npo->accompaniedTvlTrailerMsg() = row->getChar(ACCOMPANIEDTVLTRAILERMSG);
      npo->rbdMatchTrailerMsg() = row->getChar(RBDMATCHTRAILERMSG);
      npo->rbdNoMatchTrailerMsg() = row->getChar(RBDNOMATCHTRAILERMSG);
      npo->rbdNoMatchTrailerMsg2() = row->getChar(RBDNOMATCHTRAILERMSG2);
      npo->displayFareRuleWarningMsg() = row->getChar(DISPLAYFARERULEWARNINGMSG);
      npo->displayFareRuleWarningMsg2() = row->getChar(DISPLAYFARERULEWARNINGMSG2);
      npo->displayFinalWarningMsg() = row->getChar(DISPLAYFINALWARNINGMSG);
      npo->displayFinalWarningMsg2() = row->getChar(DISPLAYFINALWARNINGMSG2);
      npo->displayTaxWarningMsg() = row->getChar(DISPLAYTAXWARNINGMSG);
      npo->displayTaxWarningMsg2() = row->getChar(DISPLAYTAXWARNINGMSG2);
      npo->displayPrivateFareInd() = row->getChar(DISPLAYPRIVATEFAREIND);
      npo->displayNonCOCCurrencyInd() = row->getChar(DISPLAYNONCOCCURRENCYIND);
      npo->displayTruePTCInFareLine() = row->getChar(DISPLAYTRUEPTCINFARELINE);
      npo->applyROInFareDisplay() = row->getChar(APPLYROINFAREDISPLAY);
      npo->alwaysMapToADTPsgrType() = row->getChar(ALWAYSMAPTOADTPSGRTYPE);
      npo->noMatchRBDMessage() = row->getChar(NOMATCHRBDMESSAGE);
      npo->noMatchNoFaresErrorMsg() = row->getChar(NOMATCHNOFARESERRORMSG);
      if (!row->isNull(TOTALNOOPTIONS))
      {
        npo->totalNoOptions() = row->getInt(TOTALNOOPTIONS);
      }
      else
      {
        npo->totalNoOptions() = 0;
      }
    }

    // Load up new Seg
    if (!row->isNull(SEQNO))
    {
      NoPNROptionsSeg* newSeg = new NoPNROptionsSeg;

      newSeg->seqNo() = row->getLong(SEQNO);
      newSeg->noDisplayOptions() = row->getInt(NODISPLAYOPTIONS);
      newSeg->fareTypeGroup() = row->getInt(FARETYPEGROUP);

      npo->segs().push_back(newSeg);
    }

    return npo;
  } // mapRowToNoPNROptions()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllNoPNROptionss
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllNoPNROptionsSQLStatement : public QueryGetNoPNROptionsSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("1=1"); }
};
} // tse
