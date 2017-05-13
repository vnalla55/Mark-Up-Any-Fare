#include "AddonConstruction/AddonConstruction.h"
#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/CombFareClassMap.h"
#include "AddonConstruction/GatewayPair.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "AddonConstruction/SmfGatewayPair.h"
#include "AddonConstruction/SpecifiedFareCache.h"
#include "AddonConstruction/VendorAtpco.h"
#include "AddonConstruction/VendorSita.h"
#include "AddonConstruction/VendorSmf.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  TariffCrossRefInfo* getTXRef(GlobalDirection gd,
                               TariffNumber fareTariff,
                               TariffCode fareTariffCode,
                               TariffCategory tariffCat,
                               TariffNumber ruleTariff,
                               TariffCode ruleTariffCode,
                               TariffNumber governingTariff,
                               TariffCode governingTariffCode,
                               TariffNumber routingTariff1,
                               TariffCode routingTariff1Code,
                               TariffNumber addonTariff1,
                               TariffCode addonTariff1Code,
                               TariffNumber addonTariff2,
                               TariffCode addonTariff2Code)
  {
    TariffCrossRefInfo* ret = _memHandle.create<TariffCrossRefInfo>();
    ret->vendor() = "ATP";
    ret->carrier() = "AA";
    ret->crossRefType() = INTERNATIONAL;
    ret->globalDirection() = gd;
    ret->fareTariff() = fareTariff;
    ret->fareTariffCode() = fareTariffCode;
    ret->tariffCat() = tariffCat;
    ret->ruleTariff() = ruleTariff;
    ret->ruleTariffCode() = ruleTariffCode;
    ret->governingTariff() = governingTariff;
    ret->governingTariffCode() = governingTariffCode;
    ret->routingTariff1() = routingTariff1;
    ret->routingTariff1Code() = routingTariff1Code;
    ret->routingTariff2() = -1;
    ret->routingTariff2Code() = "";
    ret->addonTariff1() = addonTariff1;
    ret->addonTariff1Code() = addonTariff1Code;
    ret->addonTariff2() = addonTariff2;
    ret->addonTariff2Code() = addonTariff2Code;
    return ret;
  }
  FareInfo*
  getFare(LocCode market1, LocCode market2, FareClassCode fareClass, TariffNumber fareTariff)
  {
    FareInfo* ret = _memHandle.create<FareInfo>();
    ret->market1() = market1;
    ret->market2() = market2;
    ret->vendor() = "ATP";
    ret->carrier() = "AA";
    ret->constructionInd() = ' ';
    ret->fareClass() = fareClass;
    ret->fareTariff() = fareTariff;
    ret->owrt() = '2';
    return ret;
  }

