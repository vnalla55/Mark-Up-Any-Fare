//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> ContractPreferenceKey;
class ContractPreference;
class DeleteList;

class ContractPreferenceDAO : public DataAccessObject<ContractPreferenceKey, std::vector<ContractPreference*>>
{
public:
    static ContractPreferenceDAO& instance();
    const std::vector<ContractPreference*>& get(DeleteList& del,
                                                const PseudoCityCode& pseudoCity,
                                                const CarrierCode& carrier,
                                                const RuleNumber& rule,
                                                const DateTime& date,
                                                const DateTime& ticketDate);


    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData *
      compress (const std::vector<ContractPreference*> *vect) const;

    virtual std::vector<ContractPreference*> *
      uncompress (const sfc::CompressedData &compressed) const;

    ContractPreferenceDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
protected:
    static std::string _name;
    static std::string _cacheClass;

    std::vector<ContractPreference*>* create(ContractPreferenceKey key);
  void destroy(ContractPreferenceKey key, std::vector<ContractPreference*>* recs);
    void load() ;

private:
    static ContractPreferenceDAO* _instance;
    struct isEffective;
    struct groupByKey;
};
} // namespace tse
