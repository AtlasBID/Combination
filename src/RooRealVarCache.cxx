///
/// Implement the roo real vars

#include "Combination/RooRealVarCache.h"

#include <RooRealVar.h>

#include <algorithm>
#include <stdexcept>

using namespace std;

RooRealVarCache::RooRealVarCache(void)
{
}

///
/// Clean up all the roo real vars we are tracking.
///
RooRealVarCache::~RooRealVarCache(void)
{
  for(map<string,RooRealVar*>::const_iterator itr = _vars.begin(); itr != _vars.end(); itr++) {
    delete itr->second;
  }
}

///
/// See if the var is in our cache. Return null if it isn't.
///
RooRealVar *RooRealVarCache::FindRooVar (const string &what) const
{
  map<string,RooRealVar*>::const_iterator thatsIt = _vars.find(what);
  if (thatsIt == _vars.end())
    return nullptr;

  return thatsIt->second;
}

///
/// If we have a var in our cache, return it. Otherwise, we will need to create it.
///
RooRealVar *RooRealVarCache::FindOrCreateRooVar(const string &what, const double minval, const double maxval)
{
  RooRealVar *v = FindRooVar(what);
  if (v != nullptr)
    return v;

  v = new RooRealVar (what.c_str(), what.c_str(), (maxval-minval)/2.0 + minval, minval, maxval);
  _vars[what] = v;
  return v;
}

//
// Return all the variables we are holding onto
//
vector<string> RooRealVarCache::GetAllVars(void) const
{
  vector<string> result;
  
  for(map<string,RooRealVar*>::const_iterator itr = _vars.begin(); itr != _vars.end(); itr++) {
    result.push_back(itr->first);
  }
  return result;
}

//
// Return the number of guys we know about
//
size_t RooRealVarCache::size(void) const
{
  return _vars.size();
}



