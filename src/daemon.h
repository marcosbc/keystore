#ifndef __KEYSTORE_DAEMON__
#define __KEYSTORE_DAEMON__

// server-specific error types
#define ERR_USE_MSG "use: %s\n"
#define ERR_SESSION 10
#define ERR_SESSION_MSG "you already have a session running"
#define ERR_SHMCREATE 11
#define ERR_SHMCREATE_MSG "couldn't create shared memory"
#define ERR_SHMCTL 12
#define ERR_SHMCTL_MSG "while removing our shared memory segment"
#define ERR_SEMUNLINK 13
#define ERR_SEMUNLINK_MSG "while unlinking semaphores"
#define ERR_THR 15
#define ERR_THR_MSG "couldn't create thread"
#define ERR_THRJOIN 16
#define ERR_THRJOIN_MSG "couldn't join thread"
#define ERR_BIND 17
#define ERR_BIND_MSG "couldn't bind to the address"
#define ERR_LISTEN 18
#define ERR_LISTEN_MSG "while listening to the socket"
#define ERR_ACCEPT 19
#define ERR_ACCEPT_MSG "couldn't accept the request"

// non-fatal errors
#define ERR_DB 20
#define ERR_DB_MSG "db not found"
#define ERR_ENTRY 21
#define ERR_ENTRY_MSG "entry not found"

// related to server use
#define NUM_ARGS 1 // 1 + 0 arguments

void print_error_case(int error);

#endif
