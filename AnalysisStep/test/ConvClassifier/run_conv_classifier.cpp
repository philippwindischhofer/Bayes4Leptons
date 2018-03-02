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
#include "THStack.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TMath.h"

// My own files
#define Mor18

#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/ConvClassifier.h>
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/Classifier.h>
#include <ZZAnalysis/AnalysisStep/interface/Category.h>
#include <ZZAnalysis/AnalysisStep/test/Plotter_v2/src/setTDRStyle.cpp>
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/CatPlotter.h>
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/utils.h>
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/Tree.h>
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/histo_settings.h>

#ifdef Mor17
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/Mor17Classifier.h>
#endif

#ifdef Mor18
#include <ZZAnalysis/AnalysisStep/test/ConvClassifier/include/Mor18Classifier.h>
#endif

// global definitions
float lumi = 35.9f;

TString path = "/data_CMS/cms/tsculac/CJLST_NTuples/";
TString file_name = "/ZZ4lAnalysis.root";
TString out_folder("../../src/ZZAnalysis/ConvClassifierPlots/");

// cuts that are used to separate the individual signal contributions
int no_cut(Tree* in)
{
    return kTRUE;
}

int mZZ_cut(Tree* in)
{
    if((in -> ZZMass > 118) && (in -> ZZMass < 130))
	return kTRUE;
    else
	return kFALSE;
}

int countAssocLeptons(Tree* in)
{
    if((in -> GenAssocLep1Id == 0) && (in -> GenAssocLep2Id == 0))
    {
	return 0;
    }
    else if( ((in -> GenAssocLep1Id != 0) && (in -> GenAssocLep2Id == 0)) ||
	     ((in -> GenAssocLep1Id == 0) && (in -> GenAssocLep2Id != 0)) )
    {
	return 1;
    }
    else if((in -> GenAssocLep1Id != 0) && (in -> GenAssocLep2Id != 0))
    {
	return 2;
    }

    return 0;
}

int extraLeptons_0_cut(Tree* in)
{
    //if(mZZ_cut(in) && (in -> GenAssocLep1Id == 0) && (in -> GenAssocLep2Id == 0))
    if(mZZ_cut(in) && (countAssocLeptons(in) == 0))
	return kTRUE;
    else
	return kFALSE;
}

int extraLeptons_1_cut(Tree* in)
{
    //if(mZZ_cut(in) && (in -> GenAssocLep1Id != 0) && (in -> GenAssocLep2Id == 0))
    if(mZZ_cut(in) && (countAssocLeptons(in) == 1))
	return kTRUE;
    else
	return kFALSE;
}

int extraLeptons_2_cut(Tree* in)
{
//    if(mZZ_cut(in) && (in -> GenAssocLep1Id != 0) && (in -> GenAssocLep2Id != 0))
    if(mZZ_cut(in) && (countAssocLeptons(in) == 2))
	return kTRUE;
    else
	return kFALSE;
}

int countNeutrinos(Tree* in)
{
   int number_neutrinos = 0;

   if(in -> LHEAssociatedParticleId)
   {
       for(unsigned int i = 0; i < (in -> LHEAssociatedParticleId -> size()); i++)
       {
	   short pdg_code = in -> LHEAssociatedParticleId -> at(i);
	   if((TMath::Abs(pdg_code) == 12) || (TMath::Abs(pdg_code) == 14) || (TMath::Abs(pdg_code) == 16))
	       number_neutrinos++;
       }
   }

   return number_neutrinos;
}

int extraNeutrinos_0_cut(Tree* in)
{
    if(mZZ_cut(in) && countNeutrinos(in) == 0)
	return kTRUE;
    else
	return kFALSE;
}

int extraNeutrinos_2_cut(Tree* in)
{
    if(mZZ_cut(in) && countNeutrinos(in) == 2)
	return kTRUE;
    else
	return kFALSE;
}