public:
  const bool isHistorical() { return false; }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "ATL")
      return "ATL";
    else if (locCode == "GLA")
      return "GLA";
    else if (locCode == "LAX")
      return "LAX";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<TariffCrossRefInfo*>& getTariffXRef(const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const RecordScope& crossRefType)
  {
    if (vendor == "ATP" && carrier == "AA")
    {
      std::vector<TariffCrossRefInfo*>& ret =
          *_memHandle.create<std::vector<TariffCrossRefInfo*> >();
      ret += getTXRef(
          GlobalDirection::AT, 1, "TAFP", 0, 1, "IPRA", 60, "IPRG", 1, "TARG", 999, "AUSA", -1, ""),
          getTXRef(GlobalDirection::PA,
                   3,
                   "TPFP",
                   0,
                   3,
                   "IPRP",
                   60,
                   "IPRG",
                   3,
                   "TPRG",
                   996,
                   "PUSA",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   5,
                   "WHFP",
                   0,
                   5,
                   "IPRW",
                   60,
                   "IPRG",
                   5,
                   "WHRG",
                   995,
                   "WUSA",
                   -1,
                   ""),
          getTXRef(
              GlobalDirection::EH, 8, "TPFG", 0, 8, "IPRPG", -1, "", 8, "TGRG", -1, "", -1, ""),
          getTXRef(GlobalDirection::PA,
                   18,
                   "SAAS",
                   0,
                   18,
                   "IPRSAAS",
                   60,
                   "IPRG",
                   18,
                   "SARG",
                   18,
                   "WARBSPA",
                   966,
                   "PARBSPA"),
          getTXRef(GlobalDirection::EH,
                   21,
                   "EUROP",
                   0,
                   21,
                   "IPREURP",
                   -1,
                   "",
                   4,
                   "EURG",
                   986,
                   "AARBS",
                   -1,
                   ""),
          getTXRef(
              GlobalDirection::EH, 22, "EUME", 0, 22, "IPREUME", -1, "", 4, "EURG", -1, "", -1, ""),
          getTXRef(GlobalDirection::AT,
                   24,
                   "V32N2",
                   1,
                   24,
                   "V32N2R",
                   60,
                   "IPRG",
                   24,
                   "V32TARG",
                   24,
                   "AV32N2",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   25,
                   "V32N3",
                   1,
                   25,
                   "V32N3R",
                   -1,
                   "",
                   25,
                   "V32TPRG",
                   25,
                   "PV32N3",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   27,
                   "SAAR2",
                   0,
                   27,
                   "IPRSAA2",
                   60,
                   "IPRG",
                   27,
                   "SAARG",
                   969,
                   "WARBSAT",
                   961,
                   "AARBSAT"),
          getTXRef(GlobalDirection::AT,
                   29,
                   "SAAR3",
                   0,
                   29,
                   "IPRSAA3",
                   -1,
                   "",
                   27,
                   "SAARG",
                   29,
                   "PARBSAT",
                   928,
                   "WARB3AT"),
          getTXRef(GlobalDirection::AT,
                   35,
                   "V32W2",
                   1,
                   35,
                   "V32W2R",
                   60,
                   "IPRG",
                   35,
                   "V32SARG",
                   141,
                   "WV32W2",
                   35,
                   "AV32W2"),
          getTXRef(GlobalDirection::EH,
                   46,
                   "V32EI",
                   1,
                   46,
                   "V32EIR",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   46,
                   "AV32EI",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   56,
                   "V32ED",
                   1,
                   56,
                   "V32EDR",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   57,
                   "V32EF",
                   1,
                   57,
                   "V32EFR",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   57,
                   "AV32EF",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   69,
                   "V32EM",
                   1,
                   69,
                   "V32EMR",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   69,
                   "AV32EM",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   75,
                   "CAT35A",
                   1,
                   75,
                   "CAT35AR",
                   -1,
                   "",
                   1,
                   "TARG",
                   809,
                   "CAT35AA",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   77,
                   "ATAA",
                   1,
                   77,
                   "ATAAR",
                   -1,
                   "",
                   1,
                   "TARG",
                   77,
                   "ATAARBS",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   79,
                   "V32E3",
                   1,
                   79,
                   "V32E3R",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   142,
                   "PV32E3",
                   79,
                   "AV32E3"),
          getTXRef(GlobalDirection::AP,
                   82,
                   "V3223",
                   1,
                   82,
                   "V3223R",
                   -1,
                   "",
                   46,
                   "V32EURG",
                   181,
                   "PV3223",
                   82,
                   "AV3223"),
          getTXRef(GlobalDirection::AT,
                   83,
                   "ATDOALL",
                   1,
                   83,
                   "ATDOALR",
                   -1,
                   "",
                   1,
                   "TARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   101,
                   "AATCP",
                   0,
                   101,
                   "IPRAI",
                   60,
                   "IPRG",
                   101,
                   "TARP",
                   985,
                   "ACAN",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   106,
                   "WHDPVBR",
                   1,
                   106,
                   "WDRPVBR",
                   -1,
                   "",
                   105,
                   "WDRGBR",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   109,
                   "WFZEDBR",
                   1,
                   109,
                   "WRZEDBR",
                   -1,
                   "",
                   105,
                   "WDRGBR",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   128,
                   "ATMAA",
                   1,
                   128,
                   "ATMAAR",
                   -1,
                   "",
                   1,
                   "TARG",
                   128,
                   "ATMAARB",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   129,
                   "PAMAA",
                   1,
                   129,
                   "PAMAAR",
                   60,
                   "IPRG",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   130,
                   "WHMAA",
                   1,
                   130,
                   "WHMAAR",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AP,
                   136,
                   "APFARPV",
                   1,
                   136,
                   "APFRRPV",
                   -1,
                   "",
                   4,
                   "EURG",
                   136,
                   "ABAPPV",
                   137,
                   "PBAPPV"),
          getTXRef(GlobalDirection::PA,
                   152,
                   "EXPA1",
                   1,
                   152,
                   "EXPA1R",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   162,
                   "WHAA",
                   1,
                   162,
                   "WHAAR",
                   -1,
                   "",
                   5,
                   "WHRG",
                   162,
                   "WUSAAA",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   189,
                   "SAASPV",
                   1,
                   189,
                   "SAASRPV",
                   -1,
                   "",
                   18,
                   "SARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   201,
                   "PATCP",
                   0,
                   201,
                   "IPRPI",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   204,
                   "TAFPV",
                   1,
                   204,
                   "TAPVR",
                   -1,
                   "",
                   1,
                   "TARG",
                   204,
                   "AUSAPV",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   208,
                   "MARAT",
                   1,
                   208,
                   "MARATR",
                   -1,
                   "",
                   1,
                   "TARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   209,
                   "MARPA",
                   1,
                   209,
                   "MARPAR",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   210,
                   "MARWH",
                   1,
                   210,
                   "MARWHR",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   260,
                   "PAAA1",
                   1,
                   260,
                   "PAAA1R",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   286,
                   "SAAR2PV",
                   1,
                   286,
                   "SAR2RPV",
                   -1,
                   "",
                   27,
                   "SAARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   302,
                   "TPFD",
                   0,
                   302,
                   "IPRTPFD",
                   -1,
                   "",
                   8,
                   "TGRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   303,
                   "WHFI",
                   0,
                   303,
                   "IPRWI",
                   364,
                   "IPRG1",
                   17,
                   "WDRG",
                   988,
                   "WARBS",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   304,
                   "EUROPD",
                   0,
                   304,
                   "IPREURD",
                   -1,
                   "",
                   4,
                   "EURG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AP,
                   307,
                   "APFARES",
                   0,
                   307,
                   "IPRAP",
                   -1,
                   "",
                   4,
                   "EURG",
                   968,
                   "PARBSAP",
                   963,
                   "AARBSAP"),
          getTXRef(GlobalDirection::WH,
                   329,
                   "WHFDPV",
                   1,
                   329,
                   "WHFDPVR",
                   -1,
                   "",
                   17,
                   "WDRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(
              GlobalDirection::RW, 334, "RW1", 0, 334, "RWR1", -1, "", 334, "RWRG", -1, "", -1, ""),
          getTXRef(
              GlobalDirection::CT, 340, "CT1", 0, 340, "CTR1", -1, "", 340, "CTRG", -1, "", -1, ""),
          getTXRef(GlobalDirection::AT,
                   350,
                   "SAAR21S",
                   1,
                   350,
                   "SAR2R1S",
                   -1,
                   "",
                   350,
                   "SAARG1S",
                   350,
                   "ARBSA1S",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   353,
                   "SAAS1S",
                   1,
                   353,
                   "SAASR1S",
                   -1,
                   "",
                   353,
                   "SAASG1S",
                   353,
                   "PARBS1S",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   376,
                   "WHFP1S",
                   1,
                   376,
                   "WHFPR1S",
                   347,
                   "IPRG1S",
                   376,
                   "WHRG1S",
                   376,
                   "WHARB1S",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   378,
                   "TPFPJ",
                   0,
                   378,
                   "IPRPJ",
                   60,
                   "IPRG",
                   378,
                   "TPRGJ",
                   378,
                   "PUSAJ",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   389,
                   "TAFP1S",
                   1,
                   389,
                   "TAFPR1S",
                   347,
                   "IPRG1S",
                   389,
                   "TARG1S",
                   389,
                   "ARBTA1S",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   393,
                   "TPFP1S",
                   1,
                   393,
                   "TPFPR1S",
                   347,
                   "IPRG1S",
                   393,
                   "TPRG1S",
                   393,
                   "ARBTP1S",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   396,
                   "TAFPZED",
                   1,
                   396,
                   "TAFRZED",
                   -1,
                   "",
                   1,
                   "TARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   421,
                   "WHFPC",
                   0,
                   421,
                   "IPRWC",
                   60,
                   "IPRG",
                   5,
                   "WHRG",
                   421,
                   "WCAN",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   443,
                   "ATAA1",
                   1,
                   443,
                   "ATAA1R",
                   -1,
                   "",
                   1,
                   "TARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   446,
                   "TPFPC",
                   0,
                   446,
                   "IPRPC",
                   -1,
                   "",
                   3,
                   "TPRG",
                   446,
                   "PUSC",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   476,
                   "PAAA",
                   1,
                   476,
                   "PAAAR",
                   -1,
                   "",
                   3,
                   "TPRG",
                   476,
                   "APAAA",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   481,
                   "ATAA4",
                   1,
                   481,
                   "ATAA4R",
                   -1,
                   "",
                   1,
                   "TARG",
                   481,
                   "ATARB4",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   504,
                   "TPFPZED",
                   1,
                   504,
                   "TPRPZED",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   528,
                   "EURODPV",
                   1,
                   528,
                   "EURDPVR",
                   -1,
                   "",
                   4,
                   "EURG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AP,
                   549,
                   "V22823",
                   1,
                   549,
                   "V22823R",
                   -1,
                   "",
                   549,
                   "V22823G",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PN,
                   589,
                   "SAASPN",
                   0,
                   589,
                   "IPRSAPN",
                   60,
                   "IPRG",
                   589,
                   "SARGPN",
                   589,
                   "PARBSPN",
                   591,
                   "WARBSPN"),
          getTXRef(GlobalDirection::WH,
                   592,
                   "WHAMEX",
                   1,
                   592,
                   "WHAMEXR",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   631,
                   "TCYAT",
                   1,
                   631,
                   "TCYATR",
                   60,
                   "IPRG",
                   1,
                   "TARG",
                   631,
                   "ATCYAT",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   660,
                   "WHFPZED",
                   1,
                   660,
                   "WHRPZED",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   662,
                   "WHFIZED",
                   1,
                   662,
                   "WHRIZED",
                   -1,
                   "",
                   17,
                   "WDRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   668,
                   "WHFDZED",
                   1,
                   668,
                   "WHRDZED",
                   -1,
                   "",
                   17,
                   "WDRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   757,
                   "SAAR36",
                   1,
                   757,
                   "SAAR36R",
                   -1,
                   "",
                   757,
                   "SAARG36",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   779,
                   "TAFP36",
                   1,
                   779,
                   "TAFP36R",
                   -1,
                   "",
                   779,
                   "TARG36",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   824,
                   "TCYPA",
                   1,
                   824,
                   "TCYPAR",
                   -1,
                   "",
                   3,
                   "TPRG",
                   824,
                   "PTCYPA",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   847,
                   "WHAA1",
                   1,
                   847,
                   "WHAA1R",
                   -1,
                   "",
                   5,
                   "WHRG",
                   847,
                   "WHARB1",
                   -1,
                   ""),
          getTXRef(GlobalDirection::AT,
                   891,
                   "EXAT1",
                   1,
                   891,
                   "EXAT1R",
                   -1,
                   "",
                   1,
                   "TARG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   911,
                   "CAT35W",
                   1,
                   911,
                   "CAT35WR",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::PA,
                   915,
                   "CAT35P",
                   1,
                   915,
                   "CAT35PR",
                   -1,
                   "",
                   3,
                   "TPRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   939,
                   "WHFIPV",
                   1,
                   939,
                   "WHFIPVR",
                   -1,
                   "",
                   17,
                   "WDRG",
                   939,
                   "WARBSPV",
                   -1,
                   ""),
          getTXRef(GlobalDirection::EH,
                   977,
                   "CAT35O",
                   1,
                   977,
                   "CAT35OR",
                   -1,
                   "",
                   4,
                   "EURG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::WH,
                   987,
                   "EXWH1",
                   1,
                   987,
                   "EXWH1R",
                   -1,
                   "",
                   5,
                   "WHRG",
                   -1,
                   "",
                   -1,
                   ""),
          getTXRef(GlobalDirection::RW,
                   522,
                   "VONE5",
                   1,
                   522,
                   "VONE5R",
                   -1,
                   "",
                   5,
                   "VONE",
                   522,
                   "VONE5",
                   -1,
                   "");
      return ret;
    }
    return DataHandleMock::getTariffXRef(vendor, carrier, crossRefType);
  }
  const Indicator getTariffInhibit(const VendorCode& vendor,
                                   const Indicator tariffCrossRefType,
                                   const CarrierCode& carrier,
                                   const TariffNumber& fareTariff,
                                   const TariffCode& ruleTariffCode)
  {
    if (vendor == "ATP" && carrier == "AA")
      return ' ';
    return DataHandleMock::getTariffInhibit(
        vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode);
  }
  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const VendorCode& vendor)
  {
    return getFaresByMarketCxr(market1, market2, cxr, vendor, DateTime::localTime());
  }
  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const VendorCode& vendor,
                                                          const DateTime& ticketDate)
  {
    std::vector<const FareInfo*>& ret = *_memHandle.create<std::vector<const FareInfo*> >();
    if (market1 == "GLA" && market2 == "ATL")
    {
      ret += getFare(market1, market2, "HMZ", 1), getFare(market1, market2, "IRLEISE", 1),
          getFare(market1, market2, "KMZ", 1), getFare(market1, market2, "NTHXMIL", 1),
          getFare(market1, market2, "OWBTSTOW", 128);
      return ret;
    }
    else if ((market1 == "GLA" && market2 == "DFW") || market1 == "LON" || market1 == "MAN")
    {
      ret += getFare(market1, market2, "HMZ", 1), getFare(market1, market2, "IRLEISE", 1),
          getFare(market1, market2, "KMZ", 1), getFare(market1, market2, "NTHXMIL", 1),
          getFare(market1, market2, "AZFLEX", 77), getFare(market1, market2, "BZFLEX", 77),
          getFare(market1, market2, "HZFLEX", 77), getFare(market1, market2, "JZFLEX", 77),
          getFare(market1, market2, "IZFLEX", 77), getFare(market1, market2, "YZFLEX", 128),
          getFare(market1, market2, "OWBTSTOW", 128), getFare(market1, market2, "VONE5", 522);
      return ret;
    }
    else if (market1 == "GLA" && market2 == "LAX")
    {
      ret += getFare(market1, market2, "HMZ", 1), getFare(market1, market2, "IRLEISE", 1),
          getFare(market1, market2, "KMZ", 1), getFare(market1, market2, "NTHXMIL", 1),
          getFare(market1, market2, "AZFLEX", 77), getFare(market1, market2, "BZFLEX", 77),
          getFare(market1, market2, "HZFLEX", 77), getFare(market1, market2, "JZFLEX", 77),
          getFare(market1, market2, "IZFLEX", 77);
      return ret;
    }
    else if (market1 == "GLA" && market2 == "TUL")
    {
      ret += getFare(market1, market2, "HMZ", 1), getFare(market1, market2, "KMZ", 1),
          getFare(market1, market2, "NTHXMIL", 1);
      return ret;
    }
    return DataHandleMock::getFaresByMarketCxr(market1, market2, cxr, vendor, ticketDate);
  }

  const std::vector<AddonFareInfo*>&
  getAddOnFare(const LocCode& interiorMarket, const CarrierCode& carrier, const DateTime& date)
  {
    std::vector<AddonFareInfo*>* addonFares = _memHandle.create<std::vector<AddonFareInfo*> >();

    if (interiorMarket == "MAN")
    {
      AddonFareInfo* addonFare = _memHandle.create<AddonFareInfo>();

      addonFare->interiorMarket() = interiorMarket;
      addonFare->gatewayMarket() = "LON";
      addonFare->carrier() = "AA";
      addonFare->vendor() = "ATP";
      addonFare->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
      addonFare->fareClass() = "VONE5";
      addonFare->addonTariff() = 522;
      addonFare->routing() = "4444";
      addonFare->arbZone() = 0;

      addonFares->push_back(addonFare);
    }

    return *addonFares;
  }
};
}
class VendorAtpcoMock : public VendorAtpco
{
public:
  bool matchAddonFares(AddonFareCortegeVec::iterator& firstOrigFare,
                       AddonFareCortegeVec::iterator& firstDestFare)
  {
    return _matchAddonFaresResult;
  }

  AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                ConstructionPoint cp,
                                TSEDateIntervalVec& zoneIntervals)
  {
    TSEDateInterval* ti;
    _cJob->dataHandle().get(ti);
    zoneIntervals.push_back(ti);
    return _validateZonesResult;
  }

  bool isGlobalDirValid(const CarrierCode&,
                        TariffNumber /*addonTariff*/,
                        GlobalDirection)
  {
    return _isGlobalDirValidResult;
  }

  AddonZoneStatus _validateZonesResult;
  bool _matchAddonFaresResult;
  bool _isGlobalDirValidResult;
};

class VendorSitaMock : public VendorSita
{
public:
  AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                ConstructionPoint cp,
                                TSEDateIntervalVec& zoneIntervals)
  {
    return _validateZonesResult;
  }
  AddonZoneStatus _validateZonesResult;
};
class VendorSmfMock : public VendorSmf
{
public:
  AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                ConstructionPoint cp,
                                TSEDateIntervalVec& zoneIntervals)
  {
    return _validateZonesResult;
  }

  bool isGlobalDirValid(const CarrierCode&,
                        TariffNumber /*addonTariff*/,
                        GlobalDirection)
  {
    return _isGlobalDirValidResult;
  }

  AddonZoneStatus _validateZonesResult;
  bool _isGlobalDirValidResult;
};

class ConstructionVendorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ConstructionVendorTest);

  CPPUNIT_TEST(testAddAddonFareAtpco);
  CPPUNIT_TEST(testAddAddonFareSita);
  CPPUNIT_TEST(testAddAddonFareSmf);
  CPPUNIT_TEST(testAddAddonFareAtpcoFail);
  CPPUNIT_TEST(testAddAddonFareSitaFail);
  CPPUNIT_TEST(testAddAddonFareSmfFail);
  CPPUNIT_TEST(testAddAddonFareMultipleFaresDestination);
  CPPUNIT_TEST(testAddAddonFareMultipleFaresOrigin);
  CPPUNIT_TEST(testSortAddonFares);
  CPPUNIT_TEST(testVendorProcess);
  CPPUNIT_TEST(testProcessRwAddonFares);
  CPPUNIT_TEST(testIsApplicableForRw);
  CPPUNIT_TEST(testBuildDEGatewaysRw);
  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testReconstruction);
  CPPUNIT_TEST(testReconstructionForEmptyFCVectors);
  CPPUNIT_TEST(testAssignFaresToGateways);

  CPPUNIT_TEST(testGetNewVendorAtpco);
  CPPUNIT_TEST(testGetNewVendorSita);
  CPPUNIT_TEST(testGetNewVendorSmf);
  CPPUNIT_TEST(testGetNewGatewayPairAtpco);
  CPPUNIT_TEST(testGetNewGatewayPairSita);
  CPPUNIT_TEST(testGetNewGatewayPairSmf);
  //-- tests to add
  /*
    CPPUNIT_TEST( testVendorReconstruction );
    CPPUNIT_TEST( testVendorInitialize );
    CPPUNIT_TEST( testVendorAddonFare );
    CPPUNIT_TEST( testVendorMarkUpAddonFares );
    CPPUNIT_TEST( testVendorBuildSEGateways );
    CPPUNIT_TEST( testVendorBuildDEGateways );
    CPPUNIT_TEST( testVendorAssignFaresToGateways );
    CPPUNIT_TEST( testVendorsortAddonFares );
  */
  //--end new tests

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    ConstructionVendor::_taskId = TseThreadingConst::SYNCHRONOUS_TASK;
  }
  void tearDown() { _memHandle.clear(); }

  void setUpRw()
  {
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<PricingOptions>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(true);
    Itin* itin = _memHandle.create<Itin>();
    itin->tripCharacteristics().set(Itin::RW_SFC, true);
    _trx->itin().push_back(itin);

    // ConstructionJob

    _travelDate = DateTime::localTime();
    _ticketingDate = DateTime::localTime();

    _vendorCode = "ATP";
    _carrierCode = "AA";

    _origCode = "MAN";
    _destCode = "MAN";

    _boardMultiCity = "MAN";
    _offMultiCity = "MAN";

    _fareClass = "VONE5";
    _globalDirection = GlobalDirection::RW;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  _globalDirection,
                                                  false,
                                                  &_specFareCache));
#else
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  false));
#endif

    _cJob->carrier() = _carrierCode;
    _cJob->setVendorCode(ATPCO_VENDOR_CODE);

    setVendor();
  }

  void setUpConstruction(VendorCode vendor)
  {
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<PricingOptions>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(false);

    _travelDate = DateTime::localTime();
    _ticketingDate = DateTime::localTime();
    _dateInterval.createDate() = _dateInterval.effDate() = _travelDate;
    _dateInterval.expireDate() = _dateInterval.discDate() = _travelDate + boost::gregorian::days(1);

    _vendorCode = vendor;
    _carrierCode = "AA";

    _origCode = "MAN";
    _destCode = "TUL";

    _boardMultiCity = "MAN";
    _offMultiCity = "TUL";

    _fareClass = "";
    _globalDirection = GlobalDirection::AT;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  _globalDirection,
                                                  false,
                                                  &_specFareCache));
