#include <ZZAnalysis/AnalysisStep/test/classlib/include/VotingMultiClassCombinator.h>

VotingMultiClassCombinator::VotingMultiClassCombinator()
{

}

VotingMultiClassCombinator::~VotingMultiClassCombinator()
{

}

// in this simple combinator, just look at all possible combinations of classes for which have discriminants available, and return the dictionary <category, number of wins against any other category>
std::map<TString, float> VotingMultiClassCombinator::Evaluate(Tree* in, DiscriminantCollection* coll)
{
    std::map<TString, float> retval;
    
    // the list of categories that have actual discriminants behind them
    std::vector<std::pair<TString, TString>> category_pairs = coll -> GetCategoryPairs();

    for(auto& category_pair: category_pairs)
    {
	// evaluate the current pair and get its likelihood ratio
	float LR = coll -> Evaluate(category_pair, in);

	//std::cout << category_pair.first << " / " << category_pair.second << " : " << LR << std::endl;
	
	if(LR > 1.0)
	{
	    retval[category_pair.first]++;
	}
	else
	{
	    retval[category_pair.second]++;
	}
    }

    last_result = retval;

    return retval;
}
