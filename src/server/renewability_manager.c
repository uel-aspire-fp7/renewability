#include "renewability_manager.h"

/**
 * CTRL+C signal manager
 */
void sighandler() {
    printf("\nSIGNAL received, shutting down...\n");

    force_exit = 1;
}

/**
 * Logs MySQL error, closes the connection to database
 */
int terminateWithMysqlError(MYSQL *con) {
    renewabilityLog(mysql_error(con), RN_ERROR);

    if (NULL != con)
        mysql_close(con);

    return RN_MANAGER_KO;
}

/**
 * Executes a query and returns its result when succeeds or NULL when fails
*/
MYSQL_RES* execute_query(MYSQL* con, char* query) {
    MYSQL_RES *result = NULL;

    if (mysql_query(con, query)) {
        terminateWithMysqlError (con);
        return  NULL;
    }

    if ((result = mysql_store_result(con)) == NULL) {
        terminateWithMysqlError(con);
        return  NULL;
    }

    return result;
}

/**
 * Generates a new revision number for a given application
 */
char* getNewRevisionNumber (MYSQL* con, char application_id[AIDL]) {
    char* returnBuffer = (char*)malloc(sizeof(char) * RN_REVISION_NUMBER_BUFFER_LEN);
    char revisionQuery[1024];
    MYSQL_ROW row;
    MYSQL_RES* result;

    // default value when no revisions are found into database
    strcpy (returnBuffer, "00000001");

    sprintf (revisionQuery, "SELECT COUNT(id) FROM " RENEWABILITY_DB_NAME ".rn_revision WHERE application_id='%s'", application_id);

    if ((result = execute_query(con, revisionQuery)) != NULL) {
        if ((row = mysql_fetch_row(result)) != NULL) {
            // a result was found, let's increment the number of revisions a generate a new one
            sprintf (returnBuffer, RN_REVISION_FORMAT, atoi(row[0]) + 1);
        }
    }

    return returnBuffer;
}

/**
 * Generates a new application revision
 */
bool generateNewRevision(char application_id[AIDL], char revision_number[RN_REVISION_NUMBER_BUFFER_LEN], char* diversification_script, unsigned int apply_from, unsigned int apply_to) {
    int return_value;
    char script_invocation[2048], revision_directory[2048];
    DIR* dir;

    /* check if the requested revision has already been generated*/
    sprintf(revision_directory, RENEWABILITY_REVISION_PATH, application_id, revision_number);

    dir = opendir(revision_directory);

    if (dir)
    {
        /* revision has already been generated. */
        closedir(dir);

        return true;
    }

    /* we need to launch the generation script */
    sprintf(script_invocation, "%s -a %s -r %s -o %s -f %d -t %d",
            RENEWABILITY_GENERATION_SCRIPT,
            application_id,
            revision_number,
            diversification_script,
            apply_from,
            apply_to + 60);     /* we need to add some room for new revision generation */

    renewabilityLog("Executing renew script: %s", RN_DEBUG, script_invocation);

    /* invoke generation script */
    return_value = system (script_invocation);

    renewabilityLog("Renew script returned: %d (0 expected)", RN_DEBUG, return_value);

    return return_value == 0;
}

int asclWebSocketDispatcherMessage(
        TECHNIQUE_ID	technique_id,
        char			application_id[AIDL],
        int 			message_id,
        size_t			length,
        const char*		payload,
        size_t*			response_length,
        char*			response) {

    rn_manager_node* newNode = NULL;

    if (RN_RENEWABILITY != technique_id)
        return ASCL_SUCCESS;

    switch (message_id) {
        case ASCL_WS_MESSAGE_OPEN:
            renewabilityLog("BACKEND: Connection opened (TID: %d, AID: %s)\n", RN_DEBUG, technique_id, application_id);

            if (!isApplicationActive(application_id)) {
                newNode = addActiveApplication(application_id);
                newNode->last_ack_request = 0;
                newNode->last_ack_response = 0;
                newNode->last_update_time = newNode->connection_time = (unsigned long)time(NULL);

                renewabilityLog ("Application %s has been added to currently served list (%s)", RN_DEBUG, application_id, newNode->application_id);
            }

            break;
        case ASCL_WS_MESSAGE_SEND:
            renewabilityLog("BACKEND: Send received (TID: %d, AID: %s, buf: %d bytes, %s)\n", RN_DEBUG,technique_id, application_id, length, payload);

            /* this feature is not used by Renewability Framework */

            break;
        case ASCL_WS_MESSAGE_EXCHANGE:
            renewabilityLog("BACKEND: Exchange received (TID: %d, AID: %s, buf: %d bytes, %s)\n", RN_DEBUG,technique_id, application_id, length, payload);

            if (NULL != response) {
                memcpy(response, payload, length);
                *response_length = length;
            }

            break;
        case ASCL_WS_MESSAGE_CLOSE:
            renewabilityLog("BACKEND: Connection closed (TID: %d, AID: %s)\n", RN_DEBUG, technique_id, application_id);

            if (isApplicationActive(application_id)) {
                removeActiveApplication(application_id);
            }

            break;
    }

    return ASCL_SUCCESS;
}

