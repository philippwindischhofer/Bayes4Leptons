#!/bin/bash

if [[ -z "$CMSSW_BASE" ]]; then
    echo "ERROR: need to have CMSSW_BASE set! Did you forget to run 'cmsenv'?"
    exit
fi

# ---------------------------------------------
#  parse the given arguments
# ---------------------------------------------
POSARG=()

while [[ $# -gt 0 ]]
do
key=$1

# this can overwrite globally set environment variables
case $key in
    --source)
    MC_DIR="$2"
    shift
    shift
    ;;
    --dest)
    MC_PREPARED_DIR="$2"
    shift
    shift
    ;;
    *)
    POSARG+=("$1")
    shift
    ;;
esac
done

# set back the positional arguments in case they will be needed later
set -- "${POSARG[@]}"

MC_DIR=$MC_DIR"/"
MC_PREPARED_DIR=$MC_PREPARED_DIR"/"

JOB_SUBMITTER="/opt/exp_soft/cms/t3/t3submit_new"
FILE_PREPARER=$CMSSW_BASE"/bin/slc6_amd64_gcc630/run_augment_single_tree"

SOURCE_FILE_NAME="ZZ4lAnalysis.root"

cd $MC_DIR
SOURCE_FOLDERS=`ls -d */`

mkdir -p $MC_PREPARED_DIR

for SOURCE_FOLDER in $SOURCE_FOLDERS
do
    SOURCE_PATH=$MC_DIR$SOURCE_FOLDER$SOURCE_FILE_NAME
    DEST_PATH=$MC_PREPARED_DIR$SOURCE_FOLDER$SOURCE_FILE_NAME
    
    # create the destination directory tree
    mkdir -p $MC_PREPARED_DIR$SOURCE_FOLDER

    PREPARATION_SCRIPT=$MC_PREPARED_DIR"run_preparation_"${SOURCE_FOLDER%/}".sh"
    PREPARATION_LOGFILE=$MC_PREPARED_DIR"log_preparation_"${SOURCE_FOLDER%/}".txt"

    echo "#!/bin/bash" > $PREPARATION_SCRIPT
    echo $FILE_PREPARER $SOURCE_PATH $DEST_PATH "1" "&>" $PREPARATION_LOGFILE >> $PREPARATION_SCRIPT
done

cd $MC_PREPARED_DIR
JOBS=`find * | grep run_preparation.*.sh$`

for JOB in $JOBS
do
    echo "launching " $MC_PREPARED_DIR$JOB
    until $JOB_SUBMITTER "-short" $MC_PREPARED_DIR$JOB
    do
	echo "----------------------------------------------------------------"
    	echo " error submitting job, retrying ..."
    	echo "----------------------------------------------------------------"
    	sleep 1
    done
done