#else
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  false));
#endif

    _cJob->setVendorCode(_vendorCode);
    _cJob->carrier() = _carrierCode;

    setVendor();
  }

  void setVendor()
  {
    if (_cJob->isAtpco())
    {
      _constructionVendor = _cJob->constructionVendor() = _memHandle.create<VendorAtpcoMock>();
      dynamic_cast<VendorAtpcoMock*>(_constructionVendor)->_validateZonesResult = AZ_PASS;
      dynamic_cast<VendorAtpcoMock*>(_constructionVendor)->_matchAddonFaresResult = true;
      dynamic_cast<VendorAtpcoMock*>(_constructionVendor)->_isGlobalDirValidResult = true;
    }
    else if (_cJob->isSita())
    {
      _constructionVendor = _cJob->constructionVendor() = _memHandle.create<VendorSitaMock>();
      dynamic_cast<VendorSitaMock*>(_constructionVendor)->_validateZonesResult = AZ_PASS;
    }

    else if (_cJob->isSMF())
    {
      _constructionVendor = _cJob->constructionVendor() = _memHandle.create<VendorSmfMock>();
      dynamic_cast<VendorSmfMock*>(_constructionVendor)->_validateZonesResult = AZ_PASS;
      dynamic_cast<VendorSmfMock*>(_constructionVendor)->_isGlobalDirValidResult = true;
    }

    _constructionVendor->initialize(_cJob);
  }

  void testAddAddonFareAtpco()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "GLA", 77, "Y*****", 999);
    CPPUNIT_ASSERT_EQUAL(AZ_PASS, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testAddAddonFareSita()
  {
    setUpConstruction(SITA_VENDOR_CODE);
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, SITA_VENDOR_CODE, "GLA", 77, "Y*****", 999);
    CPPUNIT_ASSERT_EQUAL(AZ_PASS, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testAddAddonFareSmf()
  {
    setUpConstruction("SMF");
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, "SMF", "GLA", 77, "Y*****", 999);
    CPPUNIT_ASSERT_EQUAL(AZ_PASS, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testAddAddonFareAtpcoFail()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "GLA", 77, "Y*****", 999);
    static_cast<VendorAtpcoMock*>(_constructionVendor)->_validateZonesResult = AZ_FAIL;
    CPPUNIT_ASSERT_EQUAL(AZ_FAIL, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testAddAddonFareSitaFail()
  {
    setUpConstruction(SITA_VENDOR_CODE);
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, SITA_VENDOR_CODE, "GLA", 77, "Y*****", 999);
    static_cast<VendorSitaMock*>(_constructionVendor)->_validateZonesResult = AZ_FAIL;
    CPPUNIT_ASSERT_EQUAL(AZ_FAIL, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testAddAddonFareSmfFail()
  {
    setUpConstruction("SMF");
    AddonFareInfo* afi = populateAddonFare(CP_ORIGIN, "SMF", "GLA", 77, "Y*****", 999);
    static_cast<VendorSmfMock*>(_constructionVendor)->_validateZonesResult = AZ_FAIL;
    CPPUNIT_ASSERT_EQUAL(AZ_FAIL, _constructionVendor->addAddonFare(CP_ORIGIN, *afi));
  }

  void testSortAddonFares()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    static_cast<VendorAtpcoMock*>(_constructionVendor)->sortAddonFares();
    CPPUNIT_ASSERT_EQUAL(
        FareClassCode("YHAPABCD"),
        _constructionVendor->addonFares(CP_ORIGIN).back()->addonFare()->fareClass());
    CPPUNIT_ASSERT_EQUAL(
        LocCode("LON"),
        _constructionVendor->addonFares(CP_ORIGIN).back()->addonFare()->gatewayMarket());
    CPPUNIT_ASSERT_EQUAL(998,
                         _constructionVendor->addonFares(CP_ORIGIN).back()->addonFare()->arbZone());

    CPPUNIT_ASSERT_EQUAL(
        FareClassCode("H*****"),
        _constructionVendor->addonFares(CP_DESTINATION).back()->addonFare()->fareClass());
    CPPUNIT_ASSERT_EQUAL(
        LocCode("LAX"),
        _constructionVendor->addonFares(CP_DESTINATION).back()->addonFare()->gatewayMarket());
    CPPUNIT_ASSERT_EQUAL(
        29, _constructionVendor->addonFares(CP_DESTINATION).back()->addonFare()->arbZone());
  }

  void testAddAddonFareMultipleFaresOrigin()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    CPPUNIT_ASSERT_EQUAL(size_t(5), _constructionVendor->addonFares(CP_ORIGIN).size());
  }

  void testAddAddonFareMultipleFaresDestination()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    CPPUNIT_ASSERT_EQUAL(size_t(4), _constructionVendor->addonFares(CP_DESTINATION).size());
  }

  void testConstruction()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    ConstructedCacheDataWrapper dw;
    CPPUNIT_ASSERT(_constructionVendor->construction(dw));
  }

  void testReconstruction()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    ConstructedCacheDataWrapper dw;
    CacheGatewayPairVec gw;
    std::shared_ptr<GatewayPair> dummyGatewayPair = _constructionVendor->getNewGwPair();
    dummyGatewayPair->initialize(
        _cJob, _constructionVendor, "GLA", "TUL", false, false, 0, 2, 0, 0);
    gw.push_back(dummyGatewayPair);
    CPPUNIT_ASSERT(_constructionVendor->reconstruction(dw, gw));
  }

  void testReconstructionForEmptyFCVectors()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    ConstructedCacheDataWrapper dw;
    CacheGatewayPairVec gw;
    std::shared_ptr<GatewayPair> dummyGatewayPair = _constructionVendor->getNewGwPair();
    dummyGatewayPair->initialize(
        _cJob, _constructionVendor, "GLA", "TUL", false, false, 0, 2, 0, 0);
    gw.push_back(dummyGatewayPair);
    CPPUNIT_ASSERT(_constructionVendor->reconstruction(dw, gw));
  }

  void testAssignFaresToGateways()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    populateOrigAddonFare();
    populateDestAddonFare();
    CacheGatewayPairVec gw;
    std::shared_ptr<GatewayPair> dummyGatewayPair = _constructionVendor->getNewGwPair();
    dummyGatewayPair->initialize(_cJob, _constructionVendor, "GLA", "DFW", true, true, 0, 2, 0, 1);
    gw.push_back(dummyGatewayPair);
    CPPUNIT_ASSERT(_constructionVendor->assignFaresToGateways(gw));
  }

  void testGetNewVendorAtpco()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    _constructionVendor = NULL;
    _constructionVendor = VendorAtpco::getNewVendor(*_cJob);
    CPPUNIT_ASSERT(_constructionVendor);
    CPPUNIT_ASSERT(dynamic_cast<VendorAtpco*>(_constructionVendor));
    CPPUNIT_ASSERT_EQUAL(_cJob->vendorCode(), _constructionVendor->vendor());
  }
  void testGetNewVendorSita()
  {
    setUpConstruction(SITA_VENDOR_CODE);
    _constructionVendor = NULL;
    _constructionVendor = VendorAtpco::getNewVendor(*_cJob);
    CPPUNIT_ASSERT(_constructionVendor);
    CPPUNIT_ASSERT(dynamic_cast<VendorAtpco*>(_constructionVendor));
    CPPUNIT_ASSERT_EQUAL(_cJob->vendorCode(), _constructionVendor->vendor());
  }
  void testGetNewVendorSmf()
  {
    setUpConstruction("SMF");
    _constructionVendor = NULL;
    _constructionVendor = VendorAtpco::getNewVendor(*_cJob);
    CPPUNIT_ASSERT(_constructionVendor);
    CPPUNIT_ASSERT(dynamic_cast<VendorAtpco*>(_constructionVendor));
    CPPUNIT_ASSERT_EQUAL(_cJob->vendorCode(), _constructionVendor->vendor());
  }
  void testGetNewGatewayPairAtpco()
  {
    setUpConstruction(ATPCO_VENDOR_CODE);
    std::shared_ptr<GatewayPair> gwPair = _constructionVendor->getNewGwPair();
    CPPUNIT_ASSERT(dynamic_cast<AtpcoGatewayPair*>(gwPair.get()));
  }
  void testGetNewGatewayPairSita()
  {
    setUpConstruction(SITA_VENDOR_CODE);
    std::shared_ptr<GatewayPair> gwPair = _constructionVendor->getNewGwPair();
    CPPUNIT_ASSERT(dynamic_cast<SitaGatewayPair*>(gwPair.get()));
  }
  void testGetNewGatewayPairSmf()
  {
    setUpConstruction("SMF");
    std::shared_ptr<GatewayPair> gwPair = _constructionVendor->getNewGwPair();
    CPPUNIT_ASSERT(dynamic_cast<SmfGatewayPair*>(gwPair.get()));
  }

  void testProcessRwAddonFares()
  {
    ConstructedCacheDataWrapper localDataWrapper;

    setUpRw();
    AddonConstruction::runConstructionProcess(*_cJob, localDataWrapper);

    CPPUNIT_ASSERT(localDataWrapper.ccFares().size() == 1);
  }

  void testIsApplicableForRw()
  {
    setUpRw();

    AddonFareInfo* addonFare = _memHandle.create<AddonFareInfo>();

    addonFare->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    addonFare->routing() = "4444";
    addonFare->arbZone() = 0;

    CPPUNIT_ASSERT(AddonConstruction::isApplicableForRw(*_cJob, *addonFare) == AZ_PASS);

    addonFare->owrt() = ONE_WAY_MAY_BE_DOUBLED;

    CPPUNIT_ASSERT(AddonConstruction::isApplicableForRw(*_cJob, *addonFare) == AZ_UNACCEPTABLE);

    addonFare->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    addonFare->footNote2() = 'T';

    CPPUNIT_ASSERT(AddonConstruction::isApplicableForRw(*_cJob, *addonFare) == AZ_UNACCEPTABLE);

    addonFare->footNote2() = "";
    addonFare->routing() = "2344";

    CPPUNIT_ASSERT(AddonConstruction::isApplicableForRw(*_cJob, *addonFare) == AZ_UNACCEPTABLE);

    addonFare->routing() = "4444";
    addonFare->arbZone() = 5;

    CPPUNIT_ASSERT(AddonConstruction::isApplicableForRw(*_cJob, *addonFare) == AZ_FAIL);
  }

  void testBuildDEGatewaysRw()
  {
    setUpRw();
    const std::vector<AddonFareInfo*>& addOns =
        _trx->dataHandle().getAddOnFare(_origCode, _carrierCode);
    std::vector<AddonFareInfo*>::const_iterator iter = addOns.begin();

    for (; iter != addOns.end(); iter++)
      _cJob->constructionVendor()->addAddonFareRw(**iter);

    _cJob->constructionVendor()->addonFares(CP_ORIGIN).front()->gatewayFareCount() = 1;

    if (!_cJob->constructionVendor()->buildDEGatewaysRw())
      CPPUNIT_ASSERT(false);

    if (_cJob->constructionVendor()->gateways().size() != 1)
      CPPUNIT_ASSERT(false);

    if (_cJob->constructionVendor()->gateways().front()->gateway1() != "LON")
      CPPUNIT_ASSERT(false);

    if (_cJob->constructionVendor()->gateways().front()->gateway2() != "LON")
      CPPUNIT_ASSERT(false);

    CPPUNIT_ASSERT(true);
  }

  void testVendorProcess()
  {
    // TRX & stuff

    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<PricingOptions>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(false);

    // ConstructionJob

    _travelDate = DateTime::localTime();
    _ticketingDate = DateTime::localTime();

    _vendorCode = "ATP";
    _carrierCode = "AA";

    _origCode = "MAN";
    _destCode = "TUL";

    _boardMultiCity = "MAN";
    _offMultiCity = "TUL";

    _fareClass = "";
    _globalDirection = GlobalDirection::AT;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  _globalDirection,
                                                  false,
                                                  &_specFareCache));
