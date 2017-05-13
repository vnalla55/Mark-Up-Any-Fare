//-------------------------------------------------------------------
//
//  File:        CombFareClassMap.h
//  Created:     Jun 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class to map together add-on and fare FareClass
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "AddonConstruction/ConstructionDefs.h"
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include "AddonConstruction/SpecifiedFareCache.h"
#endif
#include "DBAccess/AddonCombFareClassInfo.h"


namespace tse
{
class AddonFareCortege;
class ConstructedFare;
class ConstructionJob;


class CombFareClassMap
{
public:
  // construction/destruction
  // ========================

  CombFareClassMap();
  ~CombFareClassMap();

  // Accessors
  //

  // main interface
  // ==== =========
  void init(ConstructionJob* cj);
  void setTariff(TariffNumber tariff);

  bool populate();

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  FareMatchCode matchFareClassesHistorical(const FareInfo& specFare,
                                           const AddonFareCortege& addonFC,
                                           const Indicator geoAppl,
                                           const Indicator owrt,
                                           TSEDateInterval& validDI);

  const AddonFareClasses* matchSpecifiedFare(const FareInfo& specFare);

#else

  FareMatchCode matchFareClasses(ConstructedFare& specFare,
                                 const AddonFareCortege& addonFC,
                                 const Indicator geoAppl,
                                 const Indicator owrt,
                                 AddonCombFareClassInfoVec& fClassCombRecords);

#endif

private:
  ConstructionJob* _cJob;
  const DateTime _date;
  TariffNumber _fareTariff;
  bool _diagHeaderPrinted;
  const AddonCombFareClassInfoVec* _fareClassCombVector;

  const AddonFareClassCombMultiMap* _fareClassCombMultiMap;

  typedef std::map<TariffNumber, const AddonFareClassCombMultiMap*>
  TariffAddonFareClassCombMultiMap;

  TariffAddonFareClassCombMultiMap _tariffCombFareClassMap;

  typedef std::map<TariffNumber, const AddonCombFareClassInfoVec*> TariffCombFareClassVecMap;
  TariffCombFareClassVecMap _tariffCombFareClassVecMap;

  void writeDiagnostics();
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  CombFareClassMap(const CombFareClassMap& rhs);
  CombFareClassMap operator=(const CombFareClassMap& rhs);

}; // End class CombFareClassMap

} // End namespace tse

