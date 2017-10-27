#!/bin/bash

usage () {
    echo "USAGE: $0 -a ASPIRE_APPLICATION_ID -d REVISION_DURATION_TIME [-m] -r DIVERSIFICATION_SCRIPT";
    echo "  -a ASPIRE Application ID"
    echo "  -d Duration of each revision in seconds. When the duration expires a new revision is issued."
    echo "  -r Path to diversification script."
    echo "  -m OPTIONAL,    if this argument is specified then a revision change is mandatory and if a"
    echo "                  client application fails to acknowledge the revision change within a given"
    echo "                  timeout no more blocks are served."

    exit 0;
}

set -o errexit
set -o pipefail

MANDATORY=0

while getopts "a:d:r:m:h" OPT
do
    case $OPT in
        "a")
            APPLICATION_ID=$OPTARG
            ;;
        "r")
            DIVERSIFICATION_SCRIPT=$OPTARG
            ;;
        "d")
            DURATION=$OPTARG

            if [ ${DURATION} -le -1 ]; then
                echo "Invalid duration: '${DURATION}'. It should be an integer >= 0" >&2

                exit 1
            fi

            ;;
        "m")
            MANDATORY=1

            ;;
        "h") usage; ;;
        \?)
            ;;
    esac
done

if [ -z ${APPLICATION_ID+x} ]; then
    echo "ASPIRE_APPLICATION_ID not specified. Use $0 -h for help."
    exit 1
fi

if [ -z ${DURATION+x} ]; then
    echo "DURATION not specified. Use $0 -h for help."
    exit 1
fi

POLICY_QUERY="USE RN_development; REPLACE INTO rn_application_policy (application_id, revisions_duration, timeout_mandatory, diversification_script) VALUES ('"${APPLICATION_ID}"', ${DURATION}, ${MANDATORY}, '"${DIVERSIFICATION_SCRIPT}"');"

local_dir=$(dirname $0)
echo $POLICY_QUERY | mysql --defaults-file=$local_dir/mysql.cnf

if [ $? -ne 0 ]; then
    echo "ERROR while setting policy."
    exit 1
else
    echo "Policy for application '${APPLICATION_ID}' correctly set."
    exit 0
fi
