#ifndef RENEWABILITY_H
#define RENEWABILITY_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

/*
 * Possible renewability message type
 */
typedef enum {
    RN_RENEW_BLOCK,
    RN_RENEW_ALLBLOCKS,
} rn_message_type;

/*
 * Defines renewability messages structure
 */
typedef struct {
    rn_message_type type;
    uint32_t length;
    char buffer[512];
} rn_message;

/*
 * Defines renew block[s] message structure
 */
typedef struct {
    uint32_t block_index;
    bool code;
    uint32_t timeout;
} rn_renewblock;

void renewabilityInit () __attribute__((constructor(105)));

/* extern unbinder definition */
extern void EraseMobileBlock (uint32_t index, bool code);
extern void EraseAllMobileBlocks ();

#define RENEWABILITY_POLLING_INTERVAL 500   /* ms */

#endif /* RENEWABILITY_H */
