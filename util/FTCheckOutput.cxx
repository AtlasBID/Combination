///
/// FTCheckOutput
///
///  Check the CDI output file for any errors that might have crept it. Things like missing
/// SF or similar...
///
#include <TFile.h>
#include <TList.h>
#include <TKey.h>

#include <iostream>
#include <sstream>

using namespace std;

void usage(void)
{
  cout << "FTCheckOutput.exe <outputfile.root>" << endl;
}

// Abstract class that will do the checking
class ICheckTagCut {
public:
  virtual string Name (void) const = 0;
  virtual void CheckOutput (TDirectory *tagCutDir, const string &name) const = 0;
};

// Check to make sure all flavors are present.
class CheckFlavor : public ICheckTagCut {
public:
  CheckFlavor (const string &fname)
    : _flavor (fname)
  {}
  string Name (void) const { return "Check that a directory exists for flavor " + _flavor; }
  void CheckOutput (TDirectory *tagCutDir, const string &name) const;
private:
  const string _flavor;
};

class CheckForHisto : public ICheckTagCut {
public:
  CheckForHisto (const string &hname)
    : _histName (hname)
  {}
  string Name (void) const { return "Check that each flavor directory contains a histogram " + _histName; }
  void CheckOutput (TDirectory *t, const string &name) const;
private:
  const string _histName;
};

int main (int argc, char **argv)
{
  if (argc != 2) {
    usage();
    return 1;
  }

  //
  // Open the root file
  //

  TFile *f = TFile::Open(argv[1], "READ");
  if (!f->IsOpen()) {
    cout << "Error opening the output file " << argv[1] << "." << endl;
    return 1;
  }

  //
  // All the checkers
  //

  vector<ICheckTagCut*> checkers;
  checkers.push_back (new CheckFlavor("B"));
  checkers.push_back (new CheckFlavor("C"));
  checkers.push_back (new CheckFlavor("Light"));
  checkers.push_back (new CheckFlavor("T"));
  checkers.push_back (new CheckForHisto("default_SF"));
  checkers.push_back (new CheckForHisto("default_Eff"));

  //
  // Now, loop down through the first several parts of the root file. The top level is
  // tagger/jetAlg/Cut... we don't need to do anything but with those guys.
  //

  for (size_t i = 0; i < checkers.size(); i++) {
    const ICheckTagCut &checker(*checkers[i]);
    cout << endl << checker.Name() << endl;

    TIter i_tagger(f->GetListOfKeys());
    TKey *tagKey;
    while ((tagKey = static_cast<TKey*>(i_tagger()))) {
      if (tagKey->IsFolder()) {
	TDirectory *tagger = static_cast<TDirectory*>(tagKey->ReadObj());
	string taggerName (tagger->GetName());

	TKey *jetAlgKey;
	TIter i_jetAlg(tagger->GetListOfKeys());
	while ((jetAlgKey = static_cast<TKey*>(i_jetAlg()))) {
	  if (jetAlgKey->IsFolder()) {
	    TDirectory *jetAlg = static_cast<TDirectory*>(jetAlgKey->ReadObj());
	    string jetAlgName (jetAlg->GetName());

	    TKey *OPKey;
	    TIter i_OP(jetAlg->GetListOfKeys());
	    while ((OPKey = static_cast<TKey*>(i_OP()))) {
	      if (OPKey->IsFolder()) {
		TDirectory *OP = static_cast<TDirectory*>(OPKey->ReadObj());
		string OPName (OP->GetName());
	      
		//
		// Ok - now we are down at a unit. Do the checks we need to do.
		//

		ostringstream name;
		name << "  " << taggerName << "/" << jetAlgName << "/" << OPName;
		checker.CheckOutput(OP, name.str());
	      }
	    }
	  }
	}
      }
    }
  }

  return 0;
}

//
// Check to see if a given directory currently exists!
//
void CheckFlavor::CheckOutput(TDirectory *d, const string &name) const {
  TObject *o = d->Get(_flavor.c_str());
  if (o == 0)
    cout << "  " << name << endl;
}

//
// Check every sub-dir contains a particular object
//
void CheckForHisto::CheckOutput(TDirectory *d, const string &name) const {
  TKey *flavKey;
  TIter i_Flav(d->GetListOfKeys());
  while ((flavKey = static_cast<TKey*>(i_Flav()))) {
    if (flavKey->IsFolder()) {
      TDirectory *flav = static_cast<TDirectory*>(flavKey->ReadObj());
      TIter i_objs(flav->GetListOfKeys());
      TKey *objKey;
      bool found = false;
      while ((objKey = static_cast<TKey*>(i_objs()))) {
	if (_histName == objKey->GetName()) {
	  found = true;
	  break;
	}
      }
      if (!found) {
	cout << "  " << name << "/" << flav->GetName() << endl;
      }
    }
  }
}
