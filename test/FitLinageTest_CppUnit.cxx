// Tests for the fit linage functions.


#include "Combination/FitLinage.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>


using namespace std;
using namespace BTagCombination;

class FitLinageTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(FitLinageTest);

	CPPUNIT_TEST(nameFromBlankAnalysis);
	CPPUNIT_TEST(nameFromLinageAnalysis);

	CPPUNIT_TEST(combineEmptyList);
	CPPUNIT_TEST(combineOneList);
	CPPUNIT_TEST(combineTwoList);

	CPPUNIT_TEST(dStarLinage);
	CPPUNIT_TEST(addSysLinage);
	CPPUNIT_TEST(extrapLinage);

	CPPUNIT_TEST_SUITE_END();

	void nameFromBlankAnalysis()
	{
		CalibrationAnalysis ana;
		ana.name = "hithere";

		string r(Linage(ana));
		CPPUNIT_ASSERT_EQUAL(string("hithere"), r);
	}

	void nameFromLinageAnalysis()
	{
		CalibrationAnalysis ana;
		ana.name = "hithere";
		ana.metadata_s["Linage"] = "noway";

		string r(Linage(ana));
		CPPUNIT_ASSERT_EQUAL(string("noway"), r);
	}

	void combineEmptyList()
	{
		vector<CalibrationAnalysis> lst;
		string r(CombineLinage(lst, LCFitCombine));
		CPPUNIT_ASSERT_EQUAL(string(""), r);
	}

	void combineOneList()
	{
		vector<CalibrationAnalysis> lst;
		CalibrationAnalysis a1;
		a1.name = "a1";
		lst.push_back(a1);
		string r(CombineLinage(lst, LCFitCombine));
		CPPUNIT_ASSERT_EQUAL(string("a1"), r);
	}

	void combineTwoList()
	{
		vector<CalibrationAnalysis> lst;
		CalibrationAnalysis a1;
		a1.name = "a1";
		lst.push_back(a1);
		CalibrationAnalysis a2;
		a2.name = "a2";
		lst.push_back(a2);
		string r(CombineLinage(lst, LCFitCombine));
		CPPUNIT_ASSERT_EQUAL(string("a1+a2"), r);
	}

	void dStarLinage()
	{
		CalibrationAnalysis dstemp;
		dstemp.name = "DStar";
		CalibrationAnalysis ttbar;
		ttbar.name = "ttbar_pdf";

		string r(BinaryLinageOp(dstemp, ttbar, LBDStar));
		CPPUNIT_ASSERT_EQUAL(string("D*(ttbar_pdf=>DStar)"), r);
	}

	void addSysLinage()
	{
		CalibrationAnalysis ttbar;
		ttbar.name = "ttbar";

		string r(BinaryLinageOp(ttbar, "newerror", LBAddSys));
		CPPUNIT_ASSERT_EQUAL(string("addSys(newerror=>ttbar)"), r);
	}
	
	void extrapLinage()
	{
		CalibrationAnalysis ttbar;
		ttbar.name = "ttbar";
		CalibrationAnalysis mc;
		mc.name = "MCCalib";

		string r(BinaryLinageOp(ttbar, mc, LBExtrapolate));
		CPPUNIT_ASSERT_EQUAL(string("extrap(MCCalib=>ttbar)"), r);
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(FitLinageTest);
