#ifndef NODES_H
#define NODES_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* length of strings containing APP ID */
#define AIDL 50

/* struct used to manage applications status */
typedef struct rn_manager_node {
    char application_id[AIDL];            /* ASPIRE Application ID */
    unsigned long connection_time;      /* Connection timestamp */
    unsigned long last_update_time;     /* Last revision update time */
    unsigned long last_ack_request;     /* Last acknowledge request timestamp */
    unsigned long last_ack_response;    /* Last acknowledge response timestamp */
    uint32_t policy_timeout;            /* Timeout for ACK */
    bool policy_mandatory;              /* Is ACK timeout mandatory? */

    struct rn_manager_node *next;
    struct rn_manager_node *previous;
} rn_manager_node;

rn_manager_node* getLastActiveApplication();
rn_manager_node* getFirstActiveApplication();
rn_manager_node* findActiveApplication(char application_id[AIDL]);
rn_manager_node* addActiveApplication(char application_id[AIDL]);
bool removeActiveApplication(char application_id[AIDL]);
bool isApplicationActive(char application_id[AIDL]);

/* used to force exit when a signal is received */
static rn_manager_node* active_applications = NULL;

#endif /* NODES_H */
