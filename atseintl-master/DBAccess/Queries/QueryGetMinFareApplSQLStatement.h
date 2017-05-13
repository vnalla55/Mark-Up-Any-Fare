//----------------------------------------------------------------------------
//          File:           QueryGetMinFareApplSQLStatement.h
//          Description:    QueryGetMinFareApplSQLStatement
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
#include "DBAccess/Queries/QueryGetMinFareAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMinFareApplBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareApplBaseSQLStatement() {};
  virtual ~QueryGetMinFareApplBaseSQLStatement() {};

  enum ColumnIndexes
  {
    TEXTTBLVENDOR = 0,
    TEXTTBLITEMNO,
    GOVERNINGCARRIER,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    VENDOR,
    ROUTINGTARIFF1,
    ROUTINGTARIFF2,
    RULETARIFF,
    TARIFFCAT,
    NMLHIPTARIFFCATIND,
    NMLCTMTARIFFCATIND,
    TKTGCARRIER,
    USERAPPLTYPE,
    USERAPPL,
    RULETARIFFCODE,
    FARETYPEAPPL,
    CABIN,
    MPMIND,
    ROUTINGIND,
    ROUTING,
    GLOBALDIR,
    DIRECTIONALIND,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    VIAEXCEPTIND,
    VIADIRECTIONALIND,
    VIALOC1TYPE,
    VIALOC1,
    VIALOC2TYPE,
    VIALOC2,
    INTERMDIRECTIONALIND,
    INTERMEDIATELOC1TYPE,
    INTERMEDIATELOC1,
    INTERMEDIATELOC2TYPE,
    INTERMEDIATELOC2,
    STOPCONNECTRESTR,
    HIPCHECKAPPL,
    HIPSTOPTKTIND,
    CTMCHECKAPPL,
    CTMSTOPTKTIND,
    BACKHAULCHKAPPL,
    BACKHAULSTOPTKTIND,
    DMCCHECKAPPL,
    DMCSTOPTKTIND,
    COMCHECKAPPL,
    COMSTOPTKTIND,
    CPMCHECKAPPL,
    CPMSTOPTKTIND,
    BETWDIRECTIONALIND,
    BETWLOC1TYPE,
    BETWLOC1,
    BETWLOC2TYPE,
    BETWLOC2,
    SERVICERESTR,
    NONSTOPIND,
    DIRECTIND,
    CONSTRUCTPOINTIND,
    EXCEPTCXRFLTRESTR,
    EXCEPTSECONDARYCXR,
    POSEXCEPTIND,
    POSLOCTYPE,
    POSLOC,
    POIEXCEPTIND,
    POILOCTYPE,
    POILOC,
    SITIIND,
    SITOIND,
    SOTIIND,
    SOTOIND,
    APPLYDEFAULTLOGIC,
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
    INTHIPCHECKAPPL,
    INTHIPSTOPTKTIND,
    INTCTMCHECKAPPL,
    INTCTMSTOPTKTIND,
    INTBACKHAULCHKAPPL,
    INTBACKHAULSTOPTKTIND,
    INTDMCCHECKAPPL,
    INTDMCSTOPTKTIND,
    INTCOMCHECKAPPL,
    INTCOMSTOPTKTIND,
    INTCPMCHECKAPPL,
    INTCPMSTOPTKTIND,
    DOMFARETYPEEXCEPT,
    NMLHIPORIGNATIONTKTPT,
    NMLHIPORIGNATIONSTOPPT,
    NMLHIPSTOPOVERPT,
    NMLHIPTICKETEDPT,
    SPCLHIPORIGNATIONTKTPT,
    SPCLHIPORIGNATIONSTOPPT,
    NMLHIPNOINTERTOINTER,
    SPCLHIPNOINTERTOINTER,
    SPCLHIPSTOPOVERPT,
    SPCLHIPTICKETEDPT,
    CARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select m.TEXTTBLVENDOR,m.TEXTTBLITEMNO,m.GOVERNINGCARRIER,m.VERSIONDATE,m.SEQNO,"
                  " m.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,VENDOR,ROUTINGTARIFF1,ROUTINGTARIFF2,"
                  " RULETARIFF,TARIFFCAT,NMLHIPTARIFFCATIND,NMLCTMTARIFFCATIND,TKTGCARRIER,"
                  " USERAPPLTYPE,USERAPPL,RULETARIFFCODE,FARETYPEAPPL,CABIN,MPMIND,ROUTINGIND,"
                  " ROUTING,GLOBALDIR,DIRECTIONALIND,LOC1TYPE,LOC1,LOC2TYPE,LOC2,VIAEXCEPTIND,"
                  " VIADIRECTIONALIND,VIALOC1TYPE,VIALOC1,VIALOC2TYPE,VIALOC2,INTERMDIRECTIONALIND,"
                  " INTERMEDIATELOC1TYPE,INTERMEDIATELOC1,INTERMEDIATELOC2TYPE,INTERMEDIATELOC2,"
                  " STOPCONNECTRESTR,HIPCHECKAPPL,HIPSTOPTKTIND,CTMCHECKAPPL,CTMSTOPTKTIND,"
                  " BACKHAULCHKAPPL,BACKHAULSTOPTKTIND,DMCCHECKAPPL,DMCSTOPTKTIND,COMCHECKAPPL,"
                  " COMSTOPTKTIND,CPMCHECKAPPL,CPMSTOPTKTIND,BETWDIRECTIONALIND,BETWLOC1TYPE,"
                  " BETWLOC1,BETWLOC2TYPE,BETWLOC2,SERVICERESTR,NONSTOPIND,DIRECTIND,"
                  " CONSTRUCTPOINTIND,EXCEPTCXRFLTRESTR,EXCEPTSECONDARYCXR,POSEXCEPTIND,POSLOCTYPE,"
                  " POSLOC,POIEXCEPTIND,POILOCTYPE,POILOC,SITIIND,SITOIND,SOTIIND,SOTOIND,"
                  " APPLYDEFAULTLOGIC,DOMAPPL,DOMEXCEPTIND,DOMLOCTYPE,DOMLOC,NMLFARECOMPAREIND,"
                  " NMLMPMBEFORERTGIND,NMLRTGBEFOREMPMIND,NMLHIPRESTRCOMPIND,NMLHIPUNRESTRCOMPIND,"
                  " NMLHIPRBDCOMPIND,NMLHIPSTOPCOMPIND,NMLHIPORIGIND,NMLHIPORIGNATIONIND,"
                  " NMLHIPFROMINTERIND,NMLHIPDESTIND,NMLHIPDESTNATIONIND,NMLHIPTOINTERIND,"
                  " SPCLHIPTARIFFCATIND,SPCLHIPRULETRFIND,SPCLHIPFARECLASSIND,SPCLHIP1STCHARIND,"
                  " SPCLHIPSTOPCOMPIND,SPCLHIPSPCLONLYIND,SPCLHIPLOCTYPE,SPCLHIPLOC,SPCLHIPORIGIND,"
                  " SPCLHIPORIGNATIONIND,SPCLHIPFROMINTERIND,SPCLHIPDESTIND,SPCLHIPDESTNATIONIND,"
                  " SPECIALPROCESSNAME,SPCLHIPTOINTERIND,NMLCTMRESTRCOMPIND,NMLCTMUNRESTRCOMPIND,"
                  " NMLCTMRBDCOMPIND,NMLCTMSTOPCOMPIND,NMLCTMORIGIND,NMLCTMDESTNATIONIND,"
                  " NMLCTMTOINTERIND,SPCLCTMTARIFFCATIND,SPCLCTMRULETRFIND,SPCLCTMFARECLASSIND,"
                  " SPCLSAME1STCHARFBIND2,SPCLCTMSTOPCOMPIND,SPCLCTMMKTCOMP,SPCLCTMORIGIND,"
                  " SPCLCTMDESTNATIONIND,SPCLCTMTOINTERIND,CPMEXCL,INTHIPCHECKAPPL,"
                  " INTHIPSTOPTKTIND,INTCTMCHECKAPPL,INTCTMSTOPTKTIND,INTBACKHAULCHKAPPL,"
                  " INTBACKHAULSTOPTKTIND,INTDMCCHECKAPPL,INTDMCSTOPTKTIND,INTCOMCHECKAPPL,"
                  " INTCOMSTOPTKTIND,INTCPMCHECKAPPL,INTCPMSTOPTKTIND,DOMFARETYPEEXCEPT,"
                  " NMLHIPORIGNATIONTKTPT,NMLHIPORIGNATIONSTOPPT,NMLHIPSTOPOVERPT,NMLHIPTICKETEDPT,"
                  " SPCLHIPORIGNATIONTKTPT,SPCLHIPORIGNATIONSTOPPT,NMLHIPNOINTERTOINTER,"
                  " SPCLHIPNOINTERTOINTER,SPCLHIPSTOPOVERPT,SPCLHIPTICKETEDPT,ec.CARRIER");

    //		        this->From("=MINIMUMFARE m LEFT OUTER JOIN =MINFAREEXCEPTCXR ec"
    //		                      "  USING (TEXTTBLVENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,"
    //		                      "         VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(6);
    joinFields.push_back("TEXTTBLVENDOR");
    joinFields.push_back("TEXTTBLITEMNO");
    joinFields.push_back("GOVERNINGCARRIER");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=MINIMUMFARE", "m", "LEFT OUTER JOIN", "=MINFAREEXCEPTCXR", "ec", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("m.TEXTTBLVENDOR in ('', %1q)"
                " and m.TEXTTBLITEMNO in (0, %2n)"
                " and m.GOVERNINGCARRIER in ('', %3q)"
                " and %cd <= EXPIREDATE");
    this->OrderBy("TEXTTBLVENDOR desc,TEXTTBLITEMNO desc,GOVERNINGCARRIER desc,SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MinFareAppl* mapRowToMinFareApplBase(Row* row, MinFareAppl* mfaPrev)
  {
    VendorCode textTblVndr = row->getString(TEXTTBLVENDOR);
    int textTblItemNo = row->getInt(TEXTTBLITEMNO);
    CarrierCode govCxr = row->getString(GOVERNINGCARRIER);
    DateTime versionDate = row->getDate(VERSIONDATE);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::MinFareAppl* mfa;

    // If Parent hasn't changed, just add Children to Prev
    if (mfaPrev != nullptr && mfaPrev->textTblVendor() == textTblVndr &&
        mfaPrev->textTblItemNo() == textTblItemNo && mfaPrev->governingCarrier() == govCxr &&
        mfaPrev->versionDate() == versionDate && mfaPrev->seqNo() == seqNo &&
        mfaPrev->createDate() == createDate)
    { // Just add to Prev
      mfa = mfaPrev;
    }
    else
    { // Build a new parent
      mfa = new tse::MinFareAppl;
      mfa->textTblVendor() = textTblVndr;
      mfa->textTblItemNo() = textTblItemNo;
      mfa->governingCarrier() = govCxr;
      mfa->versionDate() = versionDate;
      mfa->seqNo() = seqNo;
      mfa->createDate() = createDate;
      mfa->expireDate() = row->getDate(EXPIREDATE);
      mfa->effDate() = row->getDate(EFFDATE);
      mfa->discDate() = row->getDate(DISCDATE);
      mfa->vendor() = row->getString(VENDOR);
      mfa->routingTariff1() = row->getInt(ROUTINGTARIFF1);
      mfa->routingTariff2() = row->getInt(ROUTINGTARIFF2);
      mfa->ruleTariff() = row->getInt(RULETARIFF);
      mfa->tariffCat() = row->getInt(TARIFFCAT);
      mfa->nmlHipTariffCatInd() = row->getInt(NMLHIPTARIFFCATIND);
      mfa->nmlCtmTariffCatInd() = row->getInt(NMLCTMTARIFFCATIND);
      mfa->tktgCarrier() = row->getString(TKTGCARRIER);
      mfa->userApplType() = row->getChar(USERAPPLTYPE);
      mfa->userAppl() = row->getString(USERAPPL);
      mfa->ruleTariffCode() = row->getString(RULETARIFFCODE);
      mfa->fareTypeAppl() = row->getChar(FARETYPEAPPL);
      // Indicator cab = row->getChar(CABIN);
      // mfa->cabin().setClass(cab);
      mfa->mpmInd() = row->getChar(MPMIND);
      mfa->routingInd() = row->getChar(ROUTINGIND);
      mfa->routing() = row->getString(ROUTING);

      std::string gd = row->getString(GLOBALDIR);
      strToGlobalDirection(mfa->globalDir(), gd);

      std::string dir = row->getString(DIRECTIONALIND);
      if (dir == "F")
        mfa->directionalInd() = FROM;
      else if (dir == "W")
        mfa->directionalInd() = WITHIN;
      else if (dir == "O")
        mfa->directionalInd() = ORIGIN;
      else if (dir == "X")
        mfa->directionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        mfa->directionalInd() = BETWEEN;

      LocKey* loc = &mfa->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &mfa->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      mfa->viaExceptInd() = row->getChar(VIAEXCEPTIND);

      dir = row->getString(VIADIRECTIONALIND);
      if (dir == "F")
        mfa->viaDirectionalInd() = FROM;
      else if (dir == "W")
        mfa->viaDirectionalInd() = WITHIN;
      else if (dir == "O")
        mfa->viaDirectionalInd() = ORIGIN;
      else if (dir == "X")
        mfa->viaDirectionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        mfa->viaDirectionalInd() = BETWEEN;

      loc = &mfa->viaLoc1();
      loc->locType() = row->getChar(VIALOC1TYPE);
      loc->loc() = row->getString(VIALOC1);

      loc = &mfa->viaLoc2();
      loc->locType() = row->getChar(VIALOC2TYPE);
      loc->loc() = row->getString(VIALOC2);

      dir = row->getString(INTERMDIRECTIONALIND);
      if (dir == "F")
        mfa->intermDirectionalInd() = FROM;
      else if (dir == "W")
        mfa->intermDirectionalInd() = WITHIN;
      else if (dir == "O")
        mfa->intermDirectionalInd() = ORIGIN;
      else if (dir == "X")
        mfa->intermDirectionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        mfa->intermDirectionalInd() = BETWEEN;

      loc = &mfa->intermediateLoc1();
      loc->locType() = row->getChar(INTERMEDIATELOC1TYPE);
      loc->loc() = row->getString(INTERMEDIATELOC1);

      loc = &mfa->intermediateLoc2();
      loc->locType() = row->getChar(INTERMEDIATELOC2TYPE);
      loc->loc() = row->getString(INTERMEDIATELOC2);

      mfa->stopConnectRestr() = row->getChar(STOPCONNECTRESTR);
      mfa->hipCheckAppl() = row->getChar(HIPCHECKAPPL);
      mfa->hipStopTktInd() = row->getChar(HIPSTOPTKTIND);
      mfa->ctmCheckAppl() = row->getChar(CTMCHECKAPPL);
      mfa->ctmStopTktInd() = row->getChar(CTMSTOPTKTIND);
      mfa->backhaulCheckAppl() = row->getChar(BACKHAULCHKAPPL);
      mfa->backhaulStopTktInd() = row->getChar(BACKHAULSTOPTKTIND);
      mfa->dmcCheckAppl() = row->getChar(DMCCHECKAPPL);
      mfa->dmcStopTktInd() = row->getChar(DMCSTOPTKTIND);
      mfa->comCheckAppl() = row->getChar(COMCHECKAPPL);
      mfa->comStopTktInd() = row->getChar(COMSTOPTKTIND);
      mfa->cpmCheckAppl() = row->getChar(CPMCHECKAPPL);
      mfa->cpmStopTktInd() = row->getChar(CPMSTOPTKTIND);

      dir = row->getString(BETWDIRECTIONALIND);
      if (dir == "F")
        mfa->betwDirectionalInd() = FROM;
      else if (dir == "W")
        mfa->betwDirectionalInd() = WITHIN;
      else if (dir == "O")
        mfa->betwDirectionalInd() = ORIGIN;
      else if (dir == "X")
        mfa->betwDirectionalInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        mfa->betwDirectionalInd() = BETWEEN;

      loc = &mfa->betwLoc1();
      loc->locType() = row->getChar(BETWLOC1TYPE);
      loc->loc() = row->getString(BETWLOC1);

      loc = &mfa->betwLoc2();
      loc->locType() = row->getChar(BETWLOC2TYPE);
      loc->loc() = row->getString(BETWLOC2);

      mfa->serviceRestr() = row->getChar(SERVICERESTR);
      mfa->nonStopInd() = row->getChar(NONSTOPIND);

      dir = row->getString(DIRECTIND);
      if (dir == "F")
        mfa->directInd() = FROM;
      else if (dir == "W")
        mfa->directInd() = WITHIN;
      else if (dir == "O")
        mfa->directInd() = ORIGIN;
      else if (dir == "X")
        mfa->directInd() = TERMINATE;
      else if (dir.empty() || dir == " " || dir == "B")
        mfa->directInd() = BETWEEN;

      mfa->constructPointInd() = row->getChar(CONSTRUCTPOINTIND);
      mfa->exceptCxrFltRestr() = row->getChar(EXCEPTCXRFLTRESTR);
      mfa->exceptSecondaryCxr() = row->getChar(EXCEPTSECONDARYCXR);

      mfa->posExceptInd() = row->getChar(POSEXCEPTIND);
      loc = &mfa->posLoc();
      loc->locType() = row->getChar(POSLOCTYPE);
      loc->loc() = row->getString(POSLOC);

      mfa->poiExceptInd() = row->getChar(POIEXCEPTIND);
      loc = &mfa->poiLoc();
      loc->locType() = row->getChar(POILOCTYPE);
      loc->loc() = row->getString(POILOC);

      mfa->sotiInd() = row->getChar(SOTIIND);
      mfa->sotoInd() = row->getChar(SOTOIND);
      mfa->sitiInd() = row->getChar(SITIIND);
      mfa->sitoInd() = row->getChar(SITOIND);

      mfa->applyDefaultLogic() = row->getChar(APPLYDEFAULTLOGIC);

      // Not going to Default table, so load rest of fields here!
      mfa->domAppl() = row->getChar(DOMAPPL);
      mfa->domExceptInd() = row->getChar(DOMEXCEPTIND);

      loc = &mfa->domLoc();
      loc->locType() = row->getChar(DOMLOCTYPE);
      loc->loc() = row->getString(DOMLOC);

      mfa->nmlFareCompareInd() = row->getChar(NMLFARECOMPAREIND);
      mfa->nmlMpmBeforeRtgInd() = row->getChar(NMLMPMBEFORERTGIND);
      mfa->nmlRtgBeforeMpmInd() = row->getChar(NMLRTGBEFOREMPMIND);
      mfa->nmlHipRestrCompInd() = row->getChar(NMLHIPRESTRCOMPIND);
      mfa->nmlHipUnrestrCompInd() = row->getChar(NMLHIPUNRESTRCOMPIND);
      mfa->nmlHipRbdCompInd() = row->getChar(NMLHIPRBDCOMPIND);
      mfa->nmlHipStopCompInd() = row->getChar(NMLHIPSTOPCOMPIND);
      mfa->nmlHipOrigInd() = row->getChar(NMLHIPORIGIND);
      mfa->nmlHipOrigNationInd() = row->getChar(NMLHIPORIGNATIONIND);
      mfa->nmlHipFromInterInd() = row->getChar(NMLHIPFROMINTERIND);
      mfa->nmlHipDestInd() = row->getChar(NMLHIPDESTIND);
      mfa->nmlHipDestNationInd() = row->getChar(NMLHIPDESTNATIONIND);
      mfa->nmlHipToInterInd() = row->getChar(NMLHIPTOINTERIND);
      mfa->nmlHipExemptInterToInter() = row->getChar(NMLHIPNOINTERTOINTER);
      mfa->spclHipTariffCatInd() = row->getChar(SPCLHIPTARIFFCATIND);
      mfa->spclHipRuleTrfInd() = row->getChar(SPCLHIPRULETRFIND);
      mfa->spclHipFareClassInd() = row->getChar(SPCLHIPFARECLASSIND);
      mfa->spclHip1stCharInd() = row->getChar(SPCLHIP1STCHARIND);
      mfa->spclHipStopCompInd() = row->getChar(SPCLHIPSTOPCOMPIND);
      mfa->spclHipSpclOnlyInd() = row->getChar(SPCLHIPSPCLONLYIND);

      loc = &mfa->spclHipLoc();
      loc->locType() = row->getChar(SPCLHIPLOCTYPE);
      loc->loc() = row->getString(SPCLHIPLOC);

      mfa->spclHipOrigInd() = row->getChar(SPCLHIPORIGIND);
      mfa->spclHipOrigNationInd() = row->getChar(SPCLHIPORIGNATIONIND);
      mfa->spclHipFromInterInd() = row->getChar(SPCLHIPFROMINTERIND);
      mfa->spclHipDestInd() = row->getChar(SPCLHIPDESTIND);
      mfa->spclHipDestNationInd() = row->getChar(SPCLHIPDESTNATIONIND);
      mfa->specialProcessName() = row->getString(SPECIALPROCESSNAME);
      mfa->spclHipToInterInd() = row->getChar(SPCLHIPTOINTERIND);
      mfa->spclHipExemptInterToInter() = row->getChar(SPCLHIPNOINTERTOINTER);
      mfa->nmlCtmRestrCompInd() = row->getChar(NMLCTMRESTRCOMPIND);
      mfa->nmlCtmUnrestrCompInd() = row->getChar(NMLCTMUNRESTRCOMPIND);
      mfa->nmlCtmRbdCompInd() = row->getChar(NMLCTMRBDCOMPIND);
      mfa->nmlCtmStopCompInd() = row->getChar(NMLCTMSTOPCOMPIND);
      mfa->nmlCtmOrigInd() = row->getChar(NMLCTMORIGIND);
      mfa->nmlCtmDestNationInd() = row->getChar(NMLCTMDESTNATIONIND);
      mfa->nmlCtmToInterInd() = row->getChar(NMLCTMTOINTERIND);
      mfa->spclCtmTariffCatInd() = row->getChar(SPCLCTMTARIFFCATIND);
      mfa->spclCtmRuleTrfInd() = row->getChar(SPCLCTMRULETRFIND);
      mfa->spclCtmFareClassInd() = row->getChar(SPCLCTMFARECLASSIND);
      mfa->spclSame1stCharFBInd2() = row->getChar(SPCLSAME1STCHARFBIND2);
      mfa->spclCtmStopCompInd() = row->getChar(SPCLCTMSTOPCOMPIND);
      mfa->spclCtmMktComp() = row->getChar(SPCLCTMMKTCOMP);
      mfa->spclCtmOrigInd() = row->getChar(SPCLCTMORIGIND);
      mfa->spclCtmDestNationInd() = row->getChar(SPCLCTMDESTNATIONIND);
      mfa->spclCtmToInterInd() = row->getChar(SPCLCTMTOINTERIND);
      mfa->cpmExcl() = row->getChar(CPMEXCL);
      mfa->intHipCheckAppl() = row->getChar(INTHIPCHECKAPPL);
      mfa->intHipStopTktInd() = row->getChar(INTHIPSTOPTKTIND);
      mfa->intCtmCheckAppl() = row->getChar(INTCTMCHECKAPPL);
      mfa->intCtmStopTktInd() = row->getChar(INTCTMSTOPTKTIND);
      mfa->intBackhaulChkAppl() = row->getChar(INTBACKHAULCHKAPPL);
      mfa->intBackhaulStopTktInd() = row->getChar(INTBACKHAULSTOPTKTIND);
      mfa->intDmcCheckAppl() = row->getChar(INTDMCCHECKAPPL);
      mfa->intDmcStopTktInd() = row->getChar(INTDMCSTOPTKTIND);
      mfa->intComCheckAppl() = row->getChar(INTCOMCHECKAPPL);
      mfa->intCpmCheckAppl() = row->getChar(INTCPMCHECKAPPL);
      mfa->intCpmCheckAppl() = row->getChar(INTCPMCHECKAPPL);
      mfa->intCpmStopTktInd() = row->getChar(INTCPMSTOPTKTIND);
      mfa->domFareTypeExcept() = row->getChar(DOMFARETYPEEXCEPT);
      mfa->nmlHipOrigNationTktPt() = row->getChar(NMLHIPORIGNATIONTKTPT);
      mfa->nmlHipOrigNationStopPt() = row->getChar(NMLHIPORIGNATIONSTOPPT);
      mfa->nmlHipStopoverPt() = row->getChar(NMLHIPSTOPOVERPT);
      mfa->nmlHipTicketedPt() = row->getChar(NMLHIPTICKETEDPT);
      mfa->spclHipOrigNationTktPt() = row->getChar(SPCLHIPORIGNATIONTKTPT);
      mfa->spclHipOrigNationStopPt() = row->getChar(SPCLHIPORIGNATIONSTOPPT);
      mfa->spclHipStopoverPt() = row->getChar(SPCLHIPSTOPOVERPT);
      mfa->spclHipTicketedPt() = row->getChar(SPCLHIPTICKETEDPT);
    } // else (new parent)

    // Check for Child
    if (!row->isNull(CARRIER))
    {
      FareTypeAbbrev cxr = row->getString(CARRIER);
      mfa->fareTypes().push_back(cxr);
    }

    return mfa;
  } // mapRowToMinFareApplBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareApplBaseSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where and OrderBy clauses
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareApplBaseHistoricalSQLStatement
    : public QueryGetMinFareApplBaseSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" m.TEXTTBLVENDOR in ('', %1q)"
                " and m.TEXTTBLITEMNO in (0, %2n)"
                " and m.GOVERNINGCARRIER in ('', %3q)"
                " and %4n <= m.EXPIREDATE"
                " and (%5n >=  m.CREATEDATE or %6n >= m.EFFDATE)");
    this->OrderBy("TEXTTBLVENDOR desc,TEXTTBLITEMNO desc,GOVERNINGCARRIER desc,SEQNO,CREATEDATE");
  }
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareFareClasses
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareFareClassesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareFareClassesSQLStatement() {};
  virtual ~QueryGetMinFareFareClassesSQLStatement() {};

  enum ColumnIndexes
  {
    FARECLASS = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARECLASS");
    this->From("=MINFAREFARECLASS");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareClassCode mapRowToFareClass(Row* row) { return row->getString(FARECLASS); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareFareClassesSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareCxrFltRestrs
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareCxrFltRestrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareCxrFltRestrsSQLStatement() {};
  virtual ~QueryGetMinFareCxrFltRestrsSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    FLIGHTNO1,
    FLIGHTNO2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,FLIGHTNO1,FLIGHTNO2");
    this->From("=MINFARECXRFLTRESTR");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MinFareCxrFltRestr* mapRowToMinFareCxrFltRestr(Row* row)
  {
    MinFareCxrFltRestr* cfr = new MinFareCxrFltRestr;

    cfr->carrier() = row->getString(CARRIER);
    cfr->flight1() = row->getInt(FLIGHTNO1);
    cfr->flight2() = row->getInt(FLIGHTNO2);

    return cfr;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareCxrFltRestrsSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareSecCxrRestrs
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareSecCxrRestrsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareSecCxrRestrsSQLStatement() {};
  virtual ~QueryGetMinFareSecCxrRestrsSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER");
    this->From("=MINFARESECCXRRESTR");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CarrierCode mapRowToCarrier(Row* row) { return row->getString(CARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareSecCxrRestrsSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareDomFareTypes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareDomFareTypesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareDomFareTypesSQLStatement() {};
  virtual ~QueryGetMinFareDomFareTypesSQLStatement() {};

  enum ColumnIndexes
  {
    FARETYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARETYPE");
    this->From("=MINFAREDOMFARETYPE");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareTypeAbbrev mapRowToFareType(Row* row) { return row->getString(FARETYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareDomFareTypesSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareFareTypes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareFareTypesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareFareTypesSQLStatement() {};
  virtual ~QueryGetMinFareFareTypesSQLStatement() {};

  enum ColumnIndexes
  {
    FARETYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARETYPE");
    this->From("=MINFAREFARETYPE");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareTypeAbbrev mapRowToFareType(Row* row) { return row->getString(FARETYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareFareTypesSQLStatement

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMinFareRules
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinFareRulesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinFareRulesSQLStatement() {};
  virtual ~QueryGetMinFareRulesSQLStatement() {};

  enum ColumnIndexes
  {
    RULE = 0,
    FOOTNOTE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select RULE, FOOTNOTE");
    this->From("=MINFARERULE");
    this->Where("TEXTTBLVENDOR = %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and VERSIONDATE = %4n"
                " and SEQNO = %5n"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static RuleNumber mapRowToRule(Row* row) { return row->getString(RULE); }
  static Footnote mapRowToFootnote(Row* row) { return row->getString(FOOTNOTE); }
  static bool hasRowToFootnote(Row* row) { return !row->isNull(FOOTNOTE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinFareRulesSQLStatement
} // tse