/**
 * Retrieves an application policy from  database given the ASPIRE Application ID
 * If the policy is not found, NULL is returned
 */
renewability_policy* getApplicationPolicy(MYSQL *con, char application_id[AIDL]) {
    char policy_query[1024];
    MYSQL_RES *result;
    MYSQL_ROW row;
    renewability_policy* policy = NULL;

    /* application policies query */
    sprintf(policy_query, "SELECT revisions_duration, timeout_mandatory, diversification_script FROM rn_application_policy WHERE application_id = '%s'", application_id);

    if (mysql_query(con, policy_query)) {
        terminateWithMysqlError (con);
        return  NULL;
    }

    if ((result = mysql_store_result(con)) == NULL) {
        terminateWithMysqlError(con);
        return  NULL;
    }

    /* if no results return NULL */
    if (mysql_num_rows(result) > 0) {
        if ((row = mysql_fetch_row(result)) == NULL) {
            terminateWithMysqlError(con);

            return NULL;
        }

        policy = (renewability_policy*)malloc(sizeof(renewability_policy));
        policy->revision_durarion = (unsigned long)atoi(row[0]);
        policy->timeout_mandatory = atoi(row[1]);
        strcpy(policy->diversification_script, row[2]);
    }

    return policy;
}

/**
 * Requests a block renewal to a given application
 */
bool queryClient (rn_manager_node* application, struct libwebsocket_context* ws_context) {
    rn_message* message;
    rn_renewblock* renewblock;
//    char response[512];
//    size_t response_length;

    renewabilityLog("Querying client: %s.", RN_INFO, application->application_id);

    message = (rn_message*)malloc(sizeof(rn_message));
    renewblock = (rn_renewblock*)malloc(sizeof(rn_renewblock));

    message->type = RN_RENEW_ALLBLOCKS;
    message->length = sizeof(rn_renewblock);

    renewblock->block_index = 0;
    renewblock->code = true;
    renewblock->timeout = application->policy_timeout;

    memcpy((void*)message->buffer, (void*)renewblock, sizeof(renewblock));

    application->last_ack_request = (unsigned long)time(NULL);

    /* send the request through ACCL channel */
    if (ASCL_ERROR != asclWebSocketSend(ws_context,
                                            application->application_id,
                                            RN_RENEWABILITY,
                                            (void*)message,
                                            sizeof(rn_message))){/*
                                            (void*)response,
                                            &response_length)*/
        /* client correctly queried */
        application->last_ack_response = (unsigned long)time(NULL);
        application->last_update_time = (unsigned long)time(NULL);

        renewabilityLog("Client: %s acknowledged revision change.", RN_INFO, application->application_id);

        return true;
    }

    renewabilityLog("Client: %s failed acknowledgement.", RN_INFO, application->application_id);

    /* error querying client, a new try will be done at next polling time */
    return false;
}

/**
 * Loops through the active applications list, checks policies and queries clients
 * when an upgrade is needed
 */
bool checkPoliciesAndQueryClients(MYSQL *con, struct libwebsocket_context* ws_context) {
    renewability_policy* policy = NULL;
    rn_manager_node* app_iterator = getFirstActiveApplication();
    unsigned long last_update_time = 0, current_time = 0;
    bool revision_generated = false;
    char *revision_number;
    bool stop_serving_application;

    /* if there is at least an active application */
    if (NULL != app_iterator) {
        do {
            stop_serving_application = false;
            current_time = (unsigned long)time(NULL);

            renewabilityLog("Checking policies for app %s...", RN_DEBUG, app_iterator->application_id);

            /* retrieve renewability policy for current application */
            if ((policy = getApplicationPolicy(con, app_iterator->application_id)) != NULL) {
                renewabilityLog("* Policy found (revision duration: %ds, mandatory: %d)", RN_DEBUG,
                                policy->revision_durarion,
                                policy->timeout_mandatory);

                /* if timeout for current application is expired let's renew it */
                if (current_time - app_iterator->last_update_time >= policy->revision_durarion) {

                    renewabilityLog("** Renewal needed due to application '%s' policy (Timeout: %d, Elapsed: %d).",
                                    RN_INFO,
                                    app_iterator->application_id,
                                    policy->revision_durarion,
                                    current_time - app_iterator->last_update_time);

                    /* generate new revision number */
                    revision_number = getNewRevisionNumber(con, app_iterator->application_id);

                    renewabilityLog("New revision number: %s.", RN_INFO, revision_number);

                    /* generate new revision */
                    revision_generated = generateNewRevision(
                            app_iterator->application_id,
                            revision_number,
                            policy->diversification_script,
                            current_time,
                            current_time + policy->revision_durarion);

                    if (revision_generated) {
                        /* a new revision is ready, client must be warned and requested to unbind the previous one */
                        if (!queryClient(app_iterator, ws_context) && policy->timeout_mandatory)
                            stop_serving_application = true;
                    } else {
                        renewabilityLog("Revision creation failed.", RN_INFO);

                        if (policy->timeout_mandatory) {
                            stop_serving_application = true;
                        }
                    }

                    if (stop_serving_application) {
                        /* TODO activate reaction system, should we use the RA infrastructure? */
                    }
                } else {
                    /* timeout not expired */
                    renewabilityLog("** Timeout not expired %d out of %d seconds", RN_DEBUG, current_time - app_iterator->last_update_time, policy->revision_durarion);
                }
            } else {
                // no policies found for current application
                renewabilityLog("Policy list for application '%s' is NULL", RN_DEBUG, app_iterator->application_id);
            }

        } while (NULL != app_iterator->next);
    } else {
        renewabilityLog("Application list is NULL", RN_DEBUG);
    }

    sleep(1);

    return true;
}