int extraNeutrinos_0_Leptons_0_cut(Tree* in)
{
    if(mZZ_cut(in) && (countAssocLeptons(in) == 0) && (countNeutrinos(in) == 0))
	return kTRUE;
    else
	return kFALSE;
}

// generates a vector of histograms corresponding to the signals
std::vector<TH1F*> generate_signal_histvec(int fill_histos, Classifier* classifier)
{
    TString hist_storage("histograms_signal.root");
    std::vector<TString> signal_path;

    for(unsigned int i = 0; i < signal_file_names.size(); i++)
    {
	signal_path.push_back(path + signal_file_names[i] + file_name);
    }
	
    std::vector<TH1F*> hist_vec(signal_hist_names.size());

    for(unsigned int i = 0; i < signal_hist_names.size(); i++)
    {
	hist_vec[i] = new TH1F(signal_hist_names[i], signal_hist_names[i], cat_labels.size(), -0.5, cat_labels.size() - 0.5);
	hist_vec[i] -> SetFillColor(signal_source_colors[i]);
	hist_vec[i] -> SetLineColor(signal_source_colors[i]);
	hist_vec[i] -> SetFillStyle(1001);
    }    

    if(fill_histos)
    {	
	std::cout << "filling histograms" << std::endl;
//	Mor17Classifier* testclass = new Mor17Classifier();
	// for the signal files, can be very specific and select precisely the correct differential channel
	// ideally, create one separate histogram (by using generator-level cuts) for each signal category. Then, can compute the efficiency for this signal to end up in the matching signal category
	classifier -> FillHistogram(signal_path[0], lumi, hist_vec[ggHhist], mZZ_cut);
	classifier -> FillHistogram(signal_path[1], lumi, hist_vec[VBFhist], mZZ_cut);
	classifier -> FillHistogram(signal_path[2], lumi, hist_vec[WHXhist], extraLeptons_0_cut);
	classifier -> FillHistogram(signal_path[2], lumi, hist_vec[WHlnuhist], extraLeptons_1_cut); // W decays leptonically
	classifier -> FillHistogram(signal_path[3], lumi, hist_vec[WHXhist], extraLeptons_0_cut);
	classifier -> FillHistogram(signal_path[3], lumi, hist_vec[WHlnuhist], extraLeptons_1_cut); // W decays leptonically
	classifier -> FillHistogram(signal_path[4], lumi, hist_vec[ZHXhist], extraNeutrinos_0_Leptons_0_cut); // no extra leptons (electrons, muons) and neutrinos for the hadronic decay of the Z
	classifier -> FillHistogram(signal_path[4], lumi, hist_vec[ZHnunuhist], extraNeutrinos_2_cut);
	classifier -> FillHistogram(signal_path[4], lumi, hist_vec[ZH2lhist], extraLeptons_2_cut); // need exactly two extra leptons to have the Z decaying leptonically
	classifier -> FillHistogram(signal_path[5], lumi, hist_vec[ttH0lhist], extraLeptons_0_cut);    
	classifier -> FillHistogram(signal_path[5], lumi, hist_vec[ttH1lhist], extraLeptons_1_cut);
	classifier -> FillHistogram(signal_path[5], lumi, hist_vec[ttH2lhist], extraLeptons_2_cut);    
	std::cout << "end filling histograms" << std::endl;

	save_histos(out_folder + STORAGE_PREFIX + hist_storage, hist_vec);
    }    

    // read the histograms back from the root file
    hist_vec = read_histos(out_folder + STORAGE_PREFIX + hist_storage, signal_hist_names);

    return hist_vec;
}

