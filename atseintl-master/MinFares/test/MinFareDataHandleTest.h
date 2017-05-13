#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/CopParticipatingNation.h"
#include "DBAccess/CopMinimum.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/FareTypeMatrix.h"
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "test/testdata/TestContainerFactory.h"
#include "test/testdata/TestMinFareApplFactory.h"
#include <boost/assign.hpp>

using namespace boost::assign;

namespace tse
{
class MinFareDataHandleTest : public DataHandleMock
{
  TestMemHandle _memHandle;
  CopParticipatingNation* getCop(const NationCode& nation, const NationCode& copNation)
  {
    CopParticipatingNation* ret = _memHandle.create<CopParticipatingNation>();
    ret->nation() = nation;
    ret->copNation() = copNation;
    return ret;
  }
  IndustryPricingAppl* getInd(CarrierCode carrier,
                              GlobalDirection globalDir,
                              Indicator primePricingAppl,
                              Indicator minimumFareAppl,
                              LocTypeCode lt1,
                              LocCode lc1,
                              LocTypeCode lt2,
                              LocCode lc2,
                              Directionality directionality)
  {
    IndustryPricingAppl* ret = _memHandle.create<IndustryPricingAppl>();
    ret->carrier() = carrier;
    ret->globalDir() = globalDir;
    ret->primePricingAppl() = primePricingAppl;
    ret->minimumFareAppl() = minimumFareAppl;
    ret->loc1().locType() = lt1;
    ret->loc1().loc() = lc1;
    ret->loc2().locType() = lt2;
    ret->loc2().loc() = lc2;
    ret->directionality() = directionality;
    return ret;
  }
  FareTypeMatrix* getFTM(FareType ft,
                         int seqNo,
                         Indicator cabin,
                         int fareTypeDesig,
                         Indicator fareTypeAppl,
                         Indicator restrInd,
                         Indicator fareTypeDisplay)
  {
    FareTypeMatrix* ret = _memHandle.create<FareTypeMatrix>();
    ret->fareType() = ft;
    ret->cabin().setClass(cabin);
    ret->fareTypeDesig().setFareTypeDesignator(fareTypeDesig);
    ret->fareTypeAppl() = fareTypeAppl;
    ret->restrInd() = restrInd;
    ret->fareTypeDisplay() = fareTypeDisplay;
    ret->versioninheritedInd() = 'N';
    ret->versionDisplayInd() = 'Y';
    return ret;
  }

public:
  const bool isHistorical() { return false; }
  const CopParticipatingNation* getCopParticipatingNation(const NationCode& nation,
                                                          const NationCode& copNation,
                                                          const DateTime& date)
  {
    if (copNation == "FR")
    {
      if (nation == "GB" || nation == "US" || nation == "AZ" || nation == "HK" || nation == "JP" ||
          nation == "AZ" || nation == "SG" || nation == "UZ" || nation == "RU" || nation == "TM" ||
          nation == "UA" || nation == "ZA")
        return 0;
      else if (nation == "FR" || nation == "GP" || nation == "MQ" || nation == "RE")
        return getCop(nation, copNation);
    }
    else if (copNation == "RU")
    {
      if (nation == "HK" || nation == "JP" || nation == "GB" || nation == "ZA" || nation == "US" ||
          nation == "SG")
        return 0;
      if (nation == "AZ" || nation == "RU" || nation == "UZ" || nation == "UA" || nation == "TM")
        return getCop(nation, copNation);
    }
    return DataHandleMock::getCopParticipatingNation(nation, copNation, date);
  }
  const std::vector<CopMinimum*>& getCopMinimum(const NationCode& key, const DateTime& date)
  {
    if (key == "FR")
    {
      std::vector<CopMinimum*>* ret = _memHandle.create<std::vector<CopMinimum*> >();
      ret->push_back(_memHandle.create<CopMinimum>());
      return *ret;
    }
    else if (key == "RU" || key == "SG")
      return *_memHandle.create<std::vector<CopMinimum*> >();
    return DataHandleMock::getCopMinimum(key, date);
  }
  const std::vector<GeneralFareRuleInfo*>&
  getGeneralFareRule(const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& rule,
                     const CatNumber& category,
                     const DateTime& date,
                     const DateTime& applDate = DateTime::emptyDate())
  {
    if (ruleTariff == 0 || ruleTariff > 999)
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    else if (carrier == "KE")
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    else if (carrier == "BA" && ruleTariff == 304)
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();
    else if (carrier == "")
      return *_memHandle.create<std::vector<GeneralFareRuleInfo*> >();

    return DataHandleMock::getGeneralFareRule(
        vendor, carrier, ruleTariff, rule, category, date, applDate);
  }
  GeneralRuleApp* getGeneralRuleApp(const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& tariffNumber,
                                    const RuleNumber& ruleNumber,
                                    CatNumber catNum)
  {
    if (carrier == "BA" && tariffNumber == 304)
      return 0;
    else if (carrier == "KE" && tariffNumber == 304)
      return 0;
    else if (tariffNumber == 0 || tariffNumber > 999)
      return 0;
    else if (carrier == "")
      return 0;
    return DataHandleMock::getGeneralRuleApp(vendor, carrier, tariffNumber, ruleNumber, catNum);
  }
  bool getGeneralRuleAppTariffRule(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const TariffNumber& tariffNumber,
                                   const RuleNumber& ruleNumber,
                                   CatNumber catNum,
                                   RuleNumber& ruleNumOut,
                                   TariffNumber& tariffNumOut)
  {
    if (tariffNumber > 999 || tariffNumber == 0)
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRule(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut);
  }
  //msd
  GeneralRuleApp* getGeneralRuleAppByTvlDate(const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const TariffNumber& tariffNumber,
                                             const RuleNumber& ruleNumber,
                                             CatNumber catNum,
                                             const DateTime& tvlDate)
  {
    if (carrier == "BA" && tariffNumber == 304)
      return 0;
    else if (carrier == "KE" && tariffNumber == 304)
      return 0;
    else if (tariffNumber == 0 || tariffNumber > 999)
      return 0;
    else if (carrier == "")
      return 0;
    return DataHandleMock::getGeneralRuleAppByTvlDate(vendor, carrier, tariffNumber, ruleNumber, catNum, tvlDate);
  }

