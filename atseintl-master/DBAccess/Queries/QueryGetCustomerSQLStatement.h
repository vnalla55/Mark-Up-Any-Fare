//----------------------------------------------------------------------------
//          File:           QueryGetCustomerSQLStatement.h
//          Description:    QueryGetCustomerSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetCustomer.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCustomerSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCustomerSQLStatement() {};
  virtual ~QueryGetCustomerSQLStatement() {};

  enum ColumnIndexes
  {
    PSEUDOCITY = 0,
    HOMEPSEUDOCITY,
    ARCNO,
    HOMEARCNO,
    REQUESTCITY,
    AACITY,
    DEFAULTCUR,
    LBTCUSTOMERGROUP,
    NEWLBTCUSTOMERGROUP,
    AGENCYNAME,
    ALTERNATEHOMEPSEUDO,
    BRANCHACCIND,
    CURCONVIND,
    CADSUBSCRIBERIND,
    WEBSUBSCRIBERIND,
    BTSSUBSCRIBERIND,
    SELLINGFAREIND,
    TVLYINTERNETSUBRIBER,
    TVLYLOCATION,
    TVLYONLINELOCATION,
    AVAILABILITYIGRUL2ST,
    AVAILABILITYIGRUL3ST,
    ERSPNUMBER,
    CHANNELID,
    AVAILIGRUL2STWPNC,
    ACTIVATEJOURNEYPRICING,
    ACTIVATEJOURNEYSHOPPING,
    OWNERID,
    CRUISEPFACUR,
    NOROLLUPPFABULKTKT,
    CRSCARRIER,
    DONOTAPPLYSEGMENTFEE,
    SSGGROUPNO,
    HOSTNAME,
    OPTINAGENCY,
    PRIVATEFAREIND,
    SNAPHOMEAGENCYPCC,
    DONOTAPPLYOBTKTFEES,
    CREATEDATE,
    DEFAULTPASSENGERTYPE,
    ETICKETCAPABLE,
    FAREQUOTECUR,
    PRICINGAPPLTAG1,
    PRICINGAPPLTAG2,
    PRICINGAPPLTAG3,
    PRICINGAPPLTAG4,
    PRICINGAPPLTAG5,
    PRICINGAPPLTAG6,
    PRICINGAPPLTAG7,
    PRICINGAPPLTAG8,
    PRICINGAPPLTAG9,
    PRICINGAPPLTAG10,
    CAT05OVERRIDECODE,
    SETTLEMENTPLANS,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select "
        "PSEUDOCITY,HOMEPSEUDOCITY,ARCNO,HOMEARCNO,REQUESTCITY,AACITY,"
        "DEFAULTCUR,LBTCUSTOMERGROUP,NEWLBTCUSTOMERGROUP,AGENCYNAME,"
        "ALTERNATEHOMEPSEUDO,BRANCHACCIND,CURCONVIND,CADSUBSCRIBERIND,"
        "WEBSUBSCRIBERIND,BTSSUBSCRIBERIND,SELLINGFAREIND,TVLYINTERNETSUBRIBER,"
        "TVLYLOCATION,TVLYONLINELOCATION,AVAILABILITYIGRUL2ST,"
        "AVAILABILITYIGRUL3ST,ERSPNUMBER,CHANNELID,AVAILIGRUL2STWPNC,"
        "ACTIVATEJOURNEYPRICING,ACTIVATEJOURNEYSHOPPING,OWNERID,CRUISEPFACUR,"
        "NOROLLUPPFABULKTKT,CRSCARRIER,DONOTAPPLYSEGMENTFEE,SSGGROUPNO,HOSTNAME,OPTINAGENCY,"
        "PRIVATEFAREIND,SNAPHOMEAGENCYPCC,DONOTAPPLYOBTKTFEES,CREATEDATE,DEFAULTPASSENGERTYPE,"
        "ETICKETCAPABLE,FAREQUOTECUR,"
        "PRICINGAPPLTAG1,PRICINGAPPLTAG2,PRICINGAPPLTAG3,PRICINGAPPLTAG4,PRICINGAPPLTAG5,"
        "PRICINGAPPLTAG6,PRICINGAPPLTAG7,PRICINGAPPLTAG8,PRICINGAPPLTAG9,PRICINGAPPLTAG10, "
        "CAT05OVERRIDECODE,SETTLEMENTPLANS" );

    this->From("=CUSTOMER");
    this->Where("PSEUDOCITY = %1q");

    if (DataManager::forceSortOrder())
      this->OrderBy("PSEUDOCITY");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Customer* mapRowToCustomer(Row* row)
  {
    tse::Customer* c = new tse::Customer;

    c->pseudoCity() = row->getString(PSEUDOCITY);
    c->homePseudoCity() = row->getString(HOMEPSEUDOCITY);
    c->arcNo() = row->getString(ARCNO);
    c->homeArcNo() = row->getString(HOMEARCNO);
    c->requestCity() = row->getString(REQUESTCITY);
    c->aaCity() = row->getString(AACITY);
    c->defaultCur() = row->getString(DEFAULTCUR);
    c->lbtCustomerGroup() = row->getString(LBTCUSTOMERGROUP);
    c->newLbtCustomerGroup() = row->getString(NEWLBTCUSTOMERGROUP);
    c->agencyName() = row->getString(AGENCYNAME);
    c->alternateHomePseudo() = row->getString(ALTERNATEHOMEPSEUDO);
    c->branchAccInd() = row->getChar(BRANCHACCIND);
    c->curConvInd() = row->getChar(CURCONVIND);
    c->cadSubscriberInd() = row->getChar(CADSUBSCRIBERIND);
    c->webSubscriberInd() = row->getChar(WEBSUBSCRIBERIND);
    c->btsSubscriberInd() = row->getChar(BTSSUBSCRIBERIND);
    c->sellingFareInd() = row->getChar(SELLINGFAREIND);
    c->tvlyInternetSubriber() = row->getChar(TVLYINTERNETSUBRIBER);
    c->tvlyLocation() = row->getChar(TVLYLOCATION);
    c->tvlyOnlineLocation() = row->getChar(TVLYONLINELOCATION);
    c->availabilityIgRul2St() = row->getChar(AVAILABILITYIGRUL2ST);
    c->availabilityIgRul3St() = row->getChar(AVAILABILITYIGRUL3ST);
    c->erspNumber() = row->getString(ERSPNUMBER);
    c->channelId() = row->getChar(CHANNELID);
    c->availIgRul2StWpnc() = row->getChar(AVAILIGRUL2STWPNC);
    c->activateJourneyPricing() = row->getChar(ACTIVATEJOURNEYPRICING);
    c->activateJourneyShopping() = row->getChar(ACTIVATEJOURNEYSHOPPING);
    c->ownerId() = row->getString(OWNERID);
    c->cruisePfaCur() = row->getString(CRUISEPFACUR);
    c->noRollupPfaBulkTkt() = row->getChar(NOROLLUPPFABULKTKT);
    c->crsCarrier() = row->getString(CRSCARRIER);
    c->doNotApplySegmentFee() = row->getChar(DONOTAPPLYSEGMENTFEE);
    c->ssgGroupNo() = row->getInt(SSGGROUPNO);
    c->hostName() = row->getString(HOSTNAME);
    c->optInAgency() = row->getChar(OPTINAGENCY);
    c->privateFareInd() = row->getChar(PRIVATEFAREIND);
    c->snapHomeAgencyPcc() = row->getString(SNAPHOMEAGENCYPCC);
    c->doNotApplyObTktFees() = row->getChar(DONOTAPPLYOBTKTFEES);
    c->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(DEFAULTPASSENGERTYPE))
      c->defaultPassengerType() = row->getString(DEFAULTPASSENGERTYPE);
    c->eTicketCapable() = row->getChar(ETICKETCAPABLE);
    if (!row->isNull(FAREQUOTECUR))
      c->fareQuoteCur() = row->getChar(FAREQUOTECUR);

    c->pricingApplTag1() = row->getChar(PRICINGAPPLTAG1);
    c->pricingApplTag2() = row->getChar(PRICINGAPPLTAG2);
    c->pricingApplTag3() = row->getChar(PRICINGAPPLTAG3);
    c->pricingApplTag4() = row->getChar(PRICINGAPPLTAG4);
    c->pricingApplTag5() = row->getChar(PRICINGAPPLTAG5);
    c->pricingApplTag6() = row->getChar(PRICINGAPPLTAG6);
    c->pricingApplTag7() = row->getChar(PRICINGAPPLTAG7);
    c->pricingApplTag8() = row->getChar(PRICINGAPPLTAG8);
    c->pricingApplTag9() = row->getChar(PRICINGAPPLTAG9);
    c->pricingApplTag10() = row->getChar(PRICINGAPPLTAG10);
    if (!row->isNull(CAT05OVERRIDECODE))
      c->cat05OverrideCode() = row->getString(CAT05OVERRIDECODE);
    c->settlementPlans() = row->getString(SETTLEMENTPLANS);

    return c;
  } // mapRowToCustomer()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
