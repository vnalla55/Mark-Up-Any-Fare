// ----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <sstream>

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Taxes/LegacyTaxes/TaxAY.h"
#include "Taxes/LegacyTaxes/TaxBE.h"
#include "Taxes/LegacyTaxes/TaxBP.h"
#include "Taxes/LegacyTaxes/TaxC9_00.h"
#include "Taxes/LegacyTaxes/TaxCA01.h"
#include "Taxes/LegacyTaxes/TaxCA02.h"
#include "Taxes/LegacyTaxes/TaxCH3901.h"
#include "Taxes/LegacyTaxes/TaxCH3902.h"
#include "Taxes/LegacyTaxes/TaxCO.h"
#include "Taxes/LegacyTaxes/TaxD8_00.h"
#include "Taxes/LegacyTaxes/TaxD8_01.h"
#include "Taxes/LegacyTaxes/TaxD8_02.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDL.h"
#include "Taxes/LegacyTaxes/TaxDL01.h"
#include "Taxes/LegacyTaxes/TaxDU1_00.h"
#include "Taxes/LegacyTaxes/TaxEX.h"
#include "Taxes/LegacyTaxes/TaxF7_00.h"
#include "Taxes/LegacyTaxes/TaxGB01.h"
#include "Taxes/LegacyTaxes/TaxGB03.h"
#include "Taxes/LegacyTaxes/TaxHJ_00.h"
#include "Taxes/LegacyTaxes/TaxHK.h"
#include "Taxes/LegacyTaxes/TaxIN.h"
#include "Taxes/LegacyTaxes/TaxJN_00.h"
#include "Taxes/LegacyTaxes/TaxJP1_00.h"
#include "Taxes/LegacyTaxes/TaxKH1.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxOI_02.h"
#include "Taxes/LegacyTaxes/TaxQO.h"
#include "Taxes/LegacyTaxes/TaxRC.h"
#include "Taxes/LegacyTaxes/TaxSL1.h"
#include "Taxes/LegacyTaxes/TaxSP17.h"
#include "Taxes/LegacyTaxes/TaxSP1701.h"
#include "Taxes/LegacyTaxes/TaxSP1800.h"
#include "Taxes/LegacyTaxes/TaxSP1801.h"
#include "Taxes/LegacyTaxes/TaxSP24.h"
#include "Taxes/LegacyTaxes/TaxSP2901.h"
#include "Taxes/LegacyTaxes/TaxSP31.h"
#include "Taxes/LegacyTaxes/TaxSP36.h"
#include "Taxes/LegacyTaxes/TaxSP3601.h"
#include "Taxes/LegacyTaxes/TaxSP41.h"
#include "Taxes/LegacyTaxes/TaxSP4101.h"
#include "Taxes/LegacyTaxes/TaxSP4102.h"
#include "Taxes/LegacyTaxes/TaxSP43.h"
#include "Taxes/LegacyTaxes/TaxSP4301.h"
#include "Taxes/LegacyTaxes/TaxSP46.h"
#include "Taxes/LegacyTaxes/TaxSP48.h"
#include "Taxes/LegacyTaxes/TaxSP54.h"
#include "Taxes/LegacyTaxes/TaxSP60.h"
#include "Taxes/LegacyTaxes/TaxSP6602.h"
#include "Taxes/LegacyTaxes/TaxSP76.h"
#include "Taxes/LegacyTaxes/TaxSP8000.h"
#include "Taxes/LegacyTaxes/TaxSP9000.h"
#include "Taxes/LegacyTaxes/TaxSP9100.h"
#include "Taxes/LegacyTaxes/TaxSP9202.h"
#include "Taxes/LegacyTaxes/TaxSP9300.h"
#include "Taxes/LegacyTaxes/TaxSP9500.h"
#include "Taxes/LegacyTaxes/TaxSP9700.h"
#include "Taxes/LegacyTaxes/TaxSP9800.h"
#include "Taxes/LegacyTaxes/TaxSP9900.h"
#include "Taxes/LegacyTaxes/TaxSW.h"
#include "Taxes/LegacyTaxes/TaxSW1_new.h"
#include "Taxes/LegacyTaxes/TaxUH.h"
#include "Taxes/LegacyTaxes/TaxUI.h"
#include "Taxes/LegacyTaxes/TaxUI_01.h"
#include "Taxes/LegacyTaxes/TaxUS1_01.h"
#include "Taxes/LegacyTaxes/TaxUS2_01.h"
#include "Taxes/LegacyTaxes/TaxUT_00.h"
#include "Taxes/LegacyTaxes/TaxWN.h"
#include "Taxes/LegacyTaxes/TaxXG_01.h"
#include "Taxes/LegacyTaxes/TaxXG_10.h"
#include "Taxes/LegacyTaxes/TaxXS_00.h"
#include "Taxes/LegacyTaxes/TaxXV.h"
#include "Taxes/LegacyTaxes/TaxYN.h"
#include "Taxes/LegacyTaxes/TaxYS1_00.h"
#include "Taxes/LegacyTaxes/TaxZP_00.h"

