#include <ZZAnalysis/AnalysisStep/test/Profiler/include/Profiler.h>

Profiler::Profiler():Tree()
{  }

Profiler::~Profiler()
{  }

void Profiler::FillProfile(TString input_file_name, float lumi, TH1F* hist, const std::function<bool(Tree*)>& cut, const std::function<float(Tree*)>& var)
{
    input_file = new TFile(input_file_name);
    
    std::cout << "reading from " << input_file_name << std::endl;
    
    // read some auxiliary information
    hCounters = (TH1F*)input_file -> Get("ZZTree/Counters");
    n_gen_events = (Long64_t)hCounters -> GetBinContent(1);
    gen_sum_weights = (Long64_t)hCounters -> GetBinContent(40);

    input_tree = (TTree*)input_file -> Get("ZZTree/candTree");

    Init(input_tree, input_file_name);
    
    if(fChain == 0) return;

    // total number of entries stored in the entire chain
    Long64_t n_entries = fChain -> GetEntriesFast();
    std::cout << "total number of entries = " << n_entries << std::endl;

    int fill_cnt = 0;

    // loop over the entries in chain
    for(Long64_t j_entry = 0; j_entry < n_entries; j_entry++)
    {
	// get the correct tree in the chain that contains this event
	Long64_t i_entry = LoadTree(j_entry);
	if(i_entry < 0) break;

	// now actually read this entry
	fChain -> GetEntry(j_entry);

	float event_weight = (lumi * xsec * 1000. * overallEventWeight) / gen_sum_weights;
	
	if(cut(this))
	{
	    hist -> Fill(var(this), event_weight);
	    fill_cnt ++;
	}
    }

    std::cout << "fill_cnt = " << fill_cnt << std::endl;
}
