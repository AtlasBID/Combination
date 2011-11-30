///
/// Implement a cache of RooRealVars
///
#ifndef __RooRealVarCache__
#define __RooRealVarCache__

#include <map>
#include <string>
#include <vector>

class RooRealVar;

class RooRealVarCache
{
public:
	RooRealVarCache(void);
	/// When deleted all roo real vars will also be deleted!
	~RooRealVarCache(void);

	/// Looks up a roo real var, returns null if we don't know about it.
	RooRealVar *FindRooVar (const std::string &what) const;

	/// Looks up a roo real var. If not there, then will create it, with the range given, and it set in the middle.
	RooRealVar *FindOrCreateRooVar(const std::string &what, const double minval, const double maxval);

	/// Returns the names of all the vars we are hanging on to.
	std::vector<std::string> GetAllVars(void) const;

private:
	/// The cache that holds the roo real vars.
	std::map<std::string, RooRealVar *> _vars; // Cache of all things this context is measureing.
};

#endif
