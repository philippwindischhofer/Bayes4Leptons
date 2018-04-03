class Config:
    # the branches that are always going to be loaded and on which any preprocessor will act
    branches = ["PFMET", "nCleanedJetsPt30", "nCleanedJetsPt30BTagged_bTagSF", "nExtraLep", "ZZMass", "LHEAssociatedParticleId", "GenAssocLep1Id", "GenAssocLep2Id", 
                "D_VBF2j_ggH_ME", "D_VBF1j_ggH_ME", "D_WHh_ggH_ME", "D_ZHh_ggH_ME", "D_WHh_ZHh_ME", "D_VBF2j_WHh_ME", "D_VBF2j_ZHh_ME", "JetPt", "JetEta", "JetPhi", "LepPt", "LepEta", "LepPhi", "ExtraLepPt", "ExtraLepEta", "ExtraLepPhi", "nExtraZ", "Z1Mass", "Z2Mass", "Z1Pt", "Z2Pt"]
    MC_filename = "/ZZ4lAnalysis.root"

class TrainingConfig:
    def __init__(self, steps_per_epoch = 128, max_epochs = 5):
        self.steps_per_epoch = steps_per_epoch
        self.max_epochs = max_epochs