#else
    _cJob = _memHandle.insert(new ConstructionJob(*_trx,
                                                  _travelDate,
                                                  _ticketingDate,
                                                  _origCode,
                                                  _boardMultiCity,
                                                  _destCode,
                                                  _offMultiCity,
                                                  false));
#endif

    _cJob->carrier() = _carrierCode;
    _cJob->setVendorCode(ATPCO_VENDOR_CODE);

    // create/initalize ConstructionVendor instance
    try
    {
      _acv = _memHandle.create<VendorAtpcoMock>();
      dynamic_cast<VendorAtpcoMock*>(_acv)->_validateZonesResult = AZ_PASS;
      dynamic_cast<VendorAtpcoMock*>(_acv)->_matchAddonFaresResult = true;
      dynamic_cast<VendorAtpcoMock*>(_acv)->_isGlobalDirValidResult = true;

      _acv->initialize(_cJob);

      _acv->trfXrefMap().populate();
    }
    catch (...)
    {
      CPPUNIT_FAIL("Fail create/initialize an instance of ConstructionVendor class.");
      return;
    };

    // populate ConstructionVendor with test AddonFareInfo objects

    _constructionVendor = _acv;
    populateOrigAddonFare();
    populateDestAddonFare();

    // process
    ConstructedCacheDataWrapper localDataWrapper;

    //   AddonConstruction::runConstructionProcess( *_cJob, localDataWrapper );

    CPPUNIT_ASSERT(_acv->construction(localDataWrapper));

    // check cortege markUp

    checkMarkUp();

    // check gateway vector

    CacheGatewayPairVec::const_iterator gww = _acv->gateways().begin();

    checkSEGateways(gww);

    checkDEGateways(gww);

    // check for extra gateways

    CPPUNIT_ASSERT_EQUAL(size_t(11), _acv->gateways().size());

    // check Tariff Combinability
    // !!!  has to be removed: it creates DB conection and is not ConstructionVendor test
    checkTariffCombinability();

    // check Addon Fare Class Combinability
    // !!! has to be removed: it creates DB conection and is not ConstructionVendor test
    // checkAddonFareClassCombinability();
  }

  void addAddonFare(ConstructionPoint cp,
                    const VendorCode vendor,
                    const LocCode gatewayMarket,
                    TariffNumber addonTariff,
                    FareClassCode fareClass,
                    AddonZone arbZone)
  {
    AddonFareInfo* afi =
        populateAddonFare(cp, vendor, gatewayMarket, addonTariff, fareClass, arbZone);
    _constructionVendor->addAddonFare(cp, *afi);
  }

  AddonFareInfo* populateAddonFare(ConstructionPoint cp,
                                   const VendorCode vendor,
                                   const LocCode gatewayMarket,
                                   TariffNumber addonTariff,
                                   FareClassCode fareClass,
                                   AddonZone arbZone)
  {
    DateTime effDate = _travelDate + Hours(-240);
    DateTime expDate = _travelDate + Hours(240);

    AddonFareInfo* afi;
    _trx->dataHandle().get(afi);
    afi->vendor() = vendor;
    afi->interiorMarket() = (cp == CP_ORIGIN ? _origCode : _destCode);
    afi->gatewayMarket() = gatewayMarket;
    afi->addonTariff() = addonTariff;
    afi->fareClass() = fareClass;
    afi->effDate() = effDate;
    afi->expireDate() = expDate;
    afi->discDate() = expDate;
    afi->arbZone() = arbZone;

    return afi;
  }

  void populateOrigAddonFare()
  {
    // AddonFareCortegeVec& fcv = _constructionVendor -> addonFares( CP_ORIGIN );

    // to PAR
    addAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "GLA", 77, "Y*****", 999);
    // CPPUNIT_ASSERT( fcv.back()->addonFareClass() == AddonFareCortege::ALPHA_FIVE_STAR );

    addAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "GLA", 996, "YHAPABCD", 105);

    // to LON
    addAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "LON", 128, "YHAPABCD", 998);

    addAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "LON", 996, "YHAPABCD", 105);

    addAddonFare(CP_ORIGIN, ATPCO_VENDOR_CODE, "LON", 999, "YHAPABCD", 998);
  }

  void populateDestAddonFare()
  {
    // to DFW
    addAddonFare(CP_DESTINATION, ATPCO_VENDOR_CODE, "DFW", 77, "******", 999);

    // to ATL
    addAddonFare(CP_DESTINATION, ATPCO_VENDOR_CODE, "ATL", 128, "BWPX3M", 29);

    addAddonFare(CP_DESTINATION, ATPCO_VENDOR_CODE, "ATL", 963, "BHAPOW", 195);

    // to LAX
    addAddonFare(CP_DESTINATION, ATPCO_VENDOR_CODE, "LAX", 999, "H*****", 29);
  }

  void checkMarkUp()
  {
    // check cortege markUp for origin

    AddonFareCortegeVec& fcvo = _constructionVendor->addonFares(CP_ORIGIN);
    CPPUNIT_ASSERT(fcvo.size() == 5);

    CPPUNIT_ASSERT(fcvo[0]->gatewayFareCount() == 2);
    CPPUNIT_ASSERT(fcvo[1]->gatewayFareCount() == 2);
    CPPUNIT_ASSERT(fcvo[2]->gatewayFareCount() == 3);
    CPPUNIT_ASSERT(fcvo[3]->gatewayFareCount() == 3);
    CPPUNIT_ASSERT(fcvo[4]->gatewayFareCount() == 3);

    // check cortege markUp for destination

    AddonFareCortegeVec& fcvd = _constructionVendor->addonFares(CP_DESTINATION);
    CPPUNIT_ASSERT(fcvd.size() == 4);

    CPPUNIT_ASSERT(fcvd[0]->gatewayFareCount() == 2);
    CPPUNIT_ASSERT(fcvd[1]->gatewayFareCount() == 2);
    CPPUNIT_ASSERT(fcvd[2]->gatewayFareCount() == 1);
    CPPUNIT_ASSERT(fcvd[3]->gatewayFareCount() == 1);
  }

  void checkSEGateways(CacheGatewayPairVec::const_iterator& gww)
  {

    // check SEGateways for destination

    CPPUNIT_ASSERT(gww != _constructionVendor->gateways().end());

    CPPUNIT_ASSERT((*gww)->gateway1() == "GLA");
    CPPUNIT_ASSERT((*gww)->gateway2() == _destCode);

    CPPUNIT_ASSERT((*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT(!(*gww)->isGw2ConstructPoint());

    gww++;
    CPPUNIT_ASSERT((*gww)->gateway1() == "LON");
    CPPUNIT_ASSERT((*gww)->gateway2() == _destCode);

    CPPUNIT_ASSERT((*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT(!(*gww)->isGw2ConstructPoint());

    // check SEGateways for origin

    gww++;
    CPPUNIT_ASSERT((*gww)->gateway1() == _origCode);
    CPPUNIT_ASSERT((*gww)->gateway2() == "ATL");

    CPPUNIT_ASSERT(!(*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());

    gww++;
    CPPUNIT_ASSERT((*gww)->gateway1() == _origCode);
    CPPUNIT_ASSERT((*gww)->gateway2() == "DFW");

    CPPUNIT_ASSERT(!(*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());

    gww++;
    CPPUNIT_ASSERT((*gww)->gateway1() == _origCode);
    CPPUNIT_ASSERT((*gww)->gateway2() == "LAX");

    CPPUNIT_ASSERT(!(*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());
  }

  void checkDEGateways(CacheGatewayPairVec::const_iterator& gww)
  {
    // check DEGateways

    // from GLA

    gww++;
    CPPUNIT_ASSERT(gww != _constructionVendor->gateways().end());

    CPPUNIT_ASSERT_EQUAL(LocCode("GLA"), (*gww)->gateway1());
    CPPUNIT_ASSERT_EQUAL(LocCode("ATL"), (*gww)->gateway2());

    CPPUNIT_ASSERT((*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());

    // from LON

    gww++;
    CPPUNIT_ASSERT_EQUAL(LocCode("GLA"), (*gww)->gateway1());
    CPPUNIT_ASSERT_EQUAL(LocCode("DFW"), (*gww)->gateway2());

    CPPUNIT_ASSERT((*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());

    gww++;
    CPPUNIT_ASSERT_EQUAL(LocCode("GLA"), (*gww)->gateway1());
    CPPUNIT_ASSERT_EQUAL(LocCode("LAX"), (*gww)->gateway2());

    CPPUNIT_ASSERT((*gww)->isGw1ConstructPoint());
    CPPUNIT_ASSERT((*gww)->isGw2ConstructPoint());
  }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  void checkTariffCombinability()
  {
    TrfXrefMap& trfXrefMap = _acv->trfXrefMap();

    CPPUNIT_ASSERT(FM_GOOD_MATCH == trfXrefMap.matchFareAndAddonTariff(307, 968, false, _dateInterval));
    CPPUNIT_ASSERT(FM_GOOD_MATCH != trfXrefMap.matchFareAndAddonTariff(308, 968, false, _dateInterval));

    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(996, 996));

    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(963, 968));
    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(968, 963));

    CPPUNIT_ASSERT(!trfXrefMap.matchAddonTariffs(18, 968));
    CPPUNIT_ASSERT(!trfXrefMap.matchAddonTariffs(968, 18));
  }

  void checkAddonFareClassCombinability()
  {
    AddonFareCortegeVec& fdvo = _acv->addonFares(CP_DESTINATION);

    AtpcoConstructedFare cf;
    FareInfo fi;

    cf.specifiedFare() = &fi;

    CombFareClassMap* combFareClassMap = _acv->getCombFareClassMap();

    setUpConstruction(ATPCO_VENDOR_CODE);
    std::shared_ptr<GatewayPair> gw(_constructionVendor->getNewGwPair());
    gw->initialize(_cJob, _acv, "GLA", "TUL", false, false, 0, 2, 0, 0);
    AtpcoGatewayPair* agw(dynamic_cast<AtpcoGatewayPair*>(gw.get()));

    // non-generic add-on fare -> exact match

    fi.fareTariff() = 0;
    fi.fareClass() = "BWPX3M";
    fi.owrt() = '1';
    CPPUNIT_ASSERT(nullptr == combFareClassMap->matchSpecifiedFare(fi));

    fi.fareTariff() = 18;
    fi.fareClass() = "BHAPOW";
    fi.owrt() = '1';
    const AddonFareClasses* combs1 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs1);
    CPPUNIT_ASSERT(FM_GOOD_MATCH == agw->matchAddonFareClass(*fdvo[1], 'N', *combs1, _dateInterval));

    // Six Star generic add-on fare -> positive logic match
    fi.fareTariff() = 18;
    fi.fareClass() = "BFWPX3";
    fi.owrt() = '2';
    const AddonFareClasses* combs2 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs2);
    CPPUNIT_ASSERT(FM_GOOD_MATCH != agw->matchAddonFareClass(*fdvo[2], 'N', *combs2, _dateInterval));

    // REGULAR - FM_COMB_FARE_CLASS
    fi.fareTariff() = 18;
    fi.fareClass() = "BFWPX3M";
    fi.owrt() = '2';
    const AddonFareClasses* combs3 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs3);
    CPPUNIT_ASSERT(FM_GOOD_MATCH != agw->matchAddonFareClass(*fdvo[2], 'U', *combs3, _dateInterval));

    fi.fareTariff() = 18;
    fi.fareClass() = "B2";
    fi.owrt() = '1';
    const AddonFareClasses* combs4 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs4);
    CPPUNIT_ASSERT(FM_GOOD_MATCH == agw->matchAddonFareClass(*fdvo[2], 'N', *combs4, _dateInterval));

    // Alpha Five Star generic add-on fare -> positive logic match

    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    fi.owrt() = '2';
    const AddonFareClasses* combs5 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs5);
    CPPUNIT_ASSERT(FM_GOOD_MATCH == agw->matchAddonFareClass(*fdvo[3], 'N', *combs5, _dateInterval));

    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    fi.owrt() = '2';
    const AddonFareClasses* combs6 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs6);
    CPPUNIT_ASSERT(FM_GOOD_MATCH == agw->matchAddonFareClass(*fdvo[3], 'U', *combs6, _dateInterval));

    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    fi.owrt() = '1';
    const AddonFareClasses* combs7 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs7);
    CPPUNIT_ASSERT(FM_GOOD_MATCH != agw->matchAddonFareClass(*fdvo[3], 'N', *combs7, _dateInterval));

    fi.fareTariff() = 5;
    fi.fareClass() = "FOXRA";
    fi.owrt() = '2';
    const AddonFareClasses* combs8 = combFareClassMap->matchSpecifiedFare(fi);
    CPPUNIT_ASSERT(nullptr != combs8);
    CPPUNIT_ASSERT(FM_COMB_FARE_CLASS == agw->matchAddonFareClass(*fdvo[3], 'N', *combs8, _dateInterval));
  }

