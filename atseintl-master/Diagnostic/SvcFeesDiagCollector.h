//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class SvcFeesAccCodeInfo;
class SvcFeesTktDesignatorInfo;
class SvcFeesSecurityInfo;
class SvcFeesFareIdInfo;
class SvcFeesFeatureInfo;

class SvcFeesDiagCollector : public DiagCollector
{
public:
  friend class SvcFeesDiagCollectorTest;

  explicit SvcFeesDiagCollector(Diagnostic& root) : DiagCollector(root) {}
  SvcFeesDiagCollector() {}

  SvcFeesDiagCollector& operator<<(const SvcFeesAccCodeInfo& accCode);
  SvcFeesDiagCollector& operator<<(const SvcFeesTktDesignatorInfo& tktDes);
  void printValidationStatus(bool status);
  void printAccountCodeTable172Header(int itemNo);
  void printTktDesignatorTable173Header(int itemNo);
  void printTktDesignatorFailInfo(StatusT173 status);
  void printSecurityTable183Header(int itemNo);
  void
  printSecurityTable183Info(const SvcFeesSecurityInfo* feeSec, const StatusT183Security status);
  void printSecurityTable183EmptyMsg(VendorCode vc);
  void printT189DetailInfo(const SvcFeesFareIdInfo* info, bool printSeparator = true);
  void printT166DetailInfo(const SvcFeesFeatureInfo* info);

private:
  void displayMinAmount(const SvcFeesFareIdInfo* info);
  void displayMaxAmount(const SvcFeesFareIdInfo* info);
  void displaySegmentApplication(Indicator ind);

  const static std::string _passed;
  const static std::string _failed;
};
}
