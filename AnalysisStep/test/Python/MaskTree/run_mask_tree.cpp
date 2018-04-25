// C++
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/range/combine.hpp>

// ROOT
#include "TApplication.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TMultiGraph.h"

#include <ZZAnalysis/AnalysisStep/test/classlib/include/Mor18Config.h>
#include <ZZAnalysis/AnalysisStep/test/classlib/include/me_discriminants.h>

void augment_tree(TString inpath, TString outpath)
{
    TFile* input_file = new TFile(inpath, "read");

    // read the metadata
    TH1F* hCounters = (TH1F*)input_file -> Get("ZZTree/Counters");
    Long64_t n_gen_events = (Long64_t)hCounters -> GetBinContent(1);
    Long64_t gen_sum_weights = (Long64_t)hCounters -> GetBinContent(40);

    TFile* output_file = new TFile(outpath, "recreate");
    output_file -> mkdir("ZZTree");

    // read some auxiliary information
    TH1F* input_metadata = (TH1F*)input_file -> Get("ZZTree/Counters");
    TTree* input_tree = (TTree*)input_file -> Get("ZZTree/candTree");

    Tree* buffer = new Tree();
    buffer -> Init(input_tree, inpath);
    Long64_t n_entries = buffer -> fChain -> GetEntriesFast();

    TTree* output_tree = (TTree*)(input_tree -> CloneTree(0));
    TH1F* output_metadata = (TH1F*)(input_metadata -> Clone());

    // loop over the entries in chain
    for(Long64_t j_entry = 0; j_entry < n_entries; j_entry++)
    {
	// get the correct tree in the chain that contains this event
	Long64_t i_entry = buffer -> LoadTree(j_entry);
	if(i_entry < 0) break;

	// now actually read this entry
	buffer -> fChain -> GetEntry(j_entry);

	// fill the variables holding the ME discriminants (for the classes of events for which they actually make sense)
	buffer -> D_VBF2j_ggH_ME = buffer -> nCleanedJetsPt30 >= 2 ? DVBF2j_ME_disc(buffer) : 0.0;
	buffer -> D_VBF1j_ggH_ME = buffer -> nCleanedJetsPt30 == 1 ? DVBF1j_ME_disc(buffer) : 0.0;
	buffer -> D_WHh_ggH_ME = buffer -> nCleanedJetsPt30 >= 2 ? DWHh_ME_disc(buffer) : 0.0;
	buffer -> D_ZHh_ggH_ME = buffer -> nCleanedJetsPt30 >= 2 ? DZHh_ME_disc(buffer) : 0.0;
	buffer -> D_WHh_ZHh_ME = buffer -> nCleanedJetsPt30 >= 2 ? DWHZH_ME_disc(buffer) : 0.0;
	buffer -> D_VBF2j_WHh_ME = buffer -> nCleanedJetsPt30 >= 2 ? DVBFWH_ME_disc(buffer) : 0.0;
	buffer -> D_VBF2j_ZHh_ME = buffer -> nCleanedJetsPt30 >= 2 ? DVBFZH_ME_disc(buffer) : 0.0;

	output_tree -> Fill();
    }

    output_file -> cd("ZZTree");
    output_tree -> Write();
    output_metadata -> Write();

    output_file -> Close();
    input_file -> Close();
}

int main( int argc, char *argv[] )
{
    Mor18Config* conf = new Mor18Config("/data_CMS/cms/wind/CJLST_NTuples/", 35.9, true);
    TString out_dir = "/data_CMS/cms/wind/CJLST_NTuples_masked/";
    
    // first, make a list of all files, independent of the cut with which they appear in the signal routing table
    std::vector<std::pair<TString, Routing*>> routing_table = conf -> get_routing();
    std::vector<TString> infile_paths;
    std::vector<TString> outfile_paths;

    for(auto& routing: routing_table)
    {
	TString cur_file = routing.first;
	if(std::find(infile_paths.begin(), infile_paths.end(), conf -> MC_path() + cur_file + conf -> MC_filename()) == infile_paths.end())
	{
	    infile_paths.push_back(conf -> MC_path() + cur_file + conf -> MC_filename());
	    outfile_paths.push_back(out_dir + cur_file + conf -> MC_filename());

	    std::cout << "added " << conf -> MC_path() + cur_file + conf -> MC_filename() << " to queue" << std::endl;
	}
    }

    for(auto tup: boost::combine(infile_paths, outfile_paths))
    {
	TString infile;
	TString outfile;
	boost::tie(infile, outfile) = tup;

	std::cout << "now augmenting " << infile << " ==> " << outfile << std::endl;

	augment_tree(infile, outfile);
    }

    return(0);
}
