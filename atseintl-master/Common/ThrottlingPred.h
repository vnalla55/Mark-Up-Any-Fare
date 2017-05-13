//
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

#include "Common/TseCodeTypes.h"

#include <boost/lexical_cast.hpp>
#include <memory>

namespace tse
{
class AncillaryPricingTrx;
class PricingTrx;
class ShoppingTrx;
class TaxTrx;
class Trx;

class Pred
{
public:
  virtual ~Pred() = default;

  virtual bool test(const Trx& trx) const;
  virtual bool test(const PricingTrx& trx) const;
  virtual bool test(const AncillaryPricingTrx& trx) const;
  virtual bool test(const TaxTrx& trx) const;
  virtual bool test(const ShoppingTrx& trx) const;

  static void
  parse(std::vector<std::string>& keyValue, const std::string& parseStr, const std::string sep);
};

class IfThen : public Pred
{
public:
  IfThen() = default;
  IfThen(Pred& ifPred, Pred& thenPred) : _ifPred(&ifPred), _thenPred(&thenPred) {}

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  const Pred* _ifPred = nullptr;
  const Pred* _thenPred = nullptr;
};

class OrPred : public Pred
{
public:
  OrPred(Pred& oper1, Pred& oper2) : _oper1(oper1), _oper2(oper2) {}

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  Pred& _oper1;
  Pred& _oper2;
};

class AndPred : public Pred
{
public:
  AndPred(Pred& oper1, Pred& oper2) : _oper1(oper1), _oper2(oper2) {}

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  Pred& _oper1;
  Pred& _oper2;
};

class PCCPred : public Pred
{
public:
  PCCPred(const std::string& val) : _val(val) {}

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;

private:
  const std::string _val;
};

class LNIATAPred : public Pred
{
public:
  LNIATAPred(const std::string& val) : _val(val) {}

  virtual bool test(const PricingTrx& trx) const override;

private:
  const std::string _val;
};

class PNRPred : public Pred
{
public:
  PNRPred(const std::string& val) : _val(val) {}

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override { return false; }

private:
  const std::string _val;
};

class PARTPred : public Pred
{
public:
  PARTPred(const std::string& val) : _val(val) {}

  virtual bool test(const PricingTrx& trx) const override;

private:
  const std::string _val;
};

class SEGEqualPred : public Pred
{
public:
  SEGEqualPred(const std::string& val) : _val(boost::lexical_cast<uint16_t>(val)) {}

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  uint16_t _val;
};

class SEGLessPred : public Pred
{
public:
  SEGLessPred(const std::string& val) : _val(boost::lexical_cast<uint16_t>(val)) {}

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  uint16_t _val;
};

class SEGGreaterPred : public Pred
{
public:
  SEGGreaterPred(const std::string& val) : _val(boost::lexical_cast<uint16_t>(val)) {}

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  uint16_t _val;
};

class SVCPred : public Pred
{
public:
  SVCPred(const std::string& val) : _val(val) {}

  virtual bool test(const PricingTrx& trx) const override;

private:
  const std::string _val;
};

class MKTPred : public Pred
{
public:
  MKTPred(const std::string& val);

  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  LocCode _origLocCode;
  LocCode _destLocCode;
};

class OperPriorSelect : public Pred
{
public:
  OperPriorSelect(const std::string& str);

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

protected:
  void setOperands(const std::string& str, size_t& pos);

private:
  std::unique_ptr<Pred> _pred;
  std::unique_ptr<Pred> _oper1;
  std::unique_ptr<Pred> _oper2;
};

class IncrementCounter : public Pred
{
public:
  IncrementCounter(const std::string& counterCondStr, const std::string& keyCondStr);

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;

private:
  std::string _keyCondStr;
  uint16_t _concurrentTrx;
};

class KeyPred : public Pred
{
public:
  KeyPred(const std::string& str);

  virtual bool test(const Trx& trx) const override;
  virtual bool test(const PricingTrx& trx) const override;
  virtual bool test(const AncillaryPricingTrx& trx) const override;
  virtual bool test(const TaxTrx& trx) const override;
  virtual bool test(const ShoppingTrx& trx) const override;

private:
  IfThen _ifThen;
  std::unique_ptr<Pred> _operSel;
  std::unique_ptr<Pred> _incCounter;
};

class ParseCustomer : public Pred
{
public:
  ParseCustomer(const std::string& str);

  virtual bool test(const Trx& trx) const override;

private:
  IfThen _ifThen;
  std::unique_ptr<Pred> _operSel;
  std::unique_ptr<Pred> _key;
};

} // end tse
