#include <list>
#include <string>

#include <boost/assign/list_of.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/test/ProcessTagInfoMock.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ProcessTagPermutationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ProcessTagPermutationTest);

  CPPUNIT_TEST(testGetEndorsementByteValueX1);
  CPPUNIT_TEST(testGetEndorsementByteValueX2);
  CPPUNIT_TEST(testGetEndorsementByteValueX3);
  CPPUNIT_TEST(testGetEndorsementByteValueX4);
  CPPUNIT_TEST(testGetEndorsementByteValueW);
  CPPUNIT_TEST(testGetEndorsementByteValueBlank);
  CPPUNIT_TEST(testGetEndorsementByteValueY);
  CPPUNIT_TEST(testGetEndorsementByteValueYmixed);

  CPPUNIT_TEST(testGetReisueToLowerByteValueF1);
  CPPUNIT_TEST(testGetReisueToLowerByteValueF2);
  CPPUNIT_TEST(testGetReisueToLowerByteValueR);
  CPPUNIT_TEST(testGetReisueToLowerByteValueBlank);

  CPPUNIT_TEST(testCheckElectronicTicketBlank);
  CPPUNIT_TEST(testCheckElectronicTicketBlankAndR);
  CPPUNIT_TEST(testCheckElectronicTicketBlankAndN);
  CPPUNIT_TEST(testCheckElectronicTicketOnlyN);
  CPPUNIT_TEST(testCheckElectronicTicketOnlyR);
  CPPUNIT_TEST(testCheckElectronicTicketMixed);

  CPPUNIT_TEST(testGetResidualPenaltyByteBlank);
  CPPUNIT_TEST(testGetResidualPenaltyByteI);
  CPPUNIT_TEST(testGetResidualPenaltyByteN);
  CPPUNIT_TEST(testGetResidualPenaltyByteR);
  CPPUNIT_TEST(testGetResidualPenaltyByteS);
  CPPUNIT_TEST(testGetResidualPenaltyByteEmptyVec);
  CPPUNIT_TEST(testGetResidualPenaltyByteOneBlank);
  CPPUNIT_TEST(testGetResidualPenaltyByteOneN);

  CPPUNIT_TEST(testFormOfRefundIndValueS);
  CPPUNIT_TEST(testFormOfRefundIndValueV);
  CPPUNIT_TEST(testFormOfRefundIndValueM);
  CPPUNIT_TEST(testFormOfRefundIndValueBLANK);

  CPPUNIT_TEST(testFirstWithT988Second);
  CPPUNIT_TEST(testFirstWithT988No);

  CPPUNIT_TEST(testHasZeroT988True);
  CPPUNIT_TEST(testHasZeroT988False);

  CPPUNIT_TEST(testResidualMandatesSameBlank);
  CPPUNIT_TEST(testResidualMandatesSameR);
  CPPUNIT_TEST(testResidualMandatesChangedIN);
  CPPUNIT_TEST(testResidualMandatesChangedNR);
  CPPUNIT_TEST(testResidualMandatesChangedNRmost);
  CPPUNIT_TEST(testResidualMandatesChangedSRmost);

  CPPUNIT_TEST(testGetConnectionStopoverByte_empty);
  CPPUNIT_TEST(testGetConnectionStopoverByte_onlyBlank);
  CPPUNIT_TEST(testGetConnectionStopoverByte_onlyS);
  CPPUNIT_TEST(testGetConnectionStopoverByte_onlyC);
  CPPUNIT_TEST(testGetConnectionStopoverByte_onlyB);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixBlankS);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixBlankC);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixBlankB);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixSC);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixSCB);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixSB);
  CPPUNIT_TEST(testGetConnectionStopoverByte_mixCB);

  CPPUNIT_TEST_SUITE_END();

