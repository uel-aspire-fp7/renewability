#include "nodes.h"

/**
 * Returns last active node
 */
rn_manager_node* getLastActiveApplication() {
    rn_manager_node *conductor = active_applications;

    if (NULL != conductor) {
        while (NULL != conductor->next) {
            conductor = conductor->next;
        }
    }

    return conductor;
}

/**
 * Returns first active node
 */
rn_manager_node* getFirstActiveApplication() {
    return active_applications;
}

/**
 * Finds an application is active nodes structure given the Application ID,
 * it returns NULL if application is not found
 */
rn_manager_node* findActiveApplication(char application_id[AIDL]) {
    rn_manager_node* conductor = active_applications;

    if (NULL != conductor) {
        do {
            if (strcmp(application_id, conductor->application_id) == 0) {
                return conductor;
            }
            conductor = conductor->next;
        } while (NULL != conductor);
    }

    return NULL;
}

/**
 * Adds an application to the active application structure
 * and returns a reference to it
 */
rn_manager_node* addActiveApplication(char application_id[AIDL]) {
    rn_manager_node* lastApplication = getLastActiveApplication();
    rn_manager_node* newNode = (rn_manager_node*)malloc(sizeof(rn_manager_node));

    memset(newNode->application_id, 0, AIDL);

    strcpy(newNode->application_id, application_id);
    newNode->previous = NULL;
    newNode->next = NULL;

    if (NULL != lastApplication) {
        newNode->previous = lastApplication;
        lastApplication->next = newNode;
    } else {
        active_applications = newNode;
    }

    return newNode;
}

/**
 * Removes an application from the active application structure, return true
 * if the application is found and removed, false otherwise
 */
bool removeActiveApplication(char application_id[AIDL]) {
    rn_manager_node* application = findActiveApplication(application_id);
    int empty_list = 0;

    if (NULL != application) {
        if (NULL != application->previous)
            application->previous->next = application->next;

        if (NULL != application->next)
            application->next->previous = application->previous;

        if (NULL == application->previous && NULL == application->next)
            empty_list = 1;

        free (application);

        if (empty_list == 1)
            active_applications = NULL;
            
        return true;
    }

    return false;
}

/**
 * Verifies whether a certain application is active or not
 */
bool isApplicationActive(char application_id[AIDL]) {
    return findActiveApplication(application_id) != NULL;
}