std::vector<TH1F*> generate_background_histvec(int fill_histos, Classifier* classifier)
{
    TString hist_storage("histograms_background.root");
    std::vector<TString> background_path;

    for(unsigned int i = 0; i < background_file_names.size(); i++)
    {
	background_path.push_back(path + background_file_names[i] + file_name);
    }

    std::vector<TH1F*> hist_vec(background_path.size());

    for(unsigned int i = 0; i < background_hist_names.size(); i++)
    {
	hist_vec[i] = new TH1F(background_hist_names[i], background_hist_names[i], cat_labels.size(), -0.5, cat_labels.size() - 0.5);
	hist_vec[i] -> SetFillStyle(3244);
	hist_vec[i] -> SetLineColor(background_source_colors[i]);
	hist_vec[i] -> SetFillColor(background_source_colors[i]);
    }

    if(fill_histos)
    {	
	// No need to fill the histograms every time!
	std::cout << "filling background histograms" << std::endl;
//	Mor17Classifier* testclass = new Mor17Classifier();
	
	// for the background files, don't have any requirements for many categories nor for any additional cuts, they just get summed up anyways later (keep the file-induced categorization anyways)
	for(unsigned int i = 0; i < background_path.size(); i++)
	{
	    std::cout << "reading from " << background_path[i] << std::endl;
	    classifier -> FillHistogram(background_path[i], lumi, hist_vec[i], mZZ_cut);
	}

	std::cout << "end filling background histograms" << std::endl;

	save_histos(out_folder + STORAGE_PREFIX + hist_storage, hist_vec);	
    }    

    // read the histograms back from the root file
    hist_vec = read_histos(out_folder + STORAGE_PREFIX + hist_storage, background_hist_names);

    return hist_vec;
}

void make_S_purity(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> hist_vec;

    hist_vec = generate_signal_histvec(fill_histos, classifier);

    CatPlotter plotter;
    
    // make the global purity plots
    // plot the total, absolute numbers of expected events in each category
    plotter.Construct(hist_vec, cat_labels, signal_source_labels, std::vector<float>(), "exp. events", lumi);
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "categorization_S_absolute.pdf");

    // plot the renormalized version
    std::vector<float> sums_signal_only = renormalize_histograms(hist_vec);
    plotter.Construct(hist_vec, cat_labels, signal_source_labels, sums_signal_only, "signal fraction", lumi);        
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "categorization_S.pdf");

    float total_events = 0;
    for(unsigned int i = 0; i < sums_signal_only.size(); i++)
    {
	total_events += sums_signal_only[i];
    }

    std::cout << "S_purity: inclusive events = " << total_events << std::endl;
}

void make_SB_purity(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> signal_hist_vec = generate_signal_histvec(fill_histos, classifier);
    std::vector<TH1F*> background_hist_vec = generate_background_histvec(fill_histos, classifier);
    
    std::vector<TH1F*> hist_vec(signal_hist_vec);
    hist_vec.insert(hist_vec.end(), background_hist_vec.begin(), background_hist_vec.end());

    std::vector<TString> source_labels(signal_source_labels);
    source_labels.insert(source_labels.end(), background_source_labels.begin(), background_source_labels.end());

    CatPlotter plotter;

    plotter.Construct(hist_vec, cat_labels, source_labels, std::vector<float>(), "exp. events", lumi);
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "categorization_SB_absolute.pdf");
    
    std::vector<float> sums_SB = renormalize_histograms(hist_vec);
    plotter.Construct(hist_vec, cat_labels, source_labels, sums_SB, "signal fraction", lumi);        
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "categorization_SB.pdf");
}

