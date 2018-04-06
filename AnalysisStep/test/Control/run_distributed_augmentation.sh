#!/bin/bash

# ---------------------------------------------
#  global settings
# ---------------------------------------------
CURRENT_DIR=`pwd`
CAMPAIGN_DIR=$1
GLOBAL_SETTINGS_DIR=$CAMPAIGN_DIR

JOB_SUBMITTER="/opt/exp_soft/cms/t3/t3submit_new"

# the directories where the original sources are located
PYTHON_DIR_ORIGINAL="/home/llr/cms/wind/cmssw/CMSSW_9_4_2/src/ZZAnalysis/AnalysisStep/test/Python/"

# the (common) source directory for this campaign
BIN_DIR=$CAMPAIGN_DIR"bin/"

# the needed part from the python sources
AUGMENTER="SingleAugmentation.py"
PYTHON_LIB="trainlib"

MC_DIR="/data_CMS/cms/wind/CJLST_NTuples/"
datafiles="ggH125 VBFH125 ZH125 WplusH125 WminusH125 ttH125 ZZTo4l ggTo2e2mu_Contin_MCFM701 ggTo2mu2tau_Contin_MCFM701 ggTo4mu_Contin_MCFM701 ggTo2e2tau_Contin_MCFM701 ggTo4e_Contin_MCFM701 ggTo4tau_Contin_MCFM701"

# ---------------------------------------------
#  first, copy all the executables to the campaign folder
# ---------------------------------------------
echo "preparing filesystem for training campaign"

mkdir -p $BIN_DIR
cp -r $PYTHON_DIR_ORIGINAL$PYTHON_LIB $BIN_DIR
cp $PYTHON_DIR_ORIGINAL$AUGMENTER $BIN_DIR

cd $CAMPAIGN_DIR

RUN_DIRLIST=`ls -d */ | egrep -v 'bin|statistics'`

# for all the runs in this sweep
for RUN in $RUN_DIRLIST
do
    # create the directory that will hold the augmented MC files
    AUGMENTATION_DIR=$CAMPAIGN_DIR$RUN"augmentation/"
    TRAINING_DIR=$CAMPAIGN_DIR$RUN"training/"
    AUGMENTATION_SETTINGS_DIR=$CAMPAIGN_DIR$RUN"settings_augmentation/"
    mkdir -p $AUGMENTATION_DIR
    mkdir -p $AUGMENTATION_SETTINGS_DIR

    for datafile in $datafiles
    do
	mkdir -p $AUGMENTATION_SETTINGS_DIR$datafile

	# prepare the job augmenting this specific datafile
	AUGMENTATION_SCRIPT=$AUGMENTATION_SETTINGS_DIR$datafile"/run_augmentation.sh"
	AUGMENTATION_LOGFILE=$AUGMENTATION_SETTINGS_DIR$datafile"/log_augmentation.txt"
	
	echo "#!/bin/bash" > $AUGMENTATION_SCRIPT
	echo "python" $BIN_DIR$AUGMENTER $MC_DIR $datafile $GLOBAL_SETTINGS_DIR $TRAINING_DIR $AUGMENTATION_DIR "&>" $AUGMENTATION_LOGFILE >> $AUGMENTATION_SCRIPT
    done
done

# now go back and launch all the jobs that have been prepared
cd $CAMPAIGN_DIR

JOBS=`find * | grep run_augmentation.sh`

for JOB in $JOBS
do
    echo "launching augmentation for " $CAMPAIGN_DIR$JOB
    $JOB_SUBMITTER $CAMPAIGN_DIR$JOB
done

cd $CURRENT_DIR