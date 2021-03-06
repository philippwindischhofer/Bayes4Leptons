#include <ZZAnalysis/AnalysisStep/test/classlib/include/Mor17Classifier.h>
#include <ZZAnalysis/AnalysisStep/interface/Category.h>
#include <ZZAnalysis/AnalysisStep/interface/Discriminants.h>

Mor17Classifier::Mor17Classifier()
{ }

Mor17Classifier::~Mor17Classifier()
{ }

int Mor17Classifier::ClassifyThisEvent(Tree* in)
{
    float jetQGLikelihood[99];
    float jetPhi[99];

    for(int i = 0; i < in -> nCleanedJetsPt30; i++)
    {
	jetQGLikelihood[i] = in -> JetQGLikelihood -> at(i);
	jetPhi[i] = in -> JetPhi -> at(i);
    }

    return categoryMor17(
	in -> nExtraLep,
	in -> nExtraZ,
	in -> nCleanedJetsPt30, 
	in -> nCleanedJetsPt30BTagged_bTagSF,
	jetQGLikelihood,
	in -> p_JJQCD_SIG_ghg2_1_JHUGen_JECNominal,
	in -> p_JQCD_SIG_ghg2_1_JHUGen_JECNominal,
	in -> p_JJVBF_SIG_ghv1_1_JHUGen_JECNominal,
	in -> p_JVBF_SIG_ghv1_1_JHUGen_JECNominal,
	in -> pAux_JVBF_SIG_ghv1_1_JHUGen_JECNominal,
	in -> p_HadWH_SIG_ghw1_1_JHUGen_JECNominal,
	in -> p_HadZH_SIG_ghz1_1_JHUGen_JECNominal,
	in -> p_HadWH_mavjj_JECNominal,
	in -> p_HadWH_mavjj_true_JECNominal,
	in -> p_HadZH_mavjj_JECNominal,
	in -> p_HadZH_mavjj_true_JECNominal,
	jetPhi,
	in -> ZZMass,
	in -> PFMET,
	true,
	false);
}