// this treats ALL higgs events as signals, even if their production mode does not agree with the category they have been placed into
void make_SB_ratio(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> signal_hist_vec = generate_signal_histvec(fill_histos, classifier);
    std::vector<TH1F*> background_hist_vec = generate_background_histvec(fill_histos, classifier);

    // generate the vector holding both signal- and background histograms: this makes sure to get the *total* background in each category: the unwanted signal contributions + the actual background
    std::vector<float> S_sums = get_category_sums(signal_hist_vec);

    std::vector<TH1F*> hist_vec(signal_hist_vec);
    hist_vec.insert(hist_vec.end(), background_hist_vec.begin(), background_hist_vec.end());
    std::vector<float> SB_sums = get_category_sums(hist_vec);

    float total_events = 0;
    for(unsigned int i = 0; i < SB_sums.size(); i++)
    {
	total_events += SB_sums[i];
    }

    std::cout << "SB_ratio: inclusive events = " << total_events << std::endl;

    // make the plot of the punzi purity
    std::vector<TH1F*> SB_hist_vec = {new TH1F("SB", "SB", cat_labels.size(), -0.5, cat_labels.size() - 0.5)};

    for(int i = 0; i < SB_hist_vec[0] -> GetSize() - 2; i++)
    {
	SB_hist_vec[0] -> Fill(i, S_sums[i] / SB_sums[i]);
    }

    CatPlotter plotter;

    plotter.Construct(SB_hist_vec, cat_labels, std::vector<TString>(), std::vector<float>(), "S / (S + B)", lumi);       
    plotter.GetStack() -> SetMaximum(1.0);
    plotter.Update();
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "SB.pdf");
}

// is more refined and really takes ONLY those events as signals, whose production mode also agrees with the category
void make_SBfine_ratio(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> signal_hist_vec = generate_signal_histvec(fill_histos, classifier);
    std::vector<TH1F*> background_hist_vec = generate_background_histvec(fill_histos, classifier);
   
    // the correctly classified events:
    // note that this routes 
    //      WH, W -> X    and ZH, Z -> X   into VHhadr (X ... any hadronic decay)
    //      ZH, Z -> nunu                  into VHMET
    //      WH, W -> l nu and ZH, Z -> 2 l into VHlept
    //      ggH                            into untagged
    //      VBF                            into VBF1jet and VBF2jet
    //      ttH + 0,1,2 l                  into ttH
    float VBF1j_correct_events = signal_hist_vec[VBFhist] -> GetBinContent(VBF1jet_bin);
    float VBF2j_correct_events = signal_hist_vec[VBFhist] -> GetBinContent(VBF2jet_bin);

    float VHlept_correct_events = signal_hist_vec[WHlnuhist] -> GetBinContent(VHleptonic_bin);
    VHlept_correct_events += signal_hist_vec[ZH2lhist] -> GetBinContent(VHleptonic_bin);

    float VHhadr_correct_events = signal_hist_vec[WHXhist] -> GetBinContent(VHhadronic_bin);
    VHhadr_correct_events += signal_hist_vec[ZHXhist] -> GetBinContent(VHhadronic_bin);

    float VHMET_correct_events = signal_hist_vec[ZHnunuhist] -> GetBinContent(VHMET_bin);

#ifdef Mor17
    float ttH_correct_events = signal_hist_vec[ttH0lhist] -> GetBinContent(ttH_bin);
    ttH_correct_events += signal_hist_vec[ttH1lhist] -> GetBinContent(ttH_bin);
    ttH_correct_events += signal_hist_vec[ttH2lhist] -> GetBinContent(ttH_bin);
#endif

#ifdef Mor18
    float ttH_hadronic_correct_events = signal_hist_vec[ttH0lhist] -> GetBinContent(ttH_hadronic_bin);
    float ttH_leptonic_correct_events = signal_hist_vec[ttH1lhist] -> GetBinContent(ttH_leptonic_bin);
    ttH_leptonic_correct_events += signal_hist_vec[ttH2lhist] -> GetBinContent(ttH_leptonic_bin);
#endif

    float untagged_correct_events = signal_hist_vec[ggHhist] -> GetBinContent(untagged_bin);

    // generate the vector holding both signal- and background histograms: this makes sure to get the *total* background in each category: the unwanted signal contributions + the actual background
    std::vector<TH1F*> hist_vec(signal_hist_vec);
    hist_vec.insert(hist_vec.end(), background_hist_vec.begin(), background_hist_vec.end());
    std::vector<float> sums = get_category_sums(hist_vec);
    
    std::vector<TH1F*> SB_hist_vec = {new TH1F("SB_fine", "SB_fine", cat_labels.size(), -0.5, cat_labels.size() - 0.5)};

    SB_hist_vec[0] -> Fill(untagged_bin - 1, untagged_correct_events / sums[untagged_bin - 1]);
    SB_hist_vec[0] -> Fill(VBF1jet_bin - 1, VBF1j_correct_events / sums[VBF1jet_bin - 1]);
    SB_hist_vec[0] -> Fill(VBF2jet_bin - 1, VBF2j_correct_events / sums[VBF2jet_bin - 1]);
    SB_hist_vec[0] -> Fill(VHhadronic_bin - 1, VHhadr_correct_events / sums[VHhadronic_bin - 1]);
    SB_hist_vec[0] -> Fill(VHleptonic_bin - 1, VHlept_correct_events / sums[VHleptonic_bin - 1]);
    SB_hist_vec[0] -> Fill(VHMET_bin - 1, VHMET_correct_events / sums[VHMET_bin - 1]);

#ifdef Mor17
    SB_hist_vec[0] -> Fill(ttH_bin - 1, ttH_correct_events / sums[ttH_bin - 1]);
#endif

#ifdef Mor18
    SB_hist_vec[0] -> Fill(ttH_leptonic_bin - 1, ttH_leptonic_correct_events / sums[ttH_leptonic_bin - 1]);
    SB_hist_vec[0] -> Fill(ttH_hadronic_bin - 1, ttH_hadronic_correct_events / sums[ttH_hadronic_bin - 1]);
#endif

    CatPlotter plotter;

    plotter.Construct(SB_hist_vec, cat_labels, std::vector<TString>(), std::vector<float>(), "S_{fine} / (S_{fine} + B)", lumi);       
    plotter.GetStack() -> SetMaximum(1.0);
    plotter.Update();
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "SB_fine.pdf");
}

