//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h" // operator<<
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

#include <memory>
#include <iostream>
#include <string>

namespace tse
{

namespace utils
{

class ISopCombinationsGenerator: public IGenerator<SopCombination>
{
public:
  // Generator needs to know the total number
  // of legs we need to cover. If any leg
  // (including trailing) has no SOPs,
  // we know that the result set is empty.
  virtual void setNumberOfLegs(unsigned int legs) = 0;
  virtual unsigned int getNumberOfLegs() const = 0;

  virtual void addSop(unsigned int legId, uint32_t sopId) = 0;
  virtual const SopCombination& getSopsOnLeg(unsigned int legId) const = 0;

  // When all combinations are exhausted, starts
  // returning an empty SopCombination on each
  // following call.
  // Empty combinations are returned immediately if:
  // a) no input SOPs have been supplied
  // b) there is a "gap" leg having no SOPs
  SopCombination next() override = 0; // from IGenerator

  // Exposed to have possibility to initialize
  // the generator in a user-defined moment,
  // before the first call to next().
  virtual void manualInit() = 0;

  virtual ~ISopCombinationsGenerator(){}
};


class ISopCombinationsGeneratorDecorator:
    public ISopCombinationsGenerator
{
public:
  ISopCombinationsGeneratorDecorator(
      ISopCombinationsGenerator* decorated): _decorated(decorated)
  {
    TSE_ASSERT(decorated != nullptr);
  }

  void setNumberOfLegs(unsigned int legs) override
  {
    _decorated->setNumberOfLegs(legs);
  }

  unsigned int getNumberOfLegs() const override
  {
    return _decorated->getNumberOfLegs();
  }

  void addSop(unsigned int legId, uint32_t sopId) override
  {
    _decorated->addSop(legId, sopId);
  }

  const SopCombination& getSopsOnLeg(unsigned int legId) const override
  {
    return _decorated->getSopsOnLeg(legId);
  }

  SopCombination next() override
  {
    return _decorated->next();
  }

  void manualInit() override
  {
    _decorated->manualInit();
  }

private:
  ISopCombinationsGenerator* _decorated;
};


class ISopCombinationsGeneratorFactory
{
public:
  virtual ISopCombinationsGenerator* create() = 0;
  virtual ~ISopCombinationsGeneratorFactory(){}
};


class ISopBank
{
public:
  virtual void setNumberOfLegs(unsigned int legs) = 0;
  virtual unsigned int getNumberOfLegs() const = 0;

  virtual void addSop(unsigned int legId, uint32_t sopId) = 0;

  virtual SopCombination& getSopsOnLeg(unsigned int legId) = 0;

  virtual std::string toString() const = 0;

  virtual ~ISopBank(){}
};


class SopBank: public ISopBank, boost::noncopyable
{
public:
  void setNumberOfLegs(unsigned int legs) override;

  unsigned int getNumberOfLegs() const override;

  void addSop(unsigned int legId, uint32_t sopId) override;

  SopCombination& getSopsOnLeg(unsigned int legId) override;

  std::string toString() const override;

private:
  SopCombinationList _sops;
};



class BaseSopCombinationsGenerator: public ISopCombinationsGenerator,
    boost::noncopyable
{
public:

  BaseSopCombinationsGenerator(ISopBank* sopBank = new SopBank()) : _isInitialized(false)
  {
    _userInputSops.reset(sopBank);
  }

  void setNumberOfLegs(unsigned int legs) override;

  unsigned int getNumberOfLegs() const override;

  void addSop(unsigned int legId, uint32_t sopId) override;

  // Returns a list of SOPs on the given leg
  const SopCombination& getSopsOnLeg(unsigned int legId) const override;

  SopCombination next() override;

  void manualInit() override;

protected:

  virtual void initialize() = 0;

  virtual SopCombination nextElement() = 0;

  // Contains a list of SOP ids for consecutive
  // legs 0, 1, 2..
  typedef std::shared_ptr<ISopBank> SopBankPtr;
  SopBankPtr _userInputSops;

private:

  // A flag controlling if the cartesian product
  // has been loaded yet
  bool _isInitialized;
};


void formatSopCombinationsGenerator(std::ostream& out,
    const ISopCombinationsGenerator& g);

std::ostream& operator<<(std::ostream& out,
    const ISopCombinationsGenerator& g);


} // namespace utils

} // namespace tse


