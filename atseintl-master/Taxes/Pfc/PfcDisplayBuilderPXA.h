//----------------------------------------------------------------------------
//  File:           PfcDisplayBuilderPXA.h
//  Authors:        Piotr Lach
//  Created:        5/23/2008
//  Description:    PfcDisplayBuilderPXA header file for ATSE V2 PFC Display Project.
//                  The object of this class build PXA message.
//
//  Copyright Sabre 2008
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
#ifndef PFC_DISPLAY_BUILDER_PXA_H
#define PFC_DISPLAY_BUILDER_PXA_H

#include "Taxes/Pfc/PfcDisplayBuilder.h"
#include "Taxes/Pfc/PfcDisplayDataPXA.h"
#include "Taxes/Pfc/PfcDisplayFormatter.h"
#include <vector>

namespace tse
{

class PfcDisplayBuilderPXA : public PfcDisplayBuilder
{
public:
  static const std::string MAIN_HEADER_PXA;

  PfcDisplayBuilderPXA(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXA();
};

class PfcDisplayBuilderPXA_generalInfo : public PfcDisplayBuilderPXA
{
public:
  static const std::string INFO_HEADER;
  static const std::string HELP_HEADER;
  static const std::string TABLE_HEADER;
  static const std::string TABLE_FORMATTING;

  PfcDisplayBuilderPXA_generalInfo(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXA_generalInfo();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXA* data() override { return (PfcDisplayDataPXA*)_data; }
  const PfcDisplayDataPXA* data() const override { return (PfcDisplayDataPXA*)_data; }
  PfcDisplayFormatterPXA& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXA _formatter;

  friend class PfcDisplayBuilderPXATest;
};

class PfcDisplayBuilderPXA_detailInfo : public PfcDisplayBuilderPXA
{
public:
  static const std::string MULTICARRIER_INFO_HEADER;
  static const std::string INFO_HEADER;
  static const std::string TABLE_HEADER1;
  static const std::string TABLE_HEADER2;
  static const std::string HELP_HEADER;
  static const std::string SEQ_HEADER;
  static const std::string SEQ_LINE1;
  static const std::string SEQ_LINE2;
  static const std::string SEQ_LINE3;

  PfcDisplayBuilderPXA_detailInfo(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXA_detailInfo();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;
  virtual std::string buildFootnote() override;

  std::string geoApplDesc(Indicator i) const;
  std::string absorbTypeDesc(Indicator i) const;
  std::string owrtDesc(Indicator i) const;

  PfcDisplayDataPXA* data() override { return (PfcDisplayDataPXA*)_data; }
  const PfcDisplayDataPXA* data() const override { return (PfcDisplayDataPXA*)_data; }

  PfcDisplayFormatterPXA& fmt() { return _formatter; }
  PfcDisplayFormatterPXA& fmtSeqHeader() { return _formatterSeqHeader; }
  PfcDisplayFormatterPXA& fmtSeqLine1() { return _formatterSeqLine1; }
  PfcDisplayFormatterPXA& fmtSeqLine2() { return _formatterSeqLine2; }
  PfcDisplayFormatterPXA& fmtSeqLine3() { return _formatterSeqLine3; }

  bool& isMoreThanOneLine()
  {
    return _isMoreThanOneLine;
  };
  const bool isMoreThanOneLine() const
  {
    return _isMoreThanOneLine;
  };

private:
  PfcDisplayFormatterPXA _formatter; // main table formater
  PfcDisplayFormatterPXA _formatterSeqHeader;
  PfcDisplayFormatterPXA _formatterSeqLine1;
  PfcDisplayFormatterPXA _formatterSeqLine2;
  PfcDisplayFormatterPXA _formatterSeqLine3;

  bool _isMoreThanOneLine;
};

class PfcDisplayBuilderPXA_diffInfo : public PfcDisplayBuilderPXA_detailInfo
{
public:
  PfcDisplayBuilderPXA_diffInfo(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXA_diffInfo();

  virtual std::string build() override;
};

class PfcDisplayBuilderPXA_itineraryInfo : public PfcDisplayBuilderPXA
{
public:
  static const std::string INFO_HEADER;
  static const std::string HELP_HEADER;
  static const std::string TABLE_HEADER;
  static const std::string TABLE_FORMATTING;

  PfcDisplayBuilderPXA_itineraryInfo(TaxTrx* trx, PfcDisplayData* data);
  virtual ~PfcDisplayBuilderPXA_itineraryInfo();

protected:
  virtual std::string buildHeader() override;
  virtual std::string buildBody() override;

  PfcDisplayDataPXA* data() override { return (PfcDisplayDataPXA*)_data; }
  const PfcDisplayDataPXA* data() const override { return (PfcDisplayDataPXA*)_data; }
  PfcDisplayFormatterPXA& fmt() { return _formatter; }

private:
  PfcDisplayFormatterPXA _formatter;

  friend class PfcDisplayBuilderPXATest;
};

} // namespace tse
#endif
