#
# tseserver Makefile
#

include	../../makefile_common

GDC_INC_DIR = -I$(VOB_DIR)/Tools/CTAnalyzer/gdlib/gdchartlib/include

INC_DIR  = $(GLOBAL_INCLUDE_DIR) \
           $(ADP_INC_DIR) \
           $(MAN_INC_DIR) \
           $(SRV_INC_DIR) \
           $(SVC_INC_DIR)\
           $(DM_INC_DIR) \
	   $(GDC_INC_DIR)

LIB_DIR  = $(GLOBAL_LIB)
BIN_DIR  = $(GLOBAL_BIN)

GDC_DIR = $(VOB_DIR)/Tools/CTAnalyzer/gdlib/gdchartlib/lib

GD_LIB = -L. -lgdc -lgd \
	-L$(VOB_DIR)/Tools/CTAnalyzer/gdlib/lib \
	-L$(GDC_DIR) \
	-lfreetype

CC_INCS = $(INC_DIR)
LIBS = -ldl \
	$(BOOST_REGEX_LIB) \
	$(GLOBAL_LIB_DIR) \
	$(BOOST_DATE_TIME_LIB) \
	$(GD_LIB)

EXE    = CallTreeAnalyzer

EXE_SRCS = \
	CTTokenVector.cpp \
        CallTreeMethod.cpp \
        CallTreeMethodTree.cpp \
        CallTreeDataCollector.cpp \
        CallTreeDataAnalyzer.cpp \
	CallTreeAnalyzer.cpp \
        CallTreeXMLGenerator.cpp \
	CallTreeChartData.cpp

EXE_OBJS = $(EXE_SRCS:%.cpp=$(BUILD)/%.o)
OBJS = $(EXE_OBJS)
SRC = $(EXE_SRCS)

compile: $(BUILD)/$(EXE)

all: compile tests

all-clean: clean tests-clean

clean :
	@echo Deleting objects
	@rm -f ./$(BUILD)/*.o
	@rm -f ./$(BUILD)/*.so 
	@rm -f ./*.bb
	@rm -f ./*.bbg
	@rm -f ./*.da
	@rm -f ./*.gcov
	@rm -f $(BIN_DIR)/$(EXE)
	@rm -f $(GLOBAL_LIB)/$(EXE)

$(BUILD)/$(EXE) : $(EXE_OBJS)
	@echo Linking $@
	@$(LINKER) -o $@ $(LIBS) $(EXE_OBJS) -export-dynamic $(EXE_LINKFLAGS)
	@chmod a+rx $@
	@cp -f $@ $(GLOBAL_BIN)/.

$(BUILD)/%.o : %.cpp
	@echo Compiling $<
	@$(CC) -c $(COVERAGE_FLAGS) $(CFLAGS_BASE) $(CC_INCS) $< -o $@ 

#-----------------------------------------------------------------
# CPPUnit-related targets are now in makefile_common_targets
#-----------------------------------------------------------------
include	../../makefile_common_targets
