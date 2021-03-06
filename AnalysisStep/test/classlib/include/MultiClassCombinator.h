#ifndef MultiClassCombinator_h
#define MultiClassCombinator_h

// C++
#include <iostream>
#include <fstream>
#include <string>

// ROOT
#include "TApplication.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TKDE.h"
#include "TSpline.h"
#include "TF1.h"

#include <ZZAnalysis/AnalysisStep/test/classlib/include/Tree.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/DiscriminantCollection.h>

class MultiClassCombinator
{
public:
    MultiClassCombinator();
    ~MultiClassCombinator();

    virtual std::map<TString, float> Evaluate(Tree* in, DiscriminantCollection* coll);

    virtual TString GetWinningCategory();
    virtual std::vector<TString> GetWinningCategories();
    virtual float GetWinningMargin();

    virtual void SetParameter(TString parameter_name, float parameter_value){engine_parameters[parameter_name] = parameter_value;};

protected:
    std::map<TString, float> last_result;
    std::map<TString, float> engine_parameters;

private:
    TString find_max(std::map<TString, float> map);
};

#endif
