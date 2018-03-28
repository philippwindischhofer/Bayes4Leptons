import pandas as pd
import numpy as np
from trainlib.Preprocessor import Preprocessor
from trainlib.config import Config
from sklearn.decomposition import PCA
import pickle

class PCAWhiteningPreprocessor(Preprocessor):
    
    # requires the name of the data columns that should be whitened in the end
    def __init__(self, name, processed_columns, cuts):
        self.name = name
        self.processed_columns = processed_columns
        self.outcol_names = ["PCA_w_" + str(i) for i in range(len(self.processed_columns))]
        self.cuts = cuts
        n_inputs = len(processed_columns)
        self.pca = PCA(n_components = n_inputs, svd_solver = 'auto', whiten = True)

        self.last_indices = None

    def setup_generator(self, datagen, len_setupdata):
        self.len_setupdata = len_setupdata

        # draw some data from the generator to find the eigenvectors
        extracted_data = []
        extracted_rows = 0

        # the raw-scrambled generator gives a single Pandas table, containing the data
        for data in datagen:
            extracted_data.append(data)
            extracted_rows += len(data)

            if extracted_rows > self.len_setupdata:
                break

        print "setting up PCA whitening on " + str(extracted_rows) + " events"

        input_data = pd.concat(extracted_data)
        input_data = input_data.reset_index(drop = True)

        self.setup(input_data)

    def setup(self, data):
        data = self._rowcol_cut(data)

        print str(len(data)) + " remaining after the cuts"
        #print data

        # determine the PCA parameters (eigenvectors and -values) on this set of data
        data = data.as_matrix()
        self.pca.fit(data)

    def get_last_indices(self):
        return self.last_indices

    def process(self, data):
        cut_data = self._rowcol_cut(data)

        self.last_indices = cut_data.index

        if len(cut_data) != 0:
            processed_data = self.pca.transform(cut_data.as_matrix())
            #processed_data = cut_data.as_matrix()
        else:
            processed_data = [[]]

        return {self.name: np.array(processed_data)}

    def save(self, folder, filename):
        outfile = open(folder + filename, "wb")
        pickle.dump(self.pca, outfile)
        outfile.close()

    def load(self, folder, filename):
        infile = open(folder + filename, "rb")
        self.pca = pickle.load(infile)
        infile.close()

    def _rowcol_cut(self, data):
        # apply the row selection
        data = data.loc[data.apply(self.cuts, axis = 1)]

        # apply the column selection
        output_data = data.loc[:, self.processed_columns]

        #print output_data
                
        return output_data

    def _as_matrix(self, df):
        return np.array(df.as_matrix().tolist())