#else

  void checkTariffCombinability()
  {
    TrfXrefMap& trfXrefMap = _acv->trfXrefMap();

    InhibitedDateIntervalVec idi;

    CPPUNIT_ASSERT(FM_GOOD_MATCH == trfXrefMap.matchFareAndAddonTariff(307, 968, idi));
    CPPUNIT_ASSERT(FM_GOOD_MATCH != trfXrefMap.matchFareAndAddonTariff(308, 968, idi));

    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(996, 996));

    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(963, 968));
    CPPUNIT_ASSERT(trfXrefMap.matchAddonTariffs(968, 963));

    CPPUNIT_ASSERT(!trfXrefMap.matchAddonTariffs(18, 968));
    CPPUNIT_ASSERT(!trfXrefMap.matchAddonTariffs(968, 18));
  }

  void checkAddonFareClassCombinability()
  {
    AddonFareCortegeVec& fdvo = _acv->addonFares(CP_DESTINATION);

    AtpcoConstructedFare cf;
    FareInfo fi;

    cf.specifiedFare() = &fi;

    CombFareClassMap* combFareClassMap = _acv->getCombFareClassMap();

    // non-generic add-on fare -> exact match

    AddonCombFareClassInfoVec fClassCombRecords;

    fi.fareTariff() = 0;
    fi.fareClass() = "BWPX3M";
    CPPUNIT_ASSERT(FM_COMB_FARE_CLASS ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[1], 'N', '1', fClassCombRecords));

    fi.fareTariff() = 18;
    fi.fareClass() = "BHAPOW";
    CPPUNIT_ASSERT(FM_COMB_FARE_EXACT_MATCH ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[1], 'N', '1', fClassCombRecords));

    // Six Star generic add-on fare -> positive logic match
    fClassCombRecords.clear();
    fi.fareTariff() = 18;
    fi.fareClass() = "BFWPX3";
    CPPUNIT_ASSERT(FM_GOOD_MATCH !=
                   combFareClassMap->matchFareClasses(cf, *fdvo[2], 'N', '2', fClassCombRecords));

    // REGULAR - FM_COMB_FARE_CLASS
    fi.fareTariff() = 18;
    fi.fareClass() = "BFWPX3M";
    CPPUNIT_ASSERT(FM_GOOD_MATCH !=
                   combFareClassMap->matchFareClasses(cf, *fdvo[2], 'U', '2', fClassCombRecords));

    fClassCombRecords.clear();
    fi.fareTariff() = 18;
    fi.fareClass() = "B2";
    CPPUNIT_ASSERT(FM_GOOD_MATCH ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[2], 'N', '1', fClassCombRecords));

    // Alpha Five Star generic add-on fare -> positive logic match

    fClassCombRecords.clear();
    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    CPPUNIT_ASSERT(FM_GOOD_MATCH ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[3], 'N', '2', fClassCombRecords));

    fClassCombRecords.clear();
    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    CPPUNIT_ASSERT(FM_GOOD_MATCH ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[3], 'U', '2', fClassCombRecords));

    fClassCombRecords.clear();
    fi.fareTariff() = 18;
    fi.fareClass() = "FOXR";
    CPPUNIT_ASSERT(FM_GOOD_MATCH !=
                   combFareClassMap->matchFareClasses(cf, *fdvo[3], 'N', '1', fClassCombRecords));

    fi.fareTariff() = 5;
    fi.fareClass() = "FOXRA";
    CPPUNIT_ASSERT(FM_COMB_FARE_CLASS ==
                   combFareClassMap->matchFareClasses(cf, *fdvo[3], 'N', '2', fClassCombRecords));
  }

#endif

protected:
  PricingTrx* _trx;
  ConstructionJob* _cJob;
  ConstructionVendor* _constructionVendor;

  DateTime _travelDate;
  DateTime _ticketingDate;
  TSEDateInterval _dateInterval;

  VendorCode _vendorCode;
  CarrierCode _carrierCode;

  LocCode _origCode;
  LocCode _destCode;

  LocCode _boardMultiCity;
  LocCode _offMultiCity;

  FareClassCode _fareClass;
  GlobalDirection _globalDirection;
  VendorAtpco* _acv;
  TestMemHandle _memHandle;
  SpecifiedFareCache _specFareCache;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConstructionVendorTest);
}
