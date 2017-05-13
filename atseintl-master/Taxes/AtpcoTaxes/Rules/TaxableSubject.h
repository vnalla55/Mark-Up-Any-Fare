// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#pragma once

#include <vector>

#include "Rules/TaxableData.h"

namespace tax
{
template <typename Subject>
class TaxableSubjectSingle
{
public:
  explicit TaxableSubjectSingle(TaxableData& data) : _data(&data), _failedRule(nullptr) {}
  Subject _subject;
  TaxableData* _data;

  void setFailedRule(const BusinessRule* rule) { _failedRule = rule; }
  const BusinessRule* getFailedRule() const { return _failedRule; }
  bool isFailedRule() const { return _failedRule != nullptr; }

private:
  const BusinessRule* _failedRule;
};

template <typename Subject>
class TaxableSubjectMulti
{
public:
  TaxableSubjectMulti() : _subject(0), _validCount(0) {}
  void init(const std::vector<Subject>& subjects, std::vector<type::Index>& ids)
  {
    assert(ids.size() == subjects.size());
    _ids.swap(ids);
    _validCount = _ids.size();
    _data.resize(_ids.size());
    _failedRules.resize(_ids.size(), 0);
    _subject = subjects;
  }
  void setFailedRule(const type::Index id, const BusinessRule& rule)
  {
    _failedRules[id] = &rule;
    --_validCount;
  }
  const BusinessRule* getFailedRule(const type::Index id) const
  {
    return _failedRules[id];
  }
  bool isFailedRule(const type::Index id) const
  {
    return _failedRules[id] != nullptr;
  }
  bool areAllFailed() const
  {
    return _validCount == 0;
  }
  type::Index getValidCount() const { return _validCount; }

  type::MoneyAmount getTotalAmount() const
  {
    type::MoneyAmount result(0);
    for (size_t i = 0; i < _subject.size(); ++i)
      result += (_failedRules[i]) ? 0 : _data[i]._taxableAmount;
    return result;
  }

  std::vector<type::Index> _ids; // we assume that _ids is sorted in ascending order
  std::vector<TaxableData> _data;
  std::vector<Subject> _subject;
  type::MoneyAmount _taxableAmount;

protected:
  std::vector<const BusinessRule*> _failedRules;
  type::Index _validCount;
};
}

