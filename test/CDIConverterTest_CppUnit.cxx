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
#include <cmath>

using namespace std;
using namespace BTagCombination;
using namespace Analysis;

//
// Test harness/fixture for the parser
//

class CDIConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CDIConverterTest );

  CPPUNIT_TEST( testBasicGetSF );
  CPPUNIT_TEST( testBasicGetIrregularSF );
  CPPUNIT_TEST( testUncorrelatedErrors );
  CPPUNIT_TEST( testForSystematics );

  CPPUNIT_TEST( testExtendedBinNormalArea );
  CPPUNIT_TEST( testExtendedBinExtendedArea );

  CPPUNIT_TEST( testBeyondTheEdge );

  CPPUNIT_TEST_SUITE_END();

  void testBasicGetSF()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
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

    Analysis::CalibrationDataVariables v;
    v.jetPt = 50.e3;
    v.jetEta = -1.1;
    v.jetAuthor = "AntiKt4Topo";

    TObject *obj = nullptr;
    double r;
    CalibrationDataContainer::CalibrationStatus stat = c->getResult(v, r, obj);

    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.1, r, 0.001);

    stat = c->getStatUncertainty(v, r);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, r, 0.001);
  }

  void testBasicGetIrregularSF()
  {
    cout << "Starting testBasicGetIrregularSF()" << endl;
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
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

    b1.binSpec[0].lowvalue = 100.0;
    b1.binSpec[0].highvalue = 200.0;
    b1.binSpec[1].highvalue = 1.0;
    b1.centralValue = 5.0;
    ana.bins.push_back(b1);

    b1.binSpec[1].lowvalue = 1.0;
    b1.binSpec[1].highvalue = 2.5;
    b1.centralValue = 4.0;
    ana.bins.push_back(b1);

    CalibrationDataContainer *craw = ConvertToCDI (ana, "bogus");
    CalibrationDataMappedHistogramContainer *c = dynamic_cast<CalibrationDataMappedHistogramContainer *>(craw);

    CPPUNIT_ASSERT (c != 0);

    Analysis::CalibrationDataVariables v;
    v.jetPt = 50e3;
    v.jetEta = -1.1;
    v.jetAuthor = "AntiKt4Topo";

    TObject *obj = nullptr;
    double r;
    cout << "Doing first lookup for 50 x 1.1" << endl;

    CalibrationDataContainer::CalibrationStatus stat = c->getResult(v, r, obj);

    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.1, r, 0.001);

    stat = c->getStatUncertainty(v, r);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, r, 0.001);

    v.jetPt = 120.e3;
    v.jetEta = -0.3;
    obj = nullptr;
    stat = c->getResult(v, r, obj);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (5.0, r, 0.001);

    v.jetEta = 1.3;
    obj = nullptr;
    stat = c->getResult(v, r, obj);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (4.0, r, 0.001);
  }

  void testForSystematics()
  {
    cout << "Testing for systematics" << endl;
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
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

    //
    // Get a list of uncertianties
    //

    Analysis::CalibrationDataVariables v;
    v.jetPt = 50.e3;
    v.jetEta = -1.1;
    v.jetAuthor = "AntiKt4Topo";

    map<string, UncertaintyResult> all;
    CalibrationDataContainer::CalibrationStatus stat;
    stat = c->getUncertainties(v, all);
    bool found = false;
    for (map<string, UncertaintyResult>::const_iterator itr = all.begin(); itr != all.end(); itr++) {
      cout << " Uncert " << itr->first << endl;
      UncertaintyResult res (itr->second);
      if (itr->first == "systematics") {
	found = true;
	double x = 0.1*sqrt(2);
	CPPUNIT_ASSERT_DOUBLES_EQUAL (x, res.second, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL (x, res.first, 0.001);
      }
      if (itr->first == "err") {
	double x = 0.1;
	CPPUNIT_ASSERT_DOUBLES_EQUAL (x, res.second, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL (x, res.first, 0.001);
      }
    }
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_EQUAL (true, found);
    

    cout << "Done testing for systematics" << endl;
  }

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

  CalibrationAnalysis generateExtendedBinnedAna()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    // The normal bin

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
    b1.binSpec.push_back(bb1);

    SystematicError e;
    e.name = "cerr";
    e.uncorrelated = false;
    b1.systematicErrors.push_back(e);
    
    ana.bins.push_back(b1);

    // The bin with the extended error

    CalibrationBin b2;
    b2.centralValue = 1.2;
    b2.centralValueStatisticalError = 0.3;
    b2.isExtended = true;

    bb1.lowvalue = 100.0;
    bb1.highvalue = 200.0;
    bb1.variable = "pt";
    b2.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
    b2.binSpec.push_back(bb1);

    e.name = "extendederror";
    e.uncorrelated = true;
    e.value = 0.2;

    ana.bins.push_back(b2);
    return ana;
  }

  void testExtendedBinExtendedArea()
  {
    CalibrationAnalysis ana (generateExtendedBinnedAna());

    CalibrationDataContainer *craw = ConvertToCDI (ana, "bogus");
    CalibrationDataHistogramContainer *c = dynamic_cast<CalibrationDataHistogramContainer *>(craw);

    Analysis::CalibrationDataVariables v;
    v.jetPt = 150.e3;
    v.jetEta = -1.1;
    v.jetAuthor = "AntiKt4Topo";

    TObject *obj = nullptr;
    double r;
    CalibrationDataContainer::CalibrationStatus stat = c->getResult(v, r, obj);

    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.2, r, 0.001);

    stat = c->getStatUncertainty(v, r);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.3, r, 0.001);

    map<string, UncertaintyResult> allerrors;
    stat = c->getUncertainties(v, allerrors);
    //CPPUNIT_ASSERT_EQUAL((size_t)5, allerrors.size());

    CPPUNIT_ASSERT(allerrors.find("cerr") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("result") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("statistics") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("systematics") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("extendederror") != allerrors.end());
  }

  void testExtendedBinNormalArea()
  {
    CalibrationAnalysis ana (generateExtendedBinnedAna());

    CalibrationDataContainer *craw = ConvertToCDI (ana, "bogus");
    CalibrationDataHistogramContainer *c = dynamic_cast<CalibrationDataHistogramContainer *>(craw);

    Analysis::CalibrationDataVariables v;
    v.jetPt = 50.e3;
    v.jetEta = -1.1;
    v.jetAuthor = "AntiKt4Topo";

    TObject *obj = nullptr;
    double r;
    CalibrationDataContainer::CalibrationStatus stat = c->getResult(v, r, obj);

    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.1, r, 0.001);

    stat = c->getStatUncertainty(v, r);
    CPPUNIT_ASSERT_EQUAL (CalibrationDataContainer::kSuccess, stat);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, r, 0.001);

    map<string, UncertaintyResult> allerrors;
    stat = c->getUncertainties(v, allerrors);
    for (map<string, UncertaintyResult>::const_iterator itr = allerrors.begin(); itr != allerrors.end(); itr++) {
      cout << " --> Error " << itr->first << endl;
    }
    CPPUNIT_ASSERT_EQUAL((size_t)1+3, allerrors.size());

    CPPUNIT_ASSERT(allerrors.find("cerr") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("result") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("statistics") != allerrors.end());
    CPPUNIT_ASSERT(allerrors.find("systematics") != allerrors.end());
  }

  // Make sure we fail correctly if we ask for a number outside the
  // range that the CDI was made with.
  void testBeyondTheEdge()
  {
    CPPUNIT_ASSERT(false);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CDIConverterTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
