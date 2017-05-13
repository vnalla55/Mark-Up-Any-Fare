//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#ifndef COMBINABILITY_RULE_DAO_H
#define COMBINABILITY_RULE_DAO_H

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
class CombinabilityRuleInfo;
class DeleteList;

typedef HashKey<int> CombinabilityRuleKey;

class CombinabilityRuleDAO : public DataAccessObject<CombinabilityRuleKey, std::vector<CombinabilityRuleInfo*>>
{
public:
    CombinabilityRuleDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
    static CombinabilityRuleDAO& instance();

    const std::vector<CombinabilityRuleInfo*>& get(DeleteList& del,
                                                   const VendorCode&  vendor,
                                                   const CarrierCode&  carrier,
                                                   const TariffNumber&  ruleTariff,
                                                   const RuleNumber&  rule,
                                                   const DateTime& date,
                                                   const DateTime& ticketDate);

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData*
      compress(const std::vector<CombinabilityRuleInfo*>* vect) const;

    virtual std::vector<CombinabilityRuleInfo*>*
      uncompress(const sfc::CompressedData& compressed) const;
protected:

    static std::string _name;
    static std::string _cacheClass;
    std::vector<CombinabilityRuleInfo*>* create(CombinabilityRuleKey key);
    void destroy(CombinabilityRuleKey key, std::vector<CombinabilityRuleInfo*>* t);

private:
    static CombinabilityRuleDAO* _instance;
};

} // namespace tse
#endif // COMBINABILITY_RULE_DAO_H
