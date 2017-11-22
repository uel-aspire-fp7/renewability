#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

usage () {
    echo "USAGE: $0 -a ASPIRE_APPLICATION_ID -r REVISION_NUMBER -o DIVERSIFICATION_SCRIPT -f APPLY_FROM -t APPLY_TO";
    echo "  -a String, ASPIRE Application ID"
    echo "  -r String, Revision number as specified in revisions directory."
    echo "  -o Path, Path to the diversification script."
    echo "  -f String, formatted revision validity range start (yyyy-mm-dd hh:mm:ss) (e.g. date +\"%Y-%m-%d %T\")"
    echo "  -t String, formatted revision validity range end (yyyy-mm-dd hh:mm:ss) (e.g. date +\"%Y-%m-%d %T\")"

    exit 0;
}

set -o errexit
set -o pipefail

MANDATORY=0

while getopts "o:a:r:f:t:h" OPT
do
    case $OPT in
        "o")
            UPDATE_BLOCKS_SCRIPT=$OPTARG
            ;;
        "a")
            APPLICATION_ID=$OPTARG
            ;;
        "r")
            REVISION_NUMBER=$OPTARG
            ;;
       "f")
            APPLY_FROM=$OPTARG
            ;;
        "t")
            APPLY_TO=$OPTARG
            ;;
        "h") usage; ;;
        \?)
            ;;
    esac
done

#if [ ! -d ${OBJ_SOURCE_DIRECTORY} ]; then
#    echo "Invalid OBJ_SOURCE_DIRECTORY. Use $0 -h for help."
#    exit 1
#fi

if [ -z ${APPLICATION_ID+x} ]; then
    echo "ASPIRE_APPLICATION_ID not specified. Use $0 -h for help."
    exit 2
fi

if [ -z ${REVISION_NUMBER+x} ]; then
    echo "REVISION_NUMBER not specified. Use $0 -h for help."
    exit 3
fi

if [ -z ${APPLY_FROM+x} ]; then
    echo "APPLY_FROM not specified. Use $0 -h for help."
    exit 4
fi

if [ -z ${APPLY_TO+x} ]; then
    echo "APPLY_TO not specified. Use $0 -h for help."
    exit 5
fi

REVISION_QUERY="USE RN_development; INSERT INTO rn_revision (application_id, number, apply_from, apply_to) VALUES ('"${APPLICATION_ID}"', '"${REVISION_NUMBER}"', FROM_UNIXTIME(${APPLY_FROM}), FROM_UNIXTIME(${APPLY_TO}));"

# original blocks directory
#MOBILE_BLOCKS_DIRECTORY=/opt/online_backends/${APPLICATION_ID}/code_mobility/00000000

# diversified blocks directory
#REVISED_BLOCKS_DIRECTORY=/opt/online_backends/${APPLICATION_ID}/code_mobility/${REVISION_NUMBER}

#mkdir -p ${REVISED_BLOCKS_DIRECTORY}

#cp ${MOBILE_BLOCKS_DIRECTORY}/mobile_dump_* ${REVISED_BLOCKS_DIRECTORY}/

SEED=$(echo $RANDOM$RANDOM)

# Generate new block. We invoke the ACTC docker to do this.
wget actc/renewability/${UPDATE_BLOCKS_SCRIPT}/${SEED}

if [ $? -ne 0 ]; then
    echo "ERROR while updating mobile blocks."
    exit 7
else
    # inserts a revision record in DB
    local_dir=$(dirname $0)
    echo $REVISION_QUERY | mysql --defaults-file=$local_dir/mysql.cnf

    if [ $? -ne 0 ]; then
        echo "ERROR while inserting revision."
        exit 6
    else
        echo "Revision '${REVISION_NUMBER}' for application '${APPLICATION_ID}' correctly set."
    fi

    exit 0
fi