/**
 * Renewability
 */
int main (int argc, char** argv) {
    struct libwebsocket_context* ws_context;
    MYSQL *con = mysql_init(NULL);

    renewabilityLog("Starting manager...", RN_DEBUG);

    /* MySQL library initialization */
    if (NULL == con)
        return terminateWithMysqlError (con);

    renewabilityLog("Connecting to MySQL server...", RN_DEBUG);

    /* if no connection to database is available -> terminate */
    if (mysql_real_connect(
            con,
            RENEWABILITY_DB_HOST,
            RENEWABILITY_DB_USER,
            RENEWABILITY_DB_PASS,
            RENEWABILITY_DB_NAME, 0, NULL, 0) == NULL)
        return terminateWithMysqlError (con);

    /* CTRL+c signal handler */
    signal(SIGINT, sighandler);

    renewabilityLog("Initializing WebSocket server...", RN_DEBUG);

    /* websocket server initialization */
    ws_context = asclWebSocketInit (RN_RENEWABILITY);

    if (NULL != ws_context) {
        while (force_exit == 0) {
            checkPoliciesAndQueryClients(con, ws_context);

            // library service (read/write callbacks) routine
            libwebsocket_service(ws_context, 1000);   // 1000ms timeout
        }

        renewabilityLog("Shutting down manager.", RN_DEBUG);

        asclWebSocketShutdown(ws_context);
    } else {

        renewabilityLog("Unable to initialize WebSocket context.", RN_ERROR);

        return 1;
    }

    return 0;
}

/**
 * Logging facility
 */
void renewabilityLog(const char* fmt, int lvl, ...) {
    FILE * fp;
    time_t now;

    va_list ap;                                /* special type for variable */
    char format[1024];       		/* argument lists            */
    int count = 0;
    int i, j;                                  /* Need all these to store   */
    char c;                                    /* values below in switch    */
    double d;
    unsigned u;
    char *s;
    void *v;

    if (lvl < RENEWABILITY_MANAGER_LOG_LEVEL)
        return;

    fp = fopen (RENEWABILITY_MANAGER_LOG_FILE, "a");

    if (NULL == fp) {
        fprintf(stderr, "ERROR: Unable to log to file '%s'\n", RENEWABILITY_MANAGER_LOG_FILE);
        return;
    }

    if ((time_t)-1 == time(&now)) {
        fprintf(stderr, "ERROR: Unable to retrieve current time.log\n");
        return;
    }

    fprintf(fp, "%.24s [RN_MANAGER] ", ctime(&now));

    va_start(ap, lvl);                         /* must be called before work */

    while (*fmt) {
        for (j = 0; fmt[j] && fmt[j] != '%'; j++)
            format[j] = fmt[j];                    /* not a format string          */

        if (j) {
            format[j] = '\0';
            count += fprintf(fp, format, "");    /* log it verbatim              */
            fmt += j;
        } else {
            for (j = 0; !isalpha(fmt[j]); j++) {   /* find end of format specifier */
                format[j] = fmt[j];
                if (j && fmt[j] == '%')              /* special case printing '%'    */
                    break;
            }

            format[j] = fmt[j];                    /* finish writing specifier     */
            format[j + 1] = '\0';                  /* don't forget NULL terminator */
            fmt += j + 1;

            switch (format[j]) {                   /* cases for all specifiers     */
                case 'd':
                case 'i':                              /* many use identical actions   */
                    i = va_arg(ap, int);                 /* process the argument         */
                    count += fprintf(fp, format, i); /* and log it                 */
                    break;
                case 'o':
                case 'x':
                case 'X':
                case 'u':
                    u = va_arg(ap, unsigned);
                    count += fprintf(fp, format, u);
                    break;
                case 'c':
                    c = (char) va_arg(ap, int);          /* must cast!  */
                    count += fprintf(fp, format, c);
                    break;
                case 's':
                    s = va_arg(ap, char *);
                    count += fprintf(fp, format, s);
                    break;
                case 'f':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                    d = va_arg(ap, double);
                    count += fprintf(fp, format, d);
                    break;
                case 'p':
                    v = va_arg(ap, void *);
                    count += fprintf(fp, format, v);
                    break;
                case 'n':
                    count += fprintf(fp, "%d", count);
                    break;
                case '%':
                    count += fprintf(fp, "%%");
                    break;
                default:
                    fprintf(stderr, "Invalid format specifier in acclLOG().\n");
            }
        }
    }

    fprintf(fp, "\n");

    va_end(ap);  // clean up

    fclose(fp);
}