//----------------------------------------------------------------------------
//          File:           QueryGetHeaderMsgSQLStatement.h
//          Description:    QueryGetHeaderMsgSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHeaderMsg.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetHeaderMsgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetHeaderMsgSQLStatement() {};
  virtual ~QueryGetHeaderMsgSQLStatement() {};

  enum ColumnIndexes
  {
    USERAPPLTYPE = 0,
    USERAPPL,
    PSEUDOCITYTYPE,
    PSEUDOCITY,
    SSGGROUPNO,
    VERSIONDATE,
    HDRMSGSEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    FAREDISPLAYTYPE,
    CARRIER,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    GLOBALDIR,
    ROUTING1,
    ROUTING2,
    INCLUSIONCODE,
    MESSAGETYPE,
    MSGITEMNO,
    LOCKDATE,
    MEMONO,
    CREATORBUSINESSUNIT,
    CREATORID,
    STARTPOINT,
    EXCEPTPOSLOC,
    POSLOCTYPE,
    POSLOC,
    NEWSEQNO,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    ROUTINGTARIFF,
    EXCEPTCUR,
    INSIDEBUFFERZONE,
    OUTSIDEBUFFERZONE,
    EXCEPTLOC,
    TEXT,
    CUR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select a.USERAPPLTYPE,a.USERAPPL,a.PSEUDOCITYTYPE,a.PSEUDOCITY,a.SSGGROUPNO,a.VERSIONDATE,"
        " a.SEQNO HDRMSGSEQNO,a.CREATEDATE,a.EXPIREDATE,a.EFFDATE, a.DISCDATE,"
        " a.FAREDISPLAYTYPE,a.CARRIER,a.DIRECTIONALITY,a.LOC1TYPE,a.LOC1,"
        " a.LOC2TYPE,a.LOC2,a.GLOBALDIR,a.ROUTING1,a.ROUTING2,a.INCLUSIONCODE,"
        " a.MESSAGETYPE,a.MSGITEMNO,a.LOCKDATE,a.MEMONO,a.CREATORBUSINESSUNIT,"
        " a.CREATORID,a.STARTPOINT,a.EXCEPTPOSLOC,a.POSLOCTYPE,a.POSLOC,a.NEWSEQNO,"
        " a.VERSIONINHERITEDIND,a.VERSIONDISPLAYIND,a.ROUTINGTARIFF,a.EXCEPTCUR,"
        " a.INSIDEBUFFERZONE,a.OUTSIDEBUFFERZONE,a.EXCEPTLOC,"
        " b.TEXT,c.CUR ");

    //		        this->From("=FAREDISPLAYHDRMSG a"
    //		                   " left outer join =FAREDISPLAYHDRMSGSEG c"
    //		                   " using
    //(USERAPPLTYPE,USERAPPL,PSEUDOCITYTYPE,PSEUDOCITY,SSGGROUPNO,VERSIONDATE,SEQNO,CREATEDATE)"
    //		                   " join =FREETEXTSEG b "
    //		                   " on b.MESSAGETYPE = a.MESSAGETYPE "
    //		                   " and b.ITEMNO      = a.MSGITEMNO "
    //		                   " and b.SEQNO = 0 ");

    this->From("=FAREDISPLAYHDRMSG a"
               " left outer join =FAREDISPLAYHDRMSGSEG c"
               "    on a.USERAPPLTYPE = c.USERAPPLTYPE "
               "    and a.USERAPPL = c.USERAPPL "
               "    and a.PSEUDOCITYTYPE = c.PSEUDOCITYTYPE "
               "    and a.PSEUDOCITY = c.PSEUDOCITY "
               "    and a.SSGGROUPNO = c.SSGGROUPNO "
               "    and a.VERSIONDATE = c.VERSIONDATE "
               "    and a.SEQNO = c.SEQNO "
               "    and a.CREATEDATE = c.CREATEDATE "
               " join =FREETEXTSEG b "
               "    on b.MESSAGETYPE = a.MESSAGETYPE "
               "    and b.ITEMNO = a.MSGITEMNO "
               "    and b.SEQNO = 0 ");

    this->Where("a.PSEUDOCITY = %1q"
                " and a.PSEUDOCITYTYPE = %2q"
                " and a.USERAPPLTYPE = %3q"
                " and a.USERAPPL = %4q"
                " and %cd <= a.EXPIREDATE"
                " and a.SSGGROUPNO = %5n ");

    if (DataManager::forceSortOrder())
      this->OrderBy(" a.USERAPPLTYPE ,a.USERAPPL, a.PSEUDOCITYTYPE,  a.PSEUDOCITY, a.SSGGROUPNO, "
                    "a.VERSIONDATE, a.SEQNO, a.CREATEDATE, c.CUR, b.MESSAGETYPE, b.ITEMNO, "
                    "b.SEQNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::FDHeaderMsg* mapRowToFDHeaderMsg(Row* row)
  {
    tse::FDHeaderMsg* sf = new tse::FDHeaderMsg;

    sf->userApplType() = row->getChar(USERAPPLTYPE);
    sf->userAppl() = row->getString(USERAPPL);
    sf->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    sf->pseudoCityCode() = row->getString(PSEUDOCITY);
    sf->ssgGroupNo() = row->getInt(SSGGROUPNO);
    sf->versionDate() = row->getDate(VERSIONDATE);

    sf->seqNo() = row->getLong(HDRMSGSEQNO);

    sf->createDate() = row->getDate(CREATEDATE);
    sf->expireDate() = row->getDate(EXPIREDATE);
    sf->effDate() = row->getDate(EFFDATE);
    sf->discDate() = row->getDate(DISCDATE);

    sf->fareDisplayType() = row->getChar(FAREDISPLAYTYPE);
    sf->carrier() = row->getString(CARRIER);

    std::string dir = row->getString(DIRECTIONALITY);
    if (dir == "F")
      sf->directionality() = FROM;
    else if (dir == "W")
      sf->directionality() = WITHIN;
    else if (dir == "O")
      sf->directionality() = ORIGIN;
    else if (dir == "X")
      sf->directionality() = TERMINATE;
    else if (dir.empty() || dir == " " || dir == "B")
      sf->directionality() = BETWEEN;

    sf->loc1().locType() = row->getChar(LOC1TYPE);
    sf->loc1().loc() = row->getString(LOC1);
    sf->loc2().locType() = row->getChar(LOC2TYPE);
    sf->loc2().loc() = row->getString(LOC2);

    strToGlobalDirection(sf->globalDir(), row->getString(GLOBALDIR));

    sf->routing1() = row->getString(ROUTING1);
    sf->routing2() = row->getString(ROUTING2);
    sf->inclusionCode() = row->getString(INCLUSIONCODE);
    sf->messageType() = row->getString(MESSAGETYPE);
    sf->msgItemNo() = row->getLong(MSGITEMNO);
    sf->lockDate() = row->getDate(LOCKDATE);
    sf->memoNo() = row->getLong(MEMONO);
    sf->creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);
    sf->creatorId() = row->getString(CREATORID);
    sf->startPoint() = row->getString(STARTPOINT);
    sf->exceptPosLoc() = row->getChar(EXCEPTPOSLOC);
    sf->posLocType() = row->getChar(POSLOCTYPE);
    sf->posLoc() = row->getString(POSLOC);

    sf->newSeqNo() = row->getLong(NEWSEQNO);
    sf->versionInheritedInd() = row->getChar(VERSIONINHERITEDIND);
    sf->versionDisplayInd() = row->getChar(VERSIONDISPLAYIND);
    sf->routingTariff() = row->getInt(ROUTINGTARIFF);
    sf->exceptCur() = row->getChar(EXCEPTCUR);
    sf->insideBufferZone() = row->getChar(INSIDEBUFFERZONE);
    sf->outsideBufferZone() = row->getChar(OUTSIDEBUFFERZONE);
    sf->exceptLoc() = row->getChar(EXCEPTLOC);
    sf->text() = row->getString(TEXT);

    sf->textSeqNo() = 0;

    if (!row->isNull(CUR))
      sf->cur() = row->getString(CUR);

    return sf;
  } // mapRowToFDHeaderMsg()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFlightAppRuleSQLStatement
}

