// ======================================================================
// Simple dynamic thread pool.
//
// Copyright 2018 Soheyl Shahverdi
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <stdint.h>
#include <pthread.h>
#include "queue.h"


typedef struct __thread_pool_t__{
	volatile int active_count;
	volatile int curr_count;
	volatile int normal_count;
	int max_count;
	uint32_t id_pool_idx;
	uint32_t *id_pool;
	pthread_mutex_t lock;
	volatile char condition;
	pthread_cond_t cond;
	queue_t* queue;
	int(*run)(struct __thread_pool_t__*, void(*)(uint32_t id, void*), void*);
}thread_pool_t;

typedef struct{
	pthread_mutex_t lock;
	volatile char condition;
	pthread_cond_t cond;
	uint32_t id;
	void* priv;
	void(*hook)(uint32_t id, void*);
	thread_pool_t *pool;
}thread_t;

thread_pool_t* thread_pool_create(int normal_size, int max_count);
void thread_pool_normal_size_set(thread_pool_t*, int normal_size);

#endif
