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
			vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(5<pt<30)}"));
			Assert::AreEqual((unsigned int) 1, result.size(), "# of found analyses");
			Assert::AreEqual((unsigned int) 1, result[0].bins.size(), "# of bins");
			auto bb = result[0].bins[0];
			Assert::AreEqual((unsigned int) 1, bb.binSpec.size(), "# of bins");
		}
	};
}
