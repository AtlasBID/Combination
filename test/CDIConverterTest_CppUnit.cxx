///
/// CppUnit tests for the converter
///
///

#include "Combination/CDIConverter.h"
#include "Combination/Parser.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace BTagCombination;
using namespace Analysis;

//
// Test harness/fixture for the parser
//

class CDIConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CDIConverterTest );

  CPPUNIT_TEST( testUncorrelatedErrors );

  CPPUNIT_TEST_SUITE_END();

  void testUncorrelatedErrors()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.variable = "eta";
    b1.binSpec.push_back(bb1);

    SystematicError e;
    e.name = "err";
    e.value = 0.1;
    e.uncorrelated = false;
    b1.systematicErrors.push_back(e);
    e.name = "uerr";
    e.uncorrelated = true;
    b1.systematicErrors.push_back(e);
    
    ana.bins.push_back(b1);

    CalibrationDataContainer *craw = ConvertToCDI (ana, "bogus");
    CalibrationDataHistogramContainer *c = dynamic_cast<CalibrationDataHistogramContainer *>(craw);
    CPPUNIT_ASSERT(c != 0);
    CPPUNIT_ASSERT_EQUAL(true, c->isBinCorrelated("err"));
    CPPUNIT_ASSERT_EQUAL(false, c->isBinCorrelated("uerr"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CDIConverterTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
