#include "stdafx.h"

#pragma managed (push, off)
#include <Combination/Parser.h>
#pragma managed (pop)

#include <vector>
using namespace std;
using namespace BTagCombination;

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace ParserTest
{
	[TestClass]
	public ref class BasicTests
	{
	private:
		TestContext^ testContextInstance;

	public: 
		[TestMethod]
		void TestEmptyString()
		{
			vector<CalibrationAnalysis> result (Parse(""));
			Assert::AreEqual( (unsigned int) 0, result.size(), "# of found analyses");
		};

		[TestMethod]
		void TestAnalysisWithNoBin()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){}"));
			Assert::AreEqual((unsigned int) 1, result.size(), "# of found analyses");
			Assert::AreEqual((unsigned int) 0, result[0].bins.size(), "# of bins");
		}

		[TestMethod]
		void TestAnalysisWithOneBin()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30){central_value(0.9, 0.1)}}"));
			Assert::AreEqual((unsigned int) 1, result.size(), "# of found analyses");
			Assert::AreEqual((unsigned int) 1, result[0].bins.size(), "# of bins");
			auto bb = result[0].bins[0];
			Assert::AreEqual((unsigned int) 1, bb.binSpec.size(), "# of bins");
			Assert::IsTrue(bb.binSpec[0].variable == "pt", "incorrect bin name");
			Assert::AreEqual(5.0, bb.binSpec[0].lowvalue, "low value");
			Assert::AreEqual(30.0, bb.binSpec[0].highvalue, "high value");
		}

		[TestMethod]
		void TestAnalysisWithTwoBins()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){central_value(0.9, 0.1)}}"));
			Assert::AreEqual((unsigned int) 1, result.size(), "# of found analyses");
			Assert::AreEqual((unsigned int) 1, result[0].bins.size(), "# of bins");
			auto bb = result[0].bins[0];
			Assert::AreEqual((unsigned int) 2, bb.binSpec.size(), "# of bins");
			Assert::IsTrue(bb.binSpec[0].variable == "pt", "incorrect bin name");
			Assert::IsTrue(bb.binSpec[1].variable == "eta", "incorrect bin name");
		}

		[TestMethod]
		void TestAnalysisCentralValueAbs()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){central_value(0.9, 0.1)}}"));
			auto bin = result[0].bins[0];
			Assert::AreEqual(0.9, bin.centralValue, "Central Value");
			Assert::AreEqual(0.1, bin.centralValueStatisticalError, "Central value statistical error");
		}

		[TestMethod]
		[ExpectedException(System::Runtime::InteropServices::SEHException::typeid)]
		void TestAnalysisTwoCentralValue()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){central_value(0.9, 0.1)central_value(0.1, 0.1)}}"));
		}

		[TestMethod]
		void TestAnalysisCentralValueRel()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){central_value(0.9, 10.0%)}}"));
			auto bin = result[0].bins[0];
			Assert::AreEqual(0.9, bin.centralValue, "Central Value");
			Assert::IsTrue(fabs((0.09 - bin.centralValueStatisticalError)) < 0.001, "Central value statistical error");
			Assert::AreEqual((int) 0, (int) bin.systematicErrors.size(), "Number of systematic errors");
		}

		[TestMethod]
		void TestAnalysisSysErrorAbs()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){central_value(0.9, 10.0%)sys(JES,10%)}}"));
			auto bin = result[0].bins[0];
			Assert::AreEqual(1, (int) bin.systematicErrors.size(), "Number of systematic errors");
			auto sys = bin.systematicErrors[0];
			Assert::IsTrue("JES" == sys.name, "Sys Error Name");
			Assert::IsTrue (abs(0.09 - sys.value) < 0.0001, "Sys value");
		}

		[TestMethod]
		void TestAnalysisSysErrorFirst()
		{
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30, 20<eta<44){sys(JES,0.2)central_value(0.9, 10.0%)}}"));
			auto bin = result[0].bins[0];
			Assert::AreEqual(1, (int) bin.systematicErrors.size(), "Number of systematic errors");
			auto sys = bin.systematicErrors[0];
			Assert::IsTrue("JES" == sys.name, "Sys Error Name");
		}
	};
}
