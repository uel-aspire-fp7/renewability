#!/bin/bash

set -o errexit
set -o pipefail

CDEFAULT='\e[39m'
CRED='\e[31m'
CYELLOW='\e[33m'
CGREEN='\e[32m'

DROP_TABLES=false
SAMPLE_DATA=false

usage () {
    echo "USAGE: $0 [-d] [-e] [-h]";
    echo "  -d OPTIONAL Drops existing tables."
    echo "  -e OPTIONAL Create example data in database."
    echo "  -h OPTIONAL Prints this help."

    exit 0;
}

while getopts "deh" OPT
do
    case $OPT in
        "d")
            DROP_TABLES=true
            ;;
        "e")
            SAMPLE_DATA=true
            ;;
        "h") usage; ;;
        \?)
            ;;
    esac
done

cd $(dirname $0)

echo "* creating db if necessary"

mysql -uroot -p${MYSQL_ROOT_PASSWORD} < sql/000_database.sql

if ${DROP_TABLES}; then
    echo -e "* ${CRED}dropping existing tables${CDEFAULT}"

    mysql -uroot -p${MYSQL_ROOT_PASSWORD} < sql/005_drop_tables.sql
fi

echo "* creating tables"
mysql -uroot -p${MYSQL_ROOT_PASSWORD} < sql/010_tables.sql

if ${SAMPLE_DATA}; then
    echo -e "* ${CYELLOW}importing example data${CDEFAULT}"
    mysql -uroot -p${MYSQL_ROOT_PASSWORD} < sql/020_sample_data.sql
fi

echo "* listing existing applications"; echo

echo "USE RN_development; SELECT application_id FROM rn_application;" | mysql -uroot -p${MYSQL_ROOT_PASSWORD}

if [ $? -ne 0 ]; then
    echo -e "\n${CRED}Database creation failed. Please check your configuration.${CDEFAULT}"
else
    echo -e "\n${CGREEN}Database creation succeeded.${CDEFAULT}"
fi