public:
  static const Indicator LAST_RESTRICTIVE;

  template <typename T>
  T* create()
  {
    return _mem.create<T>();
  }

  void addPtis()
  {
    for (int i = 0; i != 3; ++i)
      processTagPermutation_->processTags().push_back(create<ProcessTagInfo>());
  }

  void testFirstWithT988Second()
  {
    addPtis();
    processTagPermutation_->processTags()[1]->reissueSequence()->orig() = create<ReissueSequence>();
    CPPUNIT_ASSERT_EQUAL(
        const_cast<const ProcessTagInfo*>(processTagPermutation_->processTags()[1]),
        processTagPermutation_->firstWithT988());
  }

  void testFirstWithT988No()
  {
    addPtis();
    CPPUNIT_ASSERT(!processTagPermutation_->firstWithT988());
  }

  void testHasZeroT988True()
  {
    addPtis();
    processTagPermutation_->processTags()[0]->reissueSequence()->orig() = create<ReissueSequence>();
    processTagPermutation_->processTags()[2]->reissueSequence()->orig() = create<ReissueSequence>();
    CPPUNIT_ASSERT(processTagPermutation_->hasZeroT988());
  }

  void testHasZeroT988False()
  {
    addPtis();
    for (int i = 0; i != 3; ++i)
      processTagPermutation_->processTags()[i]->reissueSequence()->orig() =
          create<ReissueSequence>();
    CPPUNIT_ASSERT(!processTagPermutation_->hasZeroT988());
  }

  void testResidualMandatesSameBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_BLANK,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testResidualMandatesSameR()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_R,
                       ProcessTagPermutation::RESIDUAL_R,
                       ProcessTagPermutation::RESIDUAL_R};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_R,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void residualPTI(const std::vector<std::pair<bool, Indicator>>& statuses)
  {
    int index = 0;
    std::vector<std::pair<bool, Indicator>>::const_iterator si = statuses.begin();
    std::vector<std::pair<bool, Indicator>>::const_iterator sie = statuses.end();
    for (; si != sie; ++si, ++index)
    {
      ProcessTagInfo& pti = *processTagPermutation_->processTags()[index];
      const_cast<VoluntaryChangesInfo&>(*pti.record3()->orig()).residualHierarchy() = si->second;

      pti.fareCompInfo() = create<FareCompInfo>();
      pti.fareCompInfo()->fareMarket() = create<FareMarket>();
      for (int i = 0; i != 3; ++i)
      {
        pti.fareCompInfo()->fareMarket()->travelSeg().push_back(create<AirSeg>());
        if (i == 1 && si->first)
          pti.fareCompInfo()->fareMarket()->travelSeg().back()->changeStatus() = TravelSeg::CHANGED;
        else
          pti.fareCompInfo()->fareMarket()->travelSeg().back()->changeStatus() =
              TravelSeg::UNCHANGED;
      }
    }
  }

  void testResidualMandatesChangedIN()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_I,
                       ProcessTagPermutation::RESIDUAL_N,
                       ProcessTagPermutation::RESIDUAL_R};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    residualPTI(boost::assign::list_of(std::make_pair(true, LAST_RESTRICTIVE))(std::make_pair(
        true, LAST_RESTRICTIVE))(std::make_pair(false, ProcessTagPermutation::RESIDUAL_BLANK)));

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_N,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testResidualMandatesChangedNR()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_N,
                       ProcessTagPermutation::RESIDUAL_S,
                       ProcessTagPermutation::RESIDUAL_R};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    residualPTI(boost::assign::list_of(std::make_pair(true, LAST_RESTRICTIVE))(std::make_pair(
        false, ProcessTagPermutation::RESIDUAL_BLANK))(std::make_pair(true, LAST_RESTRICTIVE)));

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_R,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testResidualMandatesChangedNRmost()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_N,
                       ProcessTagPermutation::RESIDUAL_I,
                       ProcessTagPermutation::RESIDUAL_R};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    residualPTI(boost::assign::list_of(std::make_pair(true, LAST_RESTRICTIVE))(std::make_pair(
        false, LAST_RESTRICTIVE))(std::make_pair(true, ProcessTagPermutation::RESIDUAL_BLANK)));

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_N,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testResidualMandatesChangedSRmost()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_S,
                       ProcessTagPermutation::RESIDUAL_R,
                       ProcessTagPermutation::RESIDUAL_I};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    residualPTI(boost::assign::list_of(std::make_pair(true, ProcessTagPermutation::RESIDUAL_BLANK))(
        std::make_pair(true, LAST_RESTRICTIVE))(std::make_pair(false, LAST_RESTRICTIVE)));

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_R,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  template <int size>
  void helperSetupProcessTags(Indicator (&ind)[size],
                              ProcessTagInfoMock* (ProcessTagInfoMock::*set_method)(Indicator))
  {
    info_ = new ProcessTagInfoMock[size];

    std::transform(info_,
                   info_ + size,
                   ind,
                   std::back_inserter(processTagPermutation_->processTags()),
                   std::mem_fun_ref(set_method));
  }

  void setUp()
  {
    processTagPermutation_ = create<ProcessTagPermutation>();
    info_ = 0;
  }

  void tearDown()
  {
    _mem.clear();
    delete[] info_;
  }

  void testGetEndorsementByteValueX1()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_X};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_BLANK,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueX2()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_W,
                       ProcessTagPermutation::ENDORSEMENT_X};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_BLANK,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueX3()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_X,
                       ProcessTagPermutation::ENDORSEMENT_W};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_BLANK,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueX4()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_X,
                       ProcessTagPermutation::ENDORSEMENT_X,
                       ProcessTagPermutation::ENDORSEMENT_X};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_X,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueW()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_W,
                       ProcessTagPermutation::ENDORSEMENT_W,
                       ProcessTagPermutation::ENDORSEMENT_W};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_W,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_BLANK,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueY()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_Y,
                       ProcessTagPermutation::ENDORSEMENT_Y,
                       ProcessTagPermutation::ENDORSEMENT_Y};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    //    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_Y,
    //                         processTagPermutation_->getEndorsementByte());
  }

  void testGetEndorsementByteValueYmixed()
  {
    Indicator ind[] = {ProcessTagPermutation::ENDORSEMENT_BLANK,
                       ProcessTagPermutation::ENDORSEMENT_X,
                       ProcessTagPermutation::ENDORSEMENT_Y};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::endorsement>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ENDORSEMENT_BLANK,
                         processTagPermutation_->getEndorsementByte());
  }

  void testGetReisueToLowerByteValueF1()
  {
    Indicator ind[] = {ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                       ProcessTagPermutation::REISSUE_TO_LOWER_F,
                       ProcessTagPermutation::REISSUE_TO_LOWER_R};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::reissueToLower>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::REISSUE_TO_LOWER_F,
                         processTagPermutation_->getReissueToLowerByte());
  }

  void testGetReisueToLowerByteValueF2()
  {
    Indicator ind[] = {ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                       ProcessTagPermutation::REISSUE_TO_LOWER_R,
                       ProcessTagPermutation::REISSUE_TO_LOWER_F,
                       ProcessTagPermutation::REISSUE_TO_LOWER_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::reissueToLower>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::REISSUE_TO_LOWER_F,
                         processTagPermutation_->getReissueToLowerByte());
  }

  void testGetReisueToLowerByteValueR()
  {
    Indicator ind[] = {ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                       ProcessTagPermutation::REISSUE_TO_LOWER_R,
                       ProcessTagPermutation::REISSUE_TO_LOWER_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::reissueToLower>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::REISSUE_TO_LOWER_R,
                         processTagPermutation_->getReissueToLowerByte());
  }

  void testGetReisueToLowerByteValueBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                       ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                       ProcessTagPermutation::REISSUE_TO_LOWER_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::reissueToLower>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::REISSUE_TO_LOWER_BLANK,
                         processTagPermutation_->getReissueToLowerByte());
  }

  void testCheckElectronicTicketBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                       ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                       ProcessTagPermutation::ELECTRONIC_TICKET_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testCheckElectronicTicketBlankAndR()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                       ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testCheckElectronicTicketBlankAndN()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                       ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testCheckElectronicTicketOnlyN()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testCheckElectronicTicketOnlyR()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testCheckElectronicTicketMixed()
  {
    Indicator ind[] = {ProcessTagPermutation::ELECTRONIC_TICKET_BLANK,
                       ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED,
                       ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::electronicTktInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::ELECTRONIC_TICKET_MIXED,
                         processTagPermutation_->checkTable988Byte123());
  }

  void testGetResidualPenaltyByteBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_BLANK,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteI()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_N,
                       ProcessTagPermutation::RESIDUAL_I};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_I,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteN()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_R,
                       ProcessTagPermutation::RESIDUAL_N};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_N,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteR()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_R,
                       ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_S};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_R,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteS()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK,
                       ProcessTagPermutation::RESIDUAL_S,
                       ProcessTagPermutation::RESIDUAL_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_S,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteEmptyVec()
  {
    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_BLANK,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteOneBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_BLANK};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_BLANK,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void testGetResidualPenaltyByteOneN()
  {
    Indicator ind[] = {ProcessTagPermutation::RESIDUAL_N};

    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::residualInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::RESIDUAL_N,
                         processTagPermutation_->getResidualPenaltyByte());
  }

  void setUpFormOfRefund(char ind1, char ind2, char ind3)
  {
    Indicator ind[] = {ind1, ind2, ind3};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setVCI<&VoluntaryChangesInfo::formOfRefund>);
  }

  void testFormOfRefundIndValueS()
  {
    setUpFormOfRefund('V', 'S', 'M');
    CPPUNIT_ASSERT_EQUAL('S', processTagPermutation_->formOfRefundInd());
  }

  void testFormOfRefundIndValueV()
  {
    setUpFormOfRefund('V', ' ', 'M');
    CPPUNIT_ASSERT_EQUAL('V', processTagPermutation_->formOfRefundInd());
  }

  void testFormOfRefundIndValueM()
  {
    setUpFormOfRefund('M', ' ', 'M');
    CPPUNIT_ASSERT_EQUAL('M', processTagPermutation_->formOfRefundInd());
  }

  void testFormOfRefundIndValueBLANK()
  {
    setUpFormOfRefund(' ', ' ', ' ');
    CPPUNIT_ASSERT_EQUAL(' ', processTagPermutation_->formOfRefundInd());
  }

  void testGetConnectionStopoverByte_empty()
  {
    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_BLANK,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_onlyBlank()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_BLANK};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_BLANK,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_onlyS()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_S};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_S,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_onlyC()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_C,
                       ProcessTagPermutation::STOPCONN_C,
                       ProcessTagPermutation::STOPCONN_C};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_C,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_onlyB()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_B,
                       ProcessTagPermutation::STOPCONN_B,
                       ProcessTagPermutation::STOPCONN_B};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixBlankS()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_BLANK};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_S,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixBlankC()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_C,
                       ProcessTagPermutation::STOPCONN_BLANK};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_C,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixBlankB()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_B,
                       ProcessTagPermutation::STOPCONN_BLANK};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixSC()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_C};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixSCB()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_B,
                       ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_C,
                       ProcessTagPermutation::STOPCONN_BLANK};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixSB()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_S,
                       ProcessTagPermutation::STOPCONN_B};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

  void testGetConnectionStopoverByte_mixCB()
  {
    Indicator ind[] = {ProcessTagPermutation::STOPCONN_BLANK,
                       ProcessTagPermutation::STOPCONN_B,
                       ProcessTagPermutation::STOPCONN_C};
    helperSetupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::stopoverConnectInd>);

    CPPUNIT_ASSERT_EQUAL(ProcessTagPermutation::STOPCONN_B,
                         processTagPermutation_->getStopoverConnectionByte());
  }

private:
  ProcessTagPermutation* processTagPermutation_;
  ProcessTagInfoMock* info_;
  TestMemHandle _mem;
};

const Indicator ProcessTagPermutationTest::LAST_RESTRICTIVE = 'X';

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessTagPermutationTest);

} // tse
