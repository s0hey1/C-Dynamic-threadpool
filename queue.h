// ======================================================================
// Simple thread safe wait queue.
//
// Copyright 2018 Soheyl Shahverdi
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>

#define WAIT_QUEUE_PROD_WAIT 1
#define WAIT_QUEUE_CONS_WAIT 2

typedef struct{
	struct{
		volatile uint32_t busy_flag;
		volatile uint32_t front;
		volatile uint32_t rear;
	}producer;
	struct{
		volatile uint32_t busy_flag;
		volatile uint32_t front;
		volatile uint32_t rear;
	}consumer;
	uint32_t queue_len;
	void** array;
	
	uint8_t wait_policy_flag;
	sem_t produce_semaphor;
	sem_t consume_semaphor;
	pthread_mutex_t produce_lock;
	pthread_mutex_t consume_lock;
}queue_t;

queue_t* queue_create(uint32_t queue_len, uint8_t wait_policy_flag);
void queue_destroy(queue_t* queue);
int queue_push(queue_t* queue, void* obj);
int queue_pop(queue_t* queue, void** obj);


static inline int is_power_of_two (uint32_t x){
  return ((x != 0) && !(x & (x - 1)));
}

#endif
