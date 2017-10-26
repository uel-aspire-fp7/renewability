#ifndef RENEWABILITY_MANAGER_H
#define RENEWABILITY_MANAGER_H

#define ASCL_EXTERN extern

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <libwebsockets.h>
#include <ascl.h>
#include <renewability.h>
#include <sys/types.h>
#include <dirent.h>
#include "nodes.h"

#define RENEWABILITY_DB_HOST                "mysql"
#define RENEWABILITY_DB_USER                "rn_user"
#define RENEWABILITY_DB_PASS                "rn_password"
#define RENEWABILITY_DB_NAME                "RN_development"

#define RN_MANAGER_KO                       -1
#define RN_REVISION_NUMBER_BUFFER_LEN       11
#define RN_REVISION_FORMAT                  "%08x"

#define RENEWABILITY_POLICIES_POLLING       1000   /* mseconds */

#define RN_DEBUG							0
#define RN_INFO								1
#define RN_ERROR							2

#define RENEWABILITY_MANAGER_LOG_FILE        "/opt/online_backends/renewability/renewability_manager.log"
#define RENEWABILITY_GENERATION_SCRIPT       "/opt/renewability/scripts/create_new_revision.sh"
#define RENEWABILITY_REVISION_PATH           "/opt/online_backends/%s/code_mobility/%s"

#ifndef RENEWABILITY_MANAGER_LOG_LEVEL
    #define RENEWABILITY_MANAGER_LOG_LEVEL 	RN_DEBUG
#endif

typedef struct renewability_policy {
    unsigned long revision_durarion;        /* Duration of revisions in s */
    bool timeout_mandatory;                 /* Is timeout mandatory? */
    char diversification_script[1024];      /* Path to diversification script */
} renewability_policy;

/* used to force exit when a signal is received */
static volatile int force_exit = 0;

void renewabilityLog(const char* fmt, int lvl, ...);

#endif /* RENEWABILITY_MANAGER_H */
