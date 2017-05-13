//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{

class GeneralFareRuleInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, CatNumber, DateTime, DateTime>
GeneralFareRuleBackDatingKey;

class GeneralFareRuleBackDatingDAO
    : public HistoricalDataAccessObject<GeneralFareRuleBackDatingKey,
                                        std::vector<GeneralFareRuleInfo*> >
{
public:
  static GeneralFareRuleBackDatingDAO& instance();

  const std::vector<GeneralFareRuleInfo*>& get(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const TariffNumber& ruleTariff,
                                               const RuleNumber& rule,
                                               const CatNumber& category,
                                               const DateTime& backDate);

  const std::vector<GeneralFareRuleInfo*>& getForFD(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& rule,
                                                    const CatNumber& category,
                                                    const DateTime& backDate);

  bool translateKey(const ObjectKey& objectKey, GeneralFareRuleBackDatingKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
                objectKey.getValue("CATEGORY", key._e) && objectKey.getValue("STARTDATE", key._f) &&
                objectKey.getValue("ENDDATE", key._g));
  }

  bool translateKey(const ObjectKey& objectKey,
                    GeneralFareRuleBackDatingKey& key,
                    const DateTime backDate) const override
  {
    DAOUtils::getDateRange(backDate, key._f, key._g, _cacheBy);
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
                objectKey.getValue("CATEGORY", key._e));
  }

  void setKeyDateRange(GeneralFareRuleBackDatingKey& key, const DateTime backDate) const override
  {
    DAOUtils::getDateRange(backDate, key._f, key._g, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<GeneralFareRuleInfo*>* vect) const override;

  virtual std::vector<GeneralFareRuleInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<GeneralFareRuleBackDatingDAO>;

  static DAOHelper<GeneralFareRuleBackDatingDAO> _helper;

  GeneralFareRuleBackDatingDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GeneralFareRuleBackDatingKey, std::vector<GeneralFareRuleInfo*> >(
          cacheSize, cacheType, 3)
  {
  }

  std::vector<GeneralFareRuleInfo*>* create(GeneralFareRuleBackDatingKey key) override;
  void destroy(GeneralFareRuleBackDatingKey key, std::vector<GeneralFareRuleInfo*>* t) override;

  std::vector<GeneralFareRuleInfo*>& commonGet(DeleteList& del,
                                               const VendorCode& vendor,
                                               const CarrierCode& carrier,
                                               const TariffNumber& ruleTariff,
                                               const RuleNumber& rule,
                                               const CatNumber& category,
                                               const DateTime& backDate);

private:
  static GeneralFareRuleBackDatingDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}

