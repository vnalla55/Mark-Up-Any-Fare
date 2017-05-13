//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/CommissionKeys.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionRuleInfo.h"

#include "test/include/TestMemHandle.h"

#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <cassert>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

namespace tse
{
namespace amc
{
namespace amcTestUtil
{

struct CommissionProgramData
{
  const CommissionProgramInfo* commProgInfo;
  const CommissionContractInfo* commContInfo;
  CommissionProgramData()
    : commProgInfo(nullptr), commContInfo(nullptr) {}
  CommissionProgramData(
      const CommissionProgramInfo* cpi,
      const CommissionContractInfo* ccInfo)
    : commProgInfo(cpi), commContInfo(ccInfo) {}
};
// trim from start
static inline std::string &ltrim(std::string &s)
{
  s.erase(s.begin(),
      std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(),
        s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string trim(std::string &s)
{
  return ltrim(rtrim(s));
}

/*
std::vector<std::string>
split(const std::string& input, const std::string& delim)
{
  std::regex rgx(delim);
  // passing -1 as the submatch index parameter performs splitting
  std::sregex_token_iterator
    first{begin(input), end(input), rgx, -1},
    last;
  return {first, last};
}
*/

void split(std::vector<std::string>& resCol,
    const std::string& input,
    const std::string& delim)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(delim.c_str());
  tokenizer tokens(input, sep);
  for (tokenizer::iterator tok_iter = tokens.begin();
      tok_iter != tokens.end(); ++tok_iter)
  {
    resCol.push_back(*tok_iter);
  }
}

CommissionProgramData getCommissionProgramInfo(
    TestMemHandle& memHandle, int& pid)
{
  CommissionProgramData cpd;
  CommissionProgramInfo* cpi = memHandle.create<CommissionProgramInfo>();
  CommissionContractInfo* ccInfo = memHandle.create<CommissionContractInfo>();
  assert(cpi != nullptr);
  assert(ccInfo != nullptr);
  cpi->programId()=pid;
  return CommissionProgramData(cpi, ccInfo);
}

CommissionRuleInfo* getCommissionRuleInfo(
    TestMemHandle& memHandle,
    uint64_t v, unsigned ct, int& pid, int& cid)
{
  CommissionRuleInfo* cri = memHandle.create<CommissionRuleInfo>();
  assert(cri != nullptr);
  cri->commissionId()=cid++;
  cri->programId()=pid;
  cri->commissionValue()=v;
  cri->commissionTypeId()=ct;
  return cri;
}

void
getRules(TestMemHandle& memHandle,
    SortedCommRuleDataVec& ruleCol,
    unsigned ct, const std::string& ruleStr) // ruleStr = 22,14,(3)
{
  //std::regex rx("\\(|\\)");
  boost::regex rx("\\(|\\)");
  int cid = 1;
  int pid = 1;

  std::vector<std::string> resCol;
  split(resCol, ruleStr, ","); // y 22,14,(3)
  for (std::string& y : resCol)
  {
    int v = 0;
    bool iq = y.find("(") != std::string::npos ? true : false;
    if (iq)
    {
      //std::string tmp = std::regex_replace(y, rx, "");
      std::string tmp = boost::regex_replace(y, rx, "");
      v = atoi(trim(tmp).c_str());
    }
    else
      v = atoi(trim(y).c_str());

    CommissionProgramData cpd = getCommissionProgramInfo(memHandle, pid);
    CommissionRuleData r(
        getCommissionRuleInfo(memHandle, v, ct, pid, cid),
        cpd.commProgInfo,
        cpd.commContInfo);
    ruleCol.insert(r);
  }
}

//9:22,14,(3) - 10:22,14,(3) - 11:22,14,(3)");
void
getRuleData(TestMemHandle& memHandle, const std::string& str, CommRuleDataColPerCT& ctRuleCol)
{
  std::vector<std::string> rdV;
  split(rdV, str, "-");
  assert(!rdV.empty());

  for (const std::string& x : rdV) // x is 11:22,14,(3)
  {
    std::vector<std::string> rdX;
    split(rdX, x, ":");
    assert(rdX.size()==2);

    int ct = atoi(trim(rdX[0]).c_str());
    assert (ct == CT9 || ct == CT10 || ct == CT11);

    SortedCommRuleDataVec ruleCol;
    getRules(memHandle, ruleCol, ct, rdX[1]);
    assert(!ruleCol.empty());

    ctRuleCol[ct] = ruleCol;
  }
}
/* Moved to FcCommissionRuleDataTest
// 9:22,14,(3) - 10:22,14,(3) - 11:22,14,(3)
FcCommissionRuleData*
parseTestInput(TestMemHandle& memHandle,
    const FareUsage* fu,
    std::string str)
{
  trim(str);
  if (str.empty() || str == "nullptr")
    return nullptr;

  CommRuleDataColPerCT criColPerCt;
  getRuleData(memHandle, str, criColPerCt);
  assert(!criColPerCt.empty());

  FcCommissionRuleData* rd = memHandle.create<FcCommissionRuleData>();
  assert(rd != nullptr);
  rd->commRuleDataColPerCommType() = criColPerCt;
  return rd;
}*/

} // amcTestUtil
} // amc
} // tse

