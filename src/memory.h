/*
 * AUTHOR: Marcos Bjorkelund
 * DATE: 06/11/2014
 * SUBJECT: Operating Systems
 * HIGHER TECHNICAL SCHOOL OF ENGINEERING, UNIVERSITY OF SEVILLE
 */
#ifndef __KEYSTORE_MEMORY__
#define __KEYSTORE_MEMORY__

#define KEY_ID 'K'
#define SHM_SIZE 1024

int memory_init();
void *memory_set(void *ent);
void *memory_get(void *ent);
int memory_clear();

#endif
