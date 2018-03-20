import pandas as pd
import os

# describes an ensemble of classifiers for a specific pair of categories. Each model in the collection targets a different region in the event space (as filtered out by the corresponding preprocessors)
class ModelCollection:

    def __init__(self, name = "noname", H1_paths = None, H0_paths = None):
        self.models = {}
        self.preprocessors = {}
        self.name = name

        self.H1_paths = H1_paths
        self.H0_paths = H0_paths

    def add_model(self, preprocessor, model):
        self.models[model.name] = model
        self.preprocessors[model.name] = preprocessor

    def load_weights(self, path):
        # look at the directory structure starting at path, and attempt to load the weights of all the models that are contained in this collection

        # remove the last slash, if there is one (to ensure os.path.basename can operate correctly)
        if path.endswith("/"):
            path = path[:-1]

        inferred_collection_name = os.path.basename(path)

        if inferred_collection_name != self.name:
            raise IOError("Error: are you sure this is the right directory for this collection? [" + inferred_collection_name + " != " + self.name + "]")

        possible_model_dirs = next(os.walk(path))[1]
        model_dirs = []
        print "found the following models belonging to this collection:"
        for model_dir in possible_model_dirs:
            if model_dir in self.models:
                model_dirs.append(model_dir)
                print model_dir

        for model_dir in model_dirs:
            model_path = path + "/" + model_dir + "/"
            print "now attempting to load model '" + model_dir + "' from file '" + model_path + "final.hdf5"
            self.models[model_dir].load(model_path, "final.hdf5")

    # contrary to its low-level equivalent model.predict, this one works directly on the Pandas dataframe, and not on the numpy array itself
    def predict(self, dataframe):
        # now need to sift through the dataframe, and apply each model on the slice that it is designed to treat. Important: all the preprocessors need to be unitary!
        predictions = []

        for (model, preprocessor) in zip(self.models.values(), self.preprocessors.values()):
            # have each model give predictions on the relevant pieces of the full data
            cur_data = preprocessor(dataframe)
            cur_prediction = model.get_keras_model().predict(x = cur_data.as_matrix(), verbose = 2, batch_size = len(cur_data)).flatten()
            
            # then, keep track of the indices (i.e. the positions of the individual chunks in the main datastream)
            cur_series = pd.Series(cur_prediction, index = cur_data.index)
            predictions.append(cur_series)

        # merge the individual chunks of predictions ...
        retval = pd.concat(predictions).sort_index()

        # ... and return them as numpy array
        return retval.as_matrix()
            