  bool getGeneralRuleAppTariffRuleByTvlDate(const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& tariffNumber,
                                            const RuleNumber& ruleNumber,
                                            CatNumber catNum,
                                            RuleNumber& ruleNumOut,
                                            TariffNumber& tariffNumOut,
                                            const DateTime& tvlDate)
  {
    if (tariffNumber > 999 || tariffNumber == 0)
      return false;
    return DataHandleMock::getGeneralRuleAppTariffRuleByTvlDate(
        vendor, carrier, tariffNumber, ruleNumber, catNum, ruleNumOut, tariffNumOut, tvlDate);
  }
  //msd
  const std::vector<MinFareAppl*>& getMinFareAppl(const VendorCode& textTblVendor,
                                                  int textTblItemNo,
                                                  const CarrierCode& governingCarrier,
                                                  const DateTime& date)
  {
    if (governingCarrier == "BA")
      return *_memHandle.create<std::vector<MinFareAppl*> >();
    else if (governingCarrier == "KE")
      return *_memHandle.create<std::vector<MinFareAppl*> >();
    if (governingCarrier == "SR")
      return *_memHandle.create<std::vector<MinFareAppl*> >();
    else if (governingCarrier == "OA")
      return *TestVectorFactory<TestMinFareApplFactory, MinFareAppl>::create(
                 "/vobs/atseintl/MinFares/test/data/MinFareAppl_OA.xml");
    else if (governingCarrier == "AZ")
      return *TestVectorFactory<TestMinFareApplFactory, MinFareAppl>::create(
                 "/vobs/atseintl/MinFares/test/data/MinFareAppl_AZ.xml");
    return DataHandleMock::getMinFareAppl(textTblVendor, textTblItemNo, governingCarrier, date);
  }
  const PaxTypeInfo* getPaxType(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
  {
    if (paxTypeCode == "ADT")
      return DataHandleMock::getPaxType("ADT", "ATP");
    else if (paxTypeCode == "")
      return 0;
    return DataHandleMock::getPaxType(paxTypeCode, vendor);
  }
  const std::vector<MinFareDefaultLogic*>&
  getMinFareDefaultLogic(const VendorCode& vendor, const CarrierCode& carrier)
  {
    if (carrier == "BA" || carrier == "OA" || carrier == "AZ")
    {
      std::vector<MinFareDefaultLogic*>* ret =
          _memHandle.create<std::vector<MinFareDefaultLogic*> >();
      MinFareDefaultLogic* m = _memHandle.create<MinFareDefaultLogic>();
      m->seqNo() = 100;
      m->nmlHipTariffCatInd() = -1;
      m->nmlCtmTariffCatInd() = -1;
      m->domAppl() = 'I';
      m->domExceptInd() = 'N';
      m->nmlFareCompareInd() = 'N';
      m->nmlMpmBeforeRtgInd() = 'N';
      m->nmlRtgBeforeMpmInd() = 'N';
      m->nmlHipRestrCompInd() = 'A';
      m->nmlHipUnrestrCompInd() = 'A';
      m->nmlHipRbdCompInd() = 'N';
      m->nmlHipStopCompInd() = 'N';
      m->nmlHipOrigInd() = 'Y';
      m->nmlHipOrigNationInd() = 'N';
      m->nmlHipFromInterInd() = 'Y';
      m->nmlHipDestInd() = 'Y';
      m->nmlHipDestNationInd() = 'N';
      m->nmlHipToInterInd() = 'Y';
      m->nmlHipExemptInterToInter() = 'N';
      m->spclHipTariffCatInd() = 'N';
      m->spclHipRuleTrfInd() = 'N';
      m->spclHipFareClassInd() = 'N';
      m->spclHip1stCharInd() = 'N';
      m->spclHipStopCompInd() = 'N';
      m->spclHipSpclOnlyInd() = 'N';
      m->spclHipOrigInd() = 'Y';
      m->spclHipOrigNationInd() = 'N';
      m->spclHipFromInterInd() = 'Y';
      m->spclHipDestInd() = 'Y';
      m->spclHipDestNationInd() = 'N';
      m->specialProcessName() = "INDUSTRY";
      m->spclHipToInterInd() = 'Y';
      m->spclHipExemptInterToInter() = 'A';
      m->nmlCtmRestrCompInd() = 'A';
      m->nmlCtmUnrestrCompInd() = 'A';
      m->nmlCtmRbdCompInd() = 'N';
      m->nmlCtmStopCompInd() = 'N';
      m->nmlCtmOrigInd() = 'N';
      m->nmlCtmDestNationInd() = 'N';
      m->nmlCtmToInterInd() = 'Y';
      m->spclCtmTariffCatInd() = 'N';
      m->spclCtmRuleTrfInd() = 'N';
      m->spclCtmFareClassInd() = 'N';
      m->spclSame1stCharFBInd2() = 'N';
      m->spclCtmStopCompInd() = 'N';
      m->spclCtmMktComp() = 'E';
      m->spclCtmOrigInd() = 'N';
      m->spclCtmDestNationInd() = 'N';
      m->spclCtmToInterInd() = 'Y';
      m->cpmExcl() = 'N';
      m->domFareTypeExcept() = 'N';
      m->nmlHipOrigNationTktPt() = 'N';
      m->nmlHipOrigNationStopPt() = 'N';
      m->nmlHipStopoverPt() = 'N';
      m->nmlHipTicketedPt() = 'N';
      m->spclHipOrigNationTktPt() = 'N';
      m->spclHipOrigNationStopPt() = 'N';
      m->spclHipStopoverPt() = 'N';
      m->spclHipTicketedPt() = 'N';
      ret->push_back(m);
      return *ret;
    }
    return DataHandleMock::getMinFareDefaultLogic(vendor, carrier);
  }
  const std::vector<MinFareRuleLevelExcl*>&
  getMinFareRuleLevelExcl(const VendorCode& vendor,
                          int textTblItemNo,
                          const CarrierCode& governingCarrier,
                          const TariffNumber& rule,
                          const DateTime& date)
  {
    if (governingCarrier == "KE")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "BA")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (rule == 23456)
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "AZ")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "GH")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "OA")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "SR")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "SA")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();
    else if (governingCarrier == "")
      return *_memHandle.create<std::vector<MinFareRuleLevelExcl*> >();

    return DataHandleMock::getMinFareRuleLevelExcl(
        vendor, textTblItemNo, governingCarrier, rule, date);
  }
  const FareTypeMatrix* getFareTypeMatrix(const FareType& key, const DateTime& date)
  {
    if (key == "PGV")
      return getFTM("PGV", 6900, '8', 90, 'S', ' ', 'G');
    else if (key == "PIT")
      return getFTM("PIT", 6700, '8', 90, 'S', ' ', 'I');
    else if (key == "XND")
      return getFTM("XND", 3900, '8', 40, 'S', ' ', 'E');
    else if (key == "EU")
      return getFTM("EU", 2219, '8', 35, 'N', 'U', 'N');
    else if (key == "BR")
      return getFTM("BR", 1500, '5', 25, 'N', 'R', 'N');
    else if (key == "BU")
      return getFTM("BU", 1400, '5', 25, 'N', 'U', 'N');
    else if (key == "ER")
      return getFTM("ER", 2500, '8', 35, 'N', 'R', 'N');
    return DataHandleMock::getFareTypeMatrix(key, date);
  }
  const std::vector<const IndustryPricingAppl*>&
  getIndustryPricingAppl(const CarrierCode& carrier,
                         const GlobalDirection& globalDir,
                         const DateTime& date)
  {
    if (carrier == "BA")
    {
      std::vector<const IndustryPricingAppl*>* ret =
          _memHandle.create<std::vector<const IndustryPricingAppl*> >();
      ret->push_back(getInd(carrier, globalDir, 'L', 'I', 'A', "1", 'A', "3", BETWEEN));
      ret->push_back(getInd(carrier, globalDir, 'C', 'I', ' ', "", ' ', " ", BETWEEN));
      return *ret;
    }
    else if (carrier == "SA" || carrier == "AZ")
    {
      std::vector<const IndustryPricingAppl*>* ret =
          _memHandle.create<std::vector<const IndustryPricingAppl*> >();
      ret->push_back(getInd(carrier, globalDir, 'C', 'I', ' ', "", ' ', "", BETWEEN));
      return *ret;
    }
    else if (carrier == "OA" || carrier == "GH")
      return *_memHandle.create<std::vector<const IndustryPricingAppl*> >();
    return DataHandleMock::getIndustryPricingAppl(carrier, globalDir, date);
  }
  const std::vector<FareTypeMatrix*>& getAllFareTypeMatrix(const DateTime& date)
  {
    std::vector<FareTypeMatrix*>& ret = *_memHandle.create<std::vector<FareTypeMatrix*> >();
    ret += getFTM("AC", 8800, '5', 80, 'N', 'U', 'N'), getFTM("AD", 9100, '8', 80, 'N', 'U', 'N'),
        getFTM("AF", 8700, '2', 80, 'N', 'U', 'N'), getFTM("AS", 9000, '8', 80, 'N', 'U', 'N'),
        getFTM("AY", 8900, '8', 80, 'N', 'U', 'N'), getFTM("BAP", 1750, '5', 25, 'S', ' ', 'E'),
        getFTM("BAR", 1775, '5', 25, 'S', ' ', 'E'), getFTM("BIT", 2050, '5', 90, 'S', ' ', 'I'),
        getFTM("BND", 1800, '5', 25, 'S', ' ', 'N'), getFTM("BON", 1737, '5', 25, 'S', ' ', 'E'),
        getFTM("BOR", 1725, '5', 25, 'S', ' ', 'E'), getFTM("BOX", 1700, '5', 25, 'S', ' ', 'E'),
        getFTM("BPR", 2150, '5', 90, 'S', ' ', 'S'), getFTM("BR", 1500, '5', 25, 'N', 'R', 'N'),
        getFTM("BRO", 2100, '5', 90, 'S', ' ', 'S'), getFTM("BRW", 1900, '5', 25, 'S', ' ', 'E'),
        getFTM("BS", 1450, '5', 25, 'N', 'U', 'N'), getFTM("BT", 1475, '5', 25, 'N', 'U', 'N'),
        getFTM("BTP", 2000, '5', 90, 'S', ' ', 'S'), getFTM("BU", 1400, '5', 25, 'N', 'U', 'N'),
        getFTM("BX", 1600, '5', 25, 'S', ' ', 'E'), getFTM("BXN", 1650, '5', 25, 'S', ' ', 'E'),
        getFTM("BXR", 1625, '5', 25, 'S', ' ', 'E'), getFTM("EAP", 3000, '8', 55, 'S', ' ', 'N'),
        getFTM("EBN", 3200, '8', 65, 'S', ' ', 'N'), getFTM("EIP", 3100, '8', 65, 'S', ' ', 'N'),
        getFTM("END", 2800, '8', 35, 'S', ' ', 'N'), getFTM("EPR", 8650, '8', 90, 'S', ' ', 'S'),
        getFTM("ER", 2500, '8', 35, 'N', 'R', 'N'), getFTM("ERS", 2600, '8', 35, 'N', 'R', 'N'),
        getFTM("ERT", 2700, '8', 35, 'N', 'R', 'N'), getFTM("ERW", 2900, '8', 35, 'S', ' ', 'E'),
        getFTM("ES", 2229, '8', 35, 'N', 'U', 'N'), getFTM("ET", 2400, '8', 35, 'N', 'U', 'N'),
        getFTM("EU", 2219, '8', 35, 'N', 'U', 'N'), getFTM("FIT", 1250, '2', 90, 'S', ' ', 'I'),
        getFTM("FND", 1000, '2', 15, 'S', ' ', 'N'), getFTM("FOX", 900, '2', 15, 'S', ' ', 'E'),
        getFTM("FPR", 1350, '2', 90, 'S', ' ', 'S'), getFTM("FR", 600, '2', 15, 'N', 'R', 'N'),
        getFTM("FRO", 1300, '2', 90, 'S', ' ', 'S'), getFTM("FRW", 1100, '2', 15, 'S', ' ', 'E'),
        getFTM("FS", 700, '2', 15, 'N', 'U', 'N'), getFTM("FTP", 1200, '2', 90, 'S', ' ', 'S'),
        getFTM("FU", 500, '2', 15, 'N', 'U', 'N'), getFTM("FX", 800, '2', 15, 'S', ' ', 'E'),
        getFTM("JR", 1387, '5', 25, 'N', 'R', 'N'), getFTM("JU", 1375, '5', 25, 'N', 'U', 'N'),
        getFTM("PBT", 6800, '8', 90, 'S', ' ', 'I'), getFTM("PCF", 8300, '8', 90, 'S', ' ', 'S'),
        getFTM("PCR", 8000, '8', 90, 'S', ' ', 'S'), getFTM("PEM", 7800, '8', 90, 'S', ' ', 'S'),
        getFTM("PFC", 8175, '8', 90, 'S', ' ', 'S'), getFTM("PFN", 8187, '8', 90, 'S', ' ', 'S'),
        getFTM("PFS", 8193, '8', 90, 'S', ' ', 'S'), getFTM("PFU", 8196, '8', 90, 'S', ' ', 'S'),
        getFTM("PG", 6200, '8', 90, 'S', ' ', 'S'), getFTM("PGA", 7000, '8', 90, 'S', ' ', 'G'),
        getFTM("PGC", 7100, '8', 90, 'S', ' ', 'G'), getFTM("PGI", 7200, '8', 90, 'S', ' ', 'G'),
        getFTM("PGL", 7150, '8', 90, 'S', ' ', 'G'), getFTM("PGM", 7500, '8', 90, 'S', ' ', 'G'),
        getFTM("PGN", 7300, '8', 90, 'S', ' ', 'G'), getFTM("PGO", 7400, '8', 90, 'S', ' ', 'G'),
        getFTM("PGP", 7650, '8', 90, 'S', ' ', 'G'), getFTM("PGV", 6900, '8', 90, 'S', ' ', 'G'),
        getFTM("PGX", 6950, '8', 90, 'S', ' ', 'G'), getFTM("PGZ", 7600, '8', 90, 'S', ' ', 'G'),
        getFTM("PIT", 6700, '8', 90, 'S', ' ', 'I'), getFTM("PM", 6100, '8', 90, 'S', ' ', 'S'),
        getFTM("PMI", 7700, '8', 90, 'S', ' ', 'S'), getFTM("PMN", 6150, '8', 90, 'S', ' ', 'S'),
        getFTM("PRO", 8600, '8', 90, 'S', ' ', 'S'), getFTM("PS", 7900, '8', 90, 'S', ' ', 'S'),
        getFTM("PSA", 8100, '8', 90, 'S', ' ', 'S'), getFTM("PSD", 6500, '8', 90, 'S', ' ', 'S'),
        getFTM("PST", 8150, '8', 90, 'S', ' ', 'S'), getFTM("PSZ", 6600, '8', 90, 'S', ' ', 'S'),
        getFTM("PTP", 8500, '8', 90, 'S', ' ', 'S'), getFTM("PU", 6300, '8', 90, 'S', ' ', 'S'),
        getFTM("PVU", 8200, '8', 90, 'S', ' ', 'S'), getFTM("PZ", 6400, '8', 90, 'S', ' ', 'S'),
        getFTM("RR", 375, '1', 10, 'N', 'R', 'N'), getFTM("RU", 250, '1', 10, 'N', 'U', 'N'),
        getFTM("SAP", 5800, '8', 50, 'S', ' ', 'N'), getFTM("SB", 5700, '8', 70, 'S', ' ', 'N'),
        getFTM("SBP", 6000, '8', 60, 'S', ' ', 'N'), getFTM("SH", 8400, '8', 90, 'S', ' ', 'S'),
        getFTM("SIP", 5900, '8', 60, 'S', ' ', 'N'), getFTM("WR", 2187, '7', 30, 'N', 'R', 'N'),
        getFTM("WU", 2175, '7', 30, 'N', 'U', 'N'), getFTM("XAB", 4400, '8', 55, 'S', ' ', 'E'),
        getFTM("XAC", 4500, '8', 55, 'S', ' ', 'E'), getFTM("XAN", 4300, '8', 55, 'S', ' ', 'E'),
        getFTM("XAP", 4000, '8', 55, 'S', ' ', 'E'), getFTM("XAT", 4200, '8', 55, 'S', ' ', 'E'),
        getFTM("XAW", 4100, '8', 55, 'S', ' ', 'E'), getFTM("XBB", 4600, '8', 55, 'S', ' ', 'E'),
        getFTM("XBN", 4700, '8', 55, 'S', ' ', 'E'), getFTM("XEF", 3600, '8', 40, 'S', ' ', 'E'),
        getFTM("XEL", 3500, '8', 40, 'S', ' ', 'E'), getFTM("XES", 3400, '8', 40, 'S', ' ', 'E'),
        getFTM("XEV", 3700, '8', 40, 'S', ' ', 'E'), getFTM("XEX", 3300, '8', 40, 'S', ' ', 'E'),
        getFTM("XND", 3900, '8', 40, 'S', ' ', 'E'), getFTM("XOA", 3850, '8', 40, 'S', ' ', 'E'),
        getFTM("XOF", 3887, '8', 40, 'S', ' ', 'E'), getFTM("XOL", 3875, '8', 40, 'S', ' ', 'E'),
        getFTM("XOV", 3893, '8', 40, 'S', ' ', 'E'), getFTM("XOW", 3896, '8', 40, 'S', ' ', 'E'),
        getFTM("XOX", 3800, '8', 40, 'S', ' ', 'E'), getFTM("XPB", 5500, '8', 65, 'S', ' ', 'E'),
        getFTM("XPC", 5600, '8', 65, 'S', ' ', 'E'), getFTM("XPF", 5100, '8', 65, 'S', ' ', 'E'),
        getFTM("XPL", 5000, '8', 65, 'S', ' ', 'E'), getFTM("XPN", 5300, '8', 65, 'S', ' ', 'E'),
        getFTM("XPO", 5450, '8', 60, 'S', ' ', 'N'), getFTM("XPS", 4900, '8', 65, 'S', ' ', 'E'),
        getFTM("XPV", 5400, '8', 65, 'S', ' ', 'E'), getFTM("XPX", 4800, '8', 65, 'S', ' ', 'E'),
        getFTM("XPY", 5200, '8', 65, 'S', ' ', 'E'), getFTM("ZAP", 2198, '7', 30, 'S', ' ', 'E'),
        getFTM("ZEX", 2193, '7', 30, 'S', ' ', 'E'), getFTM("ZIP", 2214, '7', 30, 'S', ' ', 'N'),
        getFTM("ZOX", 2196, '7', 30, 'S', ' ', 'E'), getFTM("ZPX", 2209, '7', 30, 'S', ' ', 'E'),
        getFTM("ZZP", 2199, '7', 30, 'S', ' ', 'N');
    return ret;
  }
  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "XX")
      return DataHandleMock::getCarrierPreference("", date);
    return DataHandleMock::getCarrierPreference(carrier, date);
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "FRA")
      return "FRA";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
};
}