void make_punzi(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> signal_hist_vec = generate_signal_histvec(fill_histos, classifier);
    std::vector<TH1F*> background_hist_vec = generate_background_histvec(fill_histos, classifier);
   
    // the total number of events from each category
    float VBF_events = get_total_events(signal_hist_vec[VBFhist]); // this is the sum of all VBF events (i.e. both VBF 1-jet and VBF 2-jet events)

    float VHhadr_events = get_total_events(signal_hist_vec[WHXhist]);
    VHhadr_events += get_total_events(signal_hist_vec[ZHXhist]);

    float VHlept_events = get_total_events(signal_hist_vec[WHlnuhist]);
    VHlept_events += get_total_events(signal_hist_vec[ZH2lhist]);

#ifdef Mor17
    float ttH_events = get_total_events(signal_hist_vec[ttH0lhist]);
    ttH_events += get_total_events(signal_hist_vec[ttH1lhist]);
    ttH_events += get_total_events(signal_hist_vec[ttH2lhist]);
#endif

#ifdef Mor18
    float ttH_hadronic_events = get_total_events(signal_hist_vec[ttH0lhist]);
    float ttH_leptonic_events = get_total_events(signal_hist_vec[ttH1lhist]);
    ttH_leptonic_events += get_total_events(signal_hist_vec[ttH2lhist]);
#endif

    float VHMET_events = get_total_events(signal_hist_vec[ZHnunuhist]);

    float untagged_events = get_total_events(signal_hist_vec[ggHhist]);

    // the correctly classified events:
    // note that this routes 
    //      WH, W -> X    and ZH, Z -> X   into VHhadr
    //      WH, W -> l nu and ZH, Z -> 2 l into VHlept
    //      ggH                            into untagged
    //      VBF                            into VBF1jet and VBF2jet
    //      ttH + 0,1,2 l                  into ttH
    float VBF1j_correct_events = signal_hist_vec[VBFhist] -> GetBinContent(VBF1jet_bin);
    float VBF2j_correct_events = signal_hist_vec[VBFhist] -> GetBinContent(VBF2jet_bin);

    float VHlept_correct_events = signal_hist_vec[WHlnuhist] -> GetBinContent(VHleptonic_bin);
    VHlept_correct_events += signal_hist_vec[ZH2lhist] -> GetBinContent(VHleptonic_bin);

    float VHhadr_correct_events = signal_hist_vec[WHXhist] -> GetBinContent(VHhadronic_bin);
    VHhadr_correct_events += signal_hist_vec[ZHXhist] -> GetBinContent(VHhadronic_bin);

#ifdef Mor17
    Float ttH_correct_events = signal_hist_vec[ttH0lhist] -> GetBinContent(ttH_bin);
    ttH_correct_events += signal_hist_vec[ttH1lhist] -> GetBinContent(ttH_bin);
    ttH_correct_events += signal_hist_vec[ttH2lhist] -> GetBinContent(ttH_bin);
#endif

#ifdef Mor18
    float ttH_hadronic_correct_events = signal_hist_vec[ttH0lhist] -> GetBinContent(ttH_hadronic_bin);
    float ttH_leptonic_correct_events = signal_hist_vec[ttH1lhist] -> GetBinContent(ttH_leptonic_bin);
    ttH_leptonic_correct_events += signal_hist_vec[ttH2lhist] -> GetBinContent(ttH_leptonic_bin);
#endif

    float VHMET_correct_events = signal_hist_vec[ZHnunuhist] -> GetBinContent(VHMET_bin);

    float untagged_correct_events = signal_hist_vec[ggHhist] -> GetBinContent(untagged_bin);

    // generate the vector holding both signal- and background histograms: this makes sure to get the *total* background in each category: the unwanted signal contributions + the actual background
    std::vector<TH1F*> hist_vec(signal_hist_vec);
    hist_vec.insert(hist_vec.end(), background_hist_vec.begin(), background_hist_vec.end());
    std::vector<float> sums = get_category_sums(hist_vec);

    float untagged_punzi = get_punzi_purity(
	sums[untagged_bin - 1] - untagged_correct_events, // background in this category
	untagged_correct_events, // signal in this category (but spread over multiple categories)
	untagged_events, // total signal (important for computing the efficiency)
	5, 2);

    float VBF1j_punzi = get_punzi_purity(
	sums[VBF1jet_bin - 1] - VBF1j_correct_events,
	VBF1j_correct_events,
	VBF_events,
	5, 2);

    float VBF2j_punzi = get_punzi_purity(
	sums[VBF2jet_bin - 1] - VBF2j_correct_events,
	VBF2j_correct_events,
	VBF_events,
	5, 2);

#ifdef Mor17
    float ttH_punzi = get_punzi_purity(
	sums[ttH_bin - 1] - ttH_correct_events,
	ttH_correct_events,
	ttH_events,
	5, 2);
#endif Mor17

#ifdef Mor18
    float ttH_leptonic_punzi = get_punzi_purity(
	sums[ttH_leptonic_bin - 1] - ttH_leptonic_correct_events,
	ttH_leptonic_correct_events,
	ttH_leptonic_events,
	5, 2);

    float ttH_hadronic_punzi = get_punzi_purity(
	sums[ttH_hadronic_bin - 1] - ttH_hadronic_correct_events,
	ttH_hadronic_correct_events,
	ttH_hadronic_events,
	5, 2);
#endif Mor18

    float VHhadr_punzi = get_punzi_purity(
	sums[VHhadronic_bin - 1] - VHhadr_correct_events,
	VHhadr_correct_events,
	VHhadr_events,
	5, 2);

    float VHlept_punzi = get_punzi_purity(
	sums[VHleptonic_bin - 1] - VHlept_correct_events,
	VHlept_correct_events,
	VHlept_events,
	5, 2);

    float VHMET_punzi = get_punzi_purity(
	sums[VHMET_bin - 1] - VHMET_correct_events,
	VHMET_correct_events,
	VHMET_events,
	5, 2);
    
    // make the plot of the punzi purity
    std::vector<TH1F*> punzi_hist_vec = {new TH1F("punzi_purity", "punzi_purity", cat_labels.size(), -0.5, cat_labels.size() - 0.5)};

    punzi_hist_vec[0] -> Fill(untagged_bin - 1, untagged_punzi);
    punzi_hist_vec[0] -> Fill(VBF1jet_bin - 1, VBF1j_punzi);
    punzi_hist_vec[0] -> Fill(VBF2jet_bin - 1, VBF2j_punzi);
    punzi_hist_vec[0] -> Fill(VHhadronic_bin - 1, VHhadr_punzi);
    punzi_hist_vec[0] -> Fill(VHleptonic_bin - 1, VHlept_punzi);
    punzi_hist_vec[0] -> Fill(VHMET_bin - 1, VHMET_punzi);
    
#ifdef Mor17
    punzi_hist_vec[0] -> Fill(ttH_bin - 1, ttH_punzi);
#endif

#ifdef Mor18
    punzi_hist_vec[0] -> Fill(ttH_leptonic_bin - 1, ttH_leptonic_punzi);
    punzi_hist_vec[0] -> Fill(ttH_hadronic_bin - 1, ttH_hadronic_punzi);
#endif

    CatPlotter plotter;
    plotter.Construct(punzi_hist_vec, cat_labels, std::vector<TString>(), std::vector<float>(), "Punzi purity", lumi);   
    plotter.SaveAs(out_folder + STORAGE_PREFIX + "punzi.pdf");
}

