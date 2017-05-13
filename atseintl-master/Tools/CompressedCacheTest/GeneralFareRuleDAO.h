//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> GeneralFareRuleKey;
class GeneralFareRuleInfo;
class DeleteList;

class GeneralFareRuleDAO
  : public DataAccessObject<GeneralFareRuleKey, std::vector<GeneralFareRuleInfo*>>
{
public:
  static GeneralFareRuleDAO& instance();

  GeneralFareRuleDAO(int cacheSize = 0, const std::string& cacheType = "")
    : _loadedOnStartup(false)
  {}

  const std::vector<GeneralFareRuleInfo*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const TariffNumber& ruleTariff,
                                               const RuleNumber& rule,
                                               const CatNumber& category,
                                               const DateTime& date,
                                               const DateTime& ticketDate);

  const std::vector<GeneralFareRuleInfo*>& getForFD(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& rule,
                                                    const CatNumber& category,
                                                    const DateTime& date,
                                                    const DateTime& ticketDate);
  const std::string& cacheClass() { return _cacheClass; }

  virtual sfc::CompressedData *
    compress (const std::vector<GeneralFareRuleInfo*> *vect) const;

  virtual std::vector<GeneralFareRuleInfo*> *
    uncompress (const sfc::CompressedData &compressed) const;
protected:

  std::vector<GeneralFareRuleInfo*>* create(GeneralFareRuleKey key);
  void destroy(GeneralFareRuleKey key, std::vector<GeneralFareRuleInfo*>* t);

  void load();
  void clear();

  static std::string _name;
  static std::string _cacheClass;
  bool _loadedOnStartup;

private:
  template<typename T>
  const std::vector<GeneralFareRuleInfo*>& getImpl(DeleteList& del,
                                                   const VendorCode& vendor,
                                                   const CarrierCode& carrier,
                                                   const TariffNumber& ruleTariff,
                                                   const RuleNumber& rule,
                                                   const CatNumber& category,
                                                   const T& effectiveFilter);

  struct isEffective;
  static GeneralFareRuleDAO* _instance;
};
} // namespace tse