namespace tse
{
FIXEDFALLBACK_DECL(apo43454d8tax);


namespace
{
ConfigurableValue<ConfigSet<std::string>>
newUs2Itins("TAX_SVC", "NEW_US2_ITINS");
}

const uint16_t BASE_TAX = 0; // General Tax
const uint16_t TAX_US1_01 = 1101; //
const uint16_t TAX_US1_02 = 1102; //
const uint16_t TAX_US2_01 = 13; // US International
const uint16_t TAX_ZP_00 = 1400; // US Segment Tax
const uint16_t TAX_AY = 16; // US Security Fee
const uint16_t TAX_SP17 = 17; // US Customs YC / US User Fee XA / US Inspection Fee XY
const uint16_t TAX_SP1701 = 1701; // XY inlude hidden stops
const uint16_t TAX_SP1800 = 1800; // US Security Fee (AY Tax ) - charge for separate segments
const uint16_t TAX_SP1801 = 1801; // US Security Fee (AY Tax ) - charge for separate segments
const uint16_t TAX_DL = 21; // Value Added Tax DL
const uint16_t TAX_DL_01 = 2101; // Value Added Tax DL
const uint16_t MEXICO_XV = 23; // Mexico Domestic Departure Tax
const uint16_t TAX_SP24 = 24; // Value Added Tax for Papua New Guine
const uint16_t TAX_CA_01 = 251; // Canada Air Security Charge - new implementation
const uint16_t TAX_CA_02 = 252; // Canada Air Security Charge - new implementation
const uint16_t TAX_RC = 26; // Canada Harmonized Sales Tax
const uint16_t TAX_RC_00 = 2600; // Canada Harmonized Sales Tax RC1/RC2
const uint16_t TAX_XG_01 = 2701; //
const uint16_t TAX_XG_10 = 2710; //
const uint16_t TAX_SP2901 = 2901; // XY exclude hidden stop, spn for v1 consistency
const uint16_t TAX_SP31 = 31; // Special Partial Tax XS/YF
const uint16_t TAX_JP1_00 = 3200; // Special Percentage Tax JP - new implementation
const uint16_t TAX_SL1 = 33; // Sierra Leone SL1
const uint16_t TAX_SP36 = 36; // New Special French Tax IZ
const uint16_t TAX_SP3601 = 3601; // fix for RG (farthest point logic)
const uint16_t GREAT_BRITIAN_GB3701 = 3701; // Great Britian Tax family
const uint16_t GREAT_BRITIAN_GB3703 = 3703; // Great Britian Tax family
const uint16_t GREAT_BRITIAN_GB3801 = 3801; // Great Britian Tax family
const uint16_t GREAT_BRITIAN_GB3803 = 3803; // Great Britian Tax family
const uint16_t TAX_CH3901 = 3901; //
const uint16_t TAX_CH3902 = 3902; //
const uint16_t TAX_SP41 = 41; // International Gateway BS1/MA2/PG2/QH/TU/TW/UR/VY/WC/WD/XR1
const uint16_t TAX_SP4101 = 4101; //
const uint16_t TAX_SP4102 = 4102; //
const uint16_t TAX_SP43 = 43; // Domestic Departure Tax Exemption
const uint16_t TAX_SP4301 = 4301; // WT
const uint16_t JAPAN_SW03 = 4503; // Japan PFC in NRT for stopover     ver#4
const uint16_t TAX_SP46 = 46; // From Via Loc Taxes VT/DE2
const uint16_t TAX_SP48 = 48; // Salse Tax for Columbia
const uint16_t FRANCE_UI50 = 50; // Frances Value Added adjustment tax
const uint16_t FRANCE_UI51 = 51; // Frances Value Added adjustment tax - 2016 version
const uint16_t RUSSIA_UH52 = 52; // Russia Security Charge
const uint16_t TAX_SP54 = 54; // Finland International Transit Fee(WL) and Transfer Fee (ZX)
const uint16_t TAX_SP60 = 60; // Embarkation Tax Charge & Airport Facillity Charge BR2/BR3/ZQ
const uint16_t HONG_KONG_HK = 63; // CX carrier departure exemption
const uint16_t TAX_KH1 = 65; // KH VAT Tax
const uint16_t TAX_SP6602 = 6602; // UK
const uint16_t TAX_QO = 67; // QO Argentina Immigration Tax
const uint16_t TAX_EX = 6900; // EX Italy tax
const uint16_t TAX_YS1_00 = 7200; // Base fare amount is rounded before applying tax
const uint16_t TAX_BP = 7300; // BP Korean tax
const uint16_t TAX_BE = 7400; // BE Embarkation Tax
const uint16_t TAX_SP76 = 76; // Brazil Tax
const uint16_t TAX_WN = 77; //  Macau Sar China  Passenger Service Charge
const uint16_t TAX_SW1_03 = 7803; // Japan PFC in NRT for connection   ver#4
const uint16_t TAX_OI_02 = 7902; // Japan Tax
const uint16_t TAX_YN = 80; // YN Tax
const uint16_t TAX_SP8000 = 8000; // IV IW Mexican ancillary fees
const uint16_t TAX_IN = 8800; // IN
const uint16_t TAX_SP9000 = 9000; // SG and OO to include hidden points
const uint16_t TAX_SP9100 = 9100; // SQ Canadian
const uint16_t TAX_SP9202 = 9202; // ZQ CL new
const uint16_t TAX_SP9300 = 9300; // OY German
const uint16_t TAX_BR = 9500; // BR
const uint16_t TAX_CO = 9600; // CO
const uint16_t TAX_SP9700 = 9700; // GR
const uint16_t TAX_SP9800 = 9800; // OW
const uint16_t TAX_SP9900 = 9900; // ZK
const uint16_t TAX_D8_00 = 216; // Malaysia D8 (0xd8) - first/temporary ver
const uint16_t TAX_D8_01 = 217; // Malaysia D8 - international exempts
const uint16_t TAX_D8_02 = 217; // Malaysia D8 - also domestic exempts
const uint16_t TAX_JN_00 = 250;
const uint16_t TAX_C9_00 = 201; // Bahamas
const uint16_t TAX_HJ_00 = 300; // Japan
const uint16_t TAX_F7_00 = 247; // F7 - Egiption tax
const uint16_t TAX_UT_00 = 350; // UT tax
const uint16_t TAX_DU1_00 = 450; // DU tax - COPA
const uint16_t TAX_XS_00 = 470; // XS, NV taxes

std::set<std::string> TaxMap::_newUS2Itins;

Logger
TaxMap::_logger("atseintl.Taxes");

void
TaxMap::buildTaxFactoryMap(DataHandle& dataHandle, TaxFactoryMap& taxFactoryMap)
{
  addTaxFactoryToMap<Tax>(dataHandle, taxFactoryMap, BASE_TAX);
  addTaxFactoryToMap<TaxZP_00>(dataHandle, taxFactoryMap, TAX_ZP_00);
  addTaxFactoryToMap<TaxSP17>(dataHandle, taxFactoryMap, TAX_SP17);
  addTaxFactoryToMap<TaxSP1800>(dataHandle, taxFactoryMap, TAX_SP1800);
  addTaxFactoryToMap<TaxSP1801>(dataHandle, taxFactoryMap, TAX_SP1801);
  addTaxFactoryToMap<TaxAY>(dataHandle, taxFactoryMap, TAX_AY);
  addTaxFactoryToMap<TaxDL>(dataHandle, taxFactoryMap, TAX_DL);
  addTaxFactoryToMap<TaxDL01>(dataHandle, taxFactoryMap, TAX_DL_01);
  addTaxFactoryToMap<TaxXV>(dataHandle, taxFactoryMap, MEXICO_XV);
  addTaxFactoryToMap<TaxSP24>(dataHandle, taxFactoryMap, TAX_SP24);
  addTaxFactoryToMap<TaxCA01>(dataHandle, taxFactoryMap, TAX_CA_01);
  addTaxFactoryToMap<TaxCA02>(dataHandle, taxFactoryMap, TAX_CA_02);
  addTaxFactoryToMap<TaxRC>(dataHandle, taxFactoryMap, TAX_RC);
  addTaxFactoryToMap<Tax>(dataHandle, taxFactoryMap, TAX_RC_00);
  addTaxFactoryToMap<TaxXG_01>(dataHandle, taxFactoryMap, TAX_XG_01);
  addTaxFactoryToMap<TaxXG_10>(dataHandle, taxFactoryMap, TAX_XG_10);
  addTaxFactoryToMap<TaxSP31>(dataHandle, taxFactoryMap, TAX_SP31);
  addTaxFactoryToMap<TaxSL1>(dataHandle, taxFactoryMap, TAX_SL1);
  addTaxFactoryToMap<TaxSP36>(dataHandle, taxFactoryMap, TAX_SP36);
  addTaxFactoryToMap<TaxSP41>(dataHandle, taxFactoryMap, TAX_SP41);
  addTaxFactoryToMap<TaxSP4101>(dataHandle, taxFactoryMap, TAX_SP4101);
  addTaxFactoryToMap<TaxSP4102>(dataHandle, taxFactoryMap, TAX_SP4102);
  addTaxFactoryToMap<TaxSP43>(dataHandle, taxFactoryMap, TAX_SP43);
  addTaxFactoryToMap<TaxSP46>(dataHandle, taxFactoryMap, TAX_SP46);
  addTaxFactoryToMap<TaxSP48>(dataHandle, taxFactoryMap, TAX_SP48);
  addTaxFactoryToMap<TaxUI>(dataHandle, taxFactoryMap, FRANCE_UI50);
  addTaxFactoryToMap<TaxUI_01>(dataHandle, taxFactoryMap, FRANCE_UI51);
  addTaxFactoryToMap<TaxUH>(dataHandle, taxFactoryMap, RUSSIA_UH52);
  addTaxFactoryToMap<TaxSP54>(dataHandle, taxFactoryMap, TAX_SP54);
  addTaxFactoryToMap<TaxSP60>(dataHandle, taxFactoryMap, TAX_SP60);
  addTaxFactoryToMap<TaxHK>(dataHandle, taxFactoryMap, HONG_KONG_HK);
  addTaxFactoryToMap<TaxKH1>(dataHandle, taxFactoryMap, TAX_KH1);
  addTaxFactoryToMap<TaxQO>(dataHandle, taxFactoryMap, TAX_QO);
  addTaxFactoryToMap<TaxYS1_00>(dataHandle, taxFactoryMap, TAX_YS1_00);
  addTaxFactoryToMap<TaxUS2_01>(dataHandle, taxFactoryMap, TAX_US2_01);
  addTaxFactoryToMap<TaxEX>(dataHandle, taxFactoryMap, TAX_EX);
  addTaxFactoryToMap<TaxBP>(dataHandle, taxFactoryMap, TAX_BP);
  addTaxFactoryToMap<TaxBE>(dataHandle, taxFactoryMap, TAX_BE);
  addTaxFactoryToMap<TaxCH3901>(dataHandle, taxFactoryMap, TAX_CH3901);
  addTaxFactoryToMap<TaxWN>(dataHandle, taxFactoryMap, TAX_WN);
  addTaxFactoryToMap<TaxGB01>(dataHandle, taxFactoryMap, GREAT_BRITIAN_GB3701);
  addTaxFactoryToMap<TaxGB01>(dataHandle, taxFactoryMap, GREAT_BRITIAN_GB3801);
  addTaxFactoryToMap<TaxGB03>(dataHandle, taxFactoryMap, GREAT_BRITIAN_GB3703);
  addTaxFactoryToMap<TaxGB03>(dataHandle, taxFactoryMap, GREAT_BRITIAN_GB3803);
  addTaxFactoryToMap<TaxCH3902>(dataHandle, taxFactoryMap, TAX_CH3902);
  addTaxFactoryToMap<TaxSP3601>(dataHandle, taxFactoryMap, TAX_SP3601);
  addTaxFactoryToMap<TaxUS1_01>(dataHandle, taxFactoryMap, TAX_US1_01);
  addTaxFactoryToMap<TaxUS1_01>(dataHandle, taxFactoryMap, TAX_US1_02);
  addTaxFactoryToMap<TaxSP76>(dataHandle, taxFactoryMap, TAX_SP76);
  addTaxFactoryToMap<TaxOI_02>(dataHandle, taxFactoryMap, TAX_OI_02);
  addTaxFactoryToMap<TaxJP1_00>(dataHandle, taxFactoryMap, TAX_JP1_00);
  addTaxFactoryToMap<TaxYN>(dataHandle, taxFactoryMap, TAX_YN);
  addTaxFactoryToMap<TaxSP8000>(dataHandle, taxFactoryMap, TAX_SP8000);
  addTaxFactoryToMap<TaxSP1701>(dataHandle, taxFactoryMap, TAX_SP1701);
  addTaxFactoryToMap<TaxSP2901>(dataHandle, taxFactoryMap, TAX_SP2901);
  addTaxFactoryToMap<TaxSW>(dataHandle, taxFactoryMap, JAPAN_SW03);
  addTaxFactoryToMap<TaxSW1_new>(dataHandle, taxFactoryMap, TAX_SW1_03);
  addTaxFactoryToMap<TaxSP9000>(dataHandle, taxFactoryMap, TAX_SP9000);
  addTaxFactoryToMap<TaxSP9100>(dataHandle, taxFactoryMap, TAX_SP9100);
  addTaxFactoryToMap<TaxSP9202>(dataHandle, taxFactoryMap, TAX_SP9202);
  addTaxFactoryToMap<TaxSP9300>(dataHandle, taxFactoryMap, TAX_SP9300);
  addTaxFactoryToMap<TaxSP9500>(dataHandle, taxFactoryMap, TAX_BR);
  addTaxFactoryToMap<TaxIN>(dataHandle, taxFactoryMap, TAX_IN);
  addTaxFactoryToMap<TaxCO>(dataHandle, taxFactoryMap, TAX_CO);
  addTaxFactoryToMap<TaxSP6602>(dataHandle, taxFactoryMap, TAX_SP6602);
  addTaxFactoryToMap<TaxSP4301>(dataHandle, taxFactoryMap, TAX_SP4301);
  addTaxFactoryToMap<TaxSP9700>(dataHandle, taxFactoryMap, TAX_SP9700);
  addTaxFactoryToMap<TaxSP9800>(dataHandle, taxFactoryMap, TAX_SP9800);
  addTaxFactoryToMap<TaxSP9900>(dataHandle, taxFactoryMap, TAX_SP9900);
  addTaxFactoryToMap<TaxD8_00>(dataHandle, taxFactoryMap, TAX_D8_00);
  if (fallback::fixed::apo43454d8tax())
  {
    addTaxFactoryToMap<TaxD8_01>(dataHandle, taxFactoryMap, TAX_D8_01);
  }
  else
  {
    addTaxFactoryToMap<TaxD8_02>(dataHandle, taxFactoryMap, TAX_D8_02);
  }
  addTaxFactoryToMap<TaxJN_00>(dataHandle, taxFactoryMap, TAX_JN_00);
  addTaxFactoryToMap<TaxC9_00>(dataHandle, taxFactoryMap, TAX_C9_00);
  addTaxFactoryToMap<TaxHJ_00>(dataHandle, taxFactoryMap, TAX_HJ_00);
  addTaxFactoryToMap<TaxF7_00>(dataHandle, taxFactoryMap, TAX_F7_00);
  addTaxFactoryToMap<TaxUT_00>(dataHandle, taxFactoryMap, TAX_UT_00);
  addTaxFactoryToMap<TaxDU1_00>(dataHandle, taxFactoryMap, TAX_DU1_00);
  addTaxFactoryToMap<TaxXS_00>(dataHandle, taxFactoryMap, TAX_XS_00);
}

bool
TaxMap::routeToOldUS2(Itin& itin, PricingTrx& trx)
{
  if (_routeToOldUS2Initialized)
    return _routeToOldUS2;

  _routeToOldUS2Initialized = true;
  std::string sign;
  std::string signWild;
  TaxUS2_01::calcItinSign(sign, signWild, itin, trx);
  std::set<std::string>::const_iterator it = _newUS2Itins.find(sign);

  if (it == _newUS2Itins.end())
    it = _newUS2Itins.find(signWild);

  if (it == _newUS2Itins.end())
    _routeToOldUS2 = true;
  else
    _routeToOldUS2 = false;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << std::string(52, '*') << "\n";
    if (_routeToOldUS2)
      stream << " * OLD";
    else
      stream << " * NEW";
    stream << " US2 TAX PROCESSING *  \n";
    stream << std::string(52, '*') << "\n";
    stream << "STRING LEGEND -\nX  - TICKET SOLD IN US\nU  - US*\nB  - BUFFER ZONE\n";
    stream << "A  - ALASKA\nH  - HAWAII\nO  - OTHER (INTL)\n  \n";
    stream << "* - USA EXCLUDING: ALASKA, HAWAII, AMERICAN SAMOA,\n";
    stream << "CANTON AND ENDERBURY, GUAM, MICRONESIA, MIDWAY,\n";
    stream << "NORTHERN MARIANA, PUERTORICO, SAIPAN, VIRGIN ISLAND,\nWAKE\n \n";
    stream << std::string(52, '*') << "\n";
    stream << "\nFILTER STRINGS FOR THIS ITIN\n";
    stream << "  " << sign << "\n";
    if (signWild != sign)
      stream << "  " << signWild << "\n";
    stream << "  \n" << std::string(52, '*') << "\n";
    stream << _newUS2Itins.size() << " STRINGS REDIRECTED TO NEW PROCESSING\n";
    uint16_t i = 1;
    for (it = _newUS2Itins.begin(); it != _newUS2Itins.end(); ++it, ++i)
    {
      stream << "  " << i << "  " << *it << "\n";
    }
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  return _routeToOldUS2;
}

void
TaxMap::setupNewUS2Itins()
{
  for (const auto newUs2Itin : newUs2Itins.getValue())
    _newUS2Itins.insert(newUs2Itin);
}
}
