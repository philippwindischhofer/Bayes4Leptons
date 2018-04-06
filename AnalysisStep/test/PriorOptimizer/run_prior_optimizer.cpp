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

#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/GSLSimAnMinimizer.h"

// use the local style for the histograms
#include <ZZAnalysis/AnalysisStep/test/classlib/include/PlottingUtils.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/Mor18LIClassifier.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/CatPlotter.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/utils.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/Tree.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/cuts.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/CompUtils.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/Config.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/Mor18Config.h>

#include <ZZAnalysis/AnalysisStep/interface/Category.h>

TString punzi_hist_name = "punzi_purity";
TString punzi_infile = "punzi_plot_hist.root";
TString punzi_outfile = "punzi_comp";

// where all the files from the reference classifier are stored
// TString refdir = "/data_CMS/cms/wind/180404_input_parameter_sweep_full_category_set_KL_test/input_columns_5/benchmark/";
// TString outdir = "/data_CMS/cms/wind/180405_prior_optimizer/";

TString refdir;
TString outdir;

int evalcnt = 0;

// build the classifier whose parameters are going to be optimized
Classifier* varclass;

Mor18LIClassifier* varclass18;
Mor18Config* conf;

// evaluates the cost when using certain values for the working points
double costfunc(const double* params)
{
    // set the default values for the WPs (these are the same that are used in automatic mode *without* QG tagging)
    float VBF_prior = (float)params[0];
    float ggH_prior = (float)params[1];
    float WHhadr_prior = (float)params[2];
    float ZHhadr_prior = (float)params[3];
    float WHlept_prior = (float)params[4];
    float ZHlept_prior = (float)params[5];
    float ZHMET_prior = (float)params[6];
    float ttHhadr_prior = (float)params[7];
    float ttHlept_prior = (float)params[8];

    varclass18 -> SetPriors(VBF_prior, ggH_prior, WHhadr_prior, ZHhadr_prior, WHlept_prior, ZHlept_prior, ZHMET_prior, ttHhadr_prior, ttHlept_prior);

    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "evaluation = " << evalcnt << std::endl;

    std::cout << "VBF_prior = " << VBF_prior << std::endl;
    std::cout << "ggH_prior = " << ggH_prior << std::endl;
    std::cout << "WHhadr_prior = " << WHhadr_prior << std::endl;
    std::cout << "ZHhadr_prior = " << ZHhadr_prior << std::endl;
    std::cout << "WHlept_prior = " << WHlept_prior << std::endl;
    std::cout << "ZHlept_prior = " << ZHlept_prior << std::endl;
    std::cout << "ZHMET_prior = " << ZHMET_prior << std::endl;
    std::cout << "ttHhadr_prior = " << ttHhadr_prior << std::endl;
    std::cout << "ttHlept_prior = " << ttHlept_prior << std::endl;

    // evaluate the Punzi value with this (modified) Classifier now
    PlottingUtils::make_punzi(kTRUE, varclass, outdir, "punzi", "no_cut_data", no_cut, conf);
    
    // load low the Punzi histogram of the optimized classifier and compare the two. From this point onwards, is exactly the same as in "Comp"
    float zoom_scale = 1.0;
    float cost = CompUtils::compare_punzi(outdir, refdir, "Mor18_{opt}", "Mor18", conf -> storage_prefix() + punzi_infile, punzi_hist_name, outdir, conf -> storage_prefix() + punzi_outfile + Form("%i", evalcnt), zoom_scale, conf);

    std::cout << "cost = " << cost << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;

    evalcnt++;

    return cost;
}

int main( int argc, char *argv[] )
{
    if(argc != 3)
    {
	std::cerr << "Error: exactly 2 arguments are required" << std::endl;
    }

    TString run_dir = argv[1];
    outdir = argv[2];

    std::cout << outdir << std::endl;
    
    refdir = run_dir + "benchmark/";

    varclass = new Mor18LIClassifier(run_dir + "calibration/");

    varclass18 = static_cast<Mor18LIClassifier*>(varclass);

    conf = new Mor18Config(run_dir + "augmentation/", 35.9, false);

    // start the optimization with the currently used values
    float VBF_prior_init = 1.0;
    float ggH_prior_init = 1.772352900118657;
    float WHhadr_prior_init = 0.5508480491379912;
    float ZHhadr_prior_init = 0.21093041111188665;
    float WHlept_prior_init = 0.2727716898164305;
    float ZHlept_prior_init = 0.063341941788232;
    float ZHMET_prior_init = 0.06669225937041949;
    float ttHhadr_prior_init = 0.14521532770293852;
    float ttHlept_prior_init = 0.19540029315278845;

    float var[9] = {VBF_prior_init, ggH_prior_init, WHhadr_prior_init, ZHhadr_prior_init, WHlept_prior_init, ZHlept_prior_init, ZHMET_prior_init, ttHhadr_prior_init, ttHlept_prior_init};
    float step[9] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};

    //ROOT::Math::Minimizer* min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "");
    ROOT::Math::Minimizer* min = new ROOT::Math::GSLSimAnMinimizer();
    min -> SetMaxFunctionCalls(500);
    min -> SetMaxIterations(500);
    min -> SetTolerance(0.1);
    min -> SetPrintLevel(1);

    ROOT::Math::Functor f(&costfunc, 4);
    min -> SetFunction(f);

    //min -> SetLimitedVariable(0, "VBF_prior", var[0], step[0], 0.0, 1.0);
    min -> SetFixedVariable(0, "VBF_prior", VBF_prior_init);
    min -> SetLimitedVariable(1, "ggH_prior", var[1], step[1], 1.0, 4.0);
    min -> SetLimitedVariable(2, "WHhadr_prior", var[2], step[2], 0.0, 1.0);
    min -> SetLimitedVariable(3, "ZHhadr_prior", var[3], step[3], 0.0, 1.0);
    min -> SetLimitedVariable(4, "WHlept_prior", var[4], step[4], 0.0, 1.0);
    min -> SetLimitedVariable(5, "ZHlept_prior", var[5], step[5], 0.0, 1.0);
    min -> SetLimitedVariable(6, "ZHMET_prior", var[6], step[6], 0.0, 1.0);
    min -> SetLimitedVariable(7, "ttHhadr_prior", var[7], step[7], 0.0, 1.0);
    min -> SetLimitedVariable(8, "ttHlept_prior", var[8], step[8], 0.0, 1.0);

    std::cout << "starting minimization" << std::endl;

    min -> Minimize();

    // these are the final parameters
    const double* res = min -> X();

    std::cout << "-------------------------------------------------------" << std::endl;    
    std::cout << "optimization finished!" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;    
    std::cout << "final parameter values: " << std::endl;

    std::cout << "VBF_prior = " << res[0] << std::endl;
    std::cout << "ggH_prior = " << res[1] << std::endl;
    std::cout << "WHhadr_prior = " << res[2] << std::endl;
    std::cout << "ZHhadr_prior = " << res[3] << std::endl;
    std::cout << "WHlept_prior = " << res[4] << std::endl;
    std::cout << "ZHlept_prior = " << res[5] << std::endl;
    std::cout << "ZHMET_prior = " << res[6] << std::endl;
    std::cout << "ttHhadr_prior = " << res[7] << std::endl;
    std::cout << "ttHlept_prior = " << res[8] << std::endl;

    std::cout << "-------------------------------------------------------" << std::endl;        

    return(0);
}
