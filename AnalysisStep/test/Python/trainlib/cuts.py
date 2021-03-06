def no_cut(row):
    return True

def j2cut(row):
    if row["nCleanedJetsPt30"] >= 2:
        return True
    else:
        return False

def j1cut(row):
    if row["nCleanedJetsPt30"] == 1:
        return True
    else:
        return False

def j0cut(row):
    if row["nCleanedJetsPt30"] == 0:
        return True
    else:
        return False

def mZZ_cut(row):
    if row["ZZMass"] >= 105.0 and row["ZZMass"] < 140.0:
        return True
    else:
        return False

def countNeutrinos(row):
    number_neutrinos = 0
    associated_particles = row["LHEAssociatedParticleId"]
    
    if associated_particles is not None:
        for pdg_code in associated_particles:
            if abs(pdg_code) == 12 or abs(pdg_code) == 14 or abs(pdg_code) == 16:
                number_neutrinos += 1
                
    return number_neutrinos

def countAssocLeptons(row):
    if row["GenAssocLep1Id"] == 0 and row["GenAssocLep2Id"] == 0:
        return 0
    elif (row["GenAssocLep1Id"] != 0 and row["GenAssocLep2Id"] == 0) or (row["GenAssocLep1Id"] == 0 and row["GenAssocLep2Id"] != 0):
        return 1
    elif row["GenAssocLep1Id"] != 0 and row["GenAssocLep2Id"] != 0:
        return 2
    
    return 0


def WHhadr_cut(row):
    return countAssocLeptons(row) == 0

def ZHhadr_cut(row):
    return countAssocLeptons(row) == 0 and countNeutrinos(row) == 0

def WHlept_cut(row):
    return countAssocLeptons(row) == 1

def ZHlept_cut(row):
    return countAssocLeptons(row) == 2

def ZHMET_cut(row):
    return countNeutrinos(row) == 2

def ttHhadr_cut(row):
    return countAssocLeptons(row) == 0

def ttHlept_cut(row):
    return (countAssocLeptons(row) == 1 or countAssocLeptons(row) == 2)
