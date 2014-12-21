/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_MEMORY__
#define __KEYSTORE_MEMORY__

#include <semaphore.h>

int memory_init();
void *memory_set(void *ent);
void *memory_get(void *ent);
int memory_clear();
void memory_read_lock(sem_t *sem);
void memory_read_unlock(sem_t *sem);
void memory_write_lock(sem_t *sem, sem_t *mutex);
void memory_write_unlock(sem_t *sem);

#endif
