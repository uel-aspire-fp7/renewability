#!/bin/bash

usage () {
    echo "USAGE: $0 -a ASPIRE_APPLICATION_ID [-d DESCRIPTION]";
    echo "  -a ASPIRE Application ID"
    echo "  -D OPTIONAL String, Application description."

    exit 0;
}

set -o errexit
set -o pipefail

ROOT_USER=root
ROOT_PWD=aspire

DESCRIPTION=""

while getopts "a:d:h" OPT
do
    case $OPT in
        "a")
            APPLICATION_ID=$OPTARG
            ;;
        "d")
            DESCRIPTION=$OPTARG
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

APPLICATION_QUERY="USE RN_development; REPLACE INTO rn_application (application_id, description) VALUES ('"${APPLICATION_ID}"', '"${DESCRIPTION}"');"

echo $APPLICATION_QUERY | mysql -u${ROOT_USER} -p${ROOT_PWD}

if [ $? -ne 0 ]; then
    echo "ERROR while inserting application."
    exit 1
else
    echo "Application '${APPLICATION_ID}' created correctly."
    exit 0
fi