void print_yield_table(std::vector<TH1F*> hist_vec, std::vector<TString> col_labels, std::vector<TString> source_labels)
{
    std::vector<float> sums = get_category_sums(hist_vec);

    // table header = category labels
    std::cout << "\t\t";
    for(auto cat = 0; cat < hist_vec[0] -> GetSize() - 2; cat++)
    {
	std::cout << col_labels[cat] << "\t";
    }

    std::cout << std::endl;

    int hist_nr = 0;
    for(auto& hist : hist_vec)
    {
	std::cout << source_labels[hist_nr] << "\t\t";
	hist_nr++;

	for(auto cat = 1; cat < hist -> GetSize() - 1; cat++)
	{
	    std::cout << hist -> GetBinContent(cat) << "\t";
	}
	std::cout << std::endl;
    }

    std::cout << "sum" << "\t\t";

    for(auto& sum : sums)
    {
	std::cout << sum << "\t";
    }

    std::cout << std::endl;
}

void print_signal_yield_table(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> signal_hist_vec = generate_signal_histvec(fill_histos, classifier);

    print_yield_table(signal_hist_vec, cat_labels_text, signal_source_labels_text);
}


void print_background_yield_table(int fill_histos, Classifier* classifier)
{
    std::vector<TH1F*> background_hist_vec = generate_background_histvec(fill_histos, classifier);

    print_yield_table(background_hist_vec, cat_labels_text, background_source_labels_text);
}

int main( int argc, char *argv[] )
{
#ifdef Mor17
    Classifier* refclass = new Mor17Classifier();
#endif

#ifdef Mor18
    Classifier* refclass = new Mor18Classifier();
#endif

    make_SB_purity(kFALSE, refclass);
    make_S_purity(kFALSE, refclass);
    make_punzi(kFALSE, refclass);
    make_SB_ratio(kFALSE, refclass);
    make_SBfine_ratio(kFALSE, refclass);

    print_signal_yield_table(kFALSE, refclass);
    print_background_yield_table(kFALSE, refclass);
    return(0);
}
