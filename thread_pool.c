// ======================================================================
// Simple dynamic thread pool.
//
// Copyright 2018 Soheyl Shahverdi
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include "thread_pool.h"

static void thread_loop(thread_t* new_thread){
	int retval;
	thread_t thread;
	memcpy(&thread, new_thread, sizeof(thread));
	
	pthread_mutex_lock(&thread.pool->lock);
	thread.pool->curr_count++;
	thread.pool->condition = 1;
	pthread_cond_signal(&thread.pool->cond);
	pthread_mutex_unlock(&thread.pool->lock);
	
	do{
		
		if(thread.hook != NULL){
			pthread_mutex_lock(&thread.pool->lock);
			thread.pool->active_count++;
			pthread_mutex_unlock(&thread.pool->lock);

			thread.hook(thread.id, thread.priv);			

			pthread_mutex_lock(&thread.pool->lock);
			thread.pool->active_count--;
			pthread_mutex_unlock(&thread.pool->lock);
		}
		
		pthread_mutex_lock(&thread.pool->lock);
		if(thread.pool->curr_count > thread.pool->normal_count)
		if(thread.pool->curr_count - thread.pool->active_count > thread.pool->curr_count / 2){
			thread.pool->curr_count--;
			thread.pool->id_pool[++thread.pool->id_pool_idx] = thread.id;
			pthread_mutex_unlock(&thread.pool->lock);			
			break;
		}
		pthread_mutex_unlock(&thread.pool->lock);
		
		queue_push(thread.pool->queue, &thread);
		
		pthread_mutex_lock(&thread.lock);
		while(!thread.condition)
			pthread_cond_wait(&thread.cond, &thread.lock);
		thread.condition = 0;
		pthread_mutex_unlock(&thread.lock);
		
	}while(1);
#ifdef __THREAD_POOL_DEBUG__
	printf("thread [%d] terminated.\n", thread.id);
#endif	
	pthread_detach(pthread_self());

}

static int thread_run(thread_pool_t* thread_pool, void(*hook)(void*), void* priv){
	int retval;
	thread_t tmp_thread;
	thread_t* thread;
	pthread_t pthread;
	int thread_id = -1;
	
	retval = queue_pop(thread_pool->queue, &thread);
	
	if(retval < 0){
		
		pthread_mutex_lock(&thread_pool->lock);
		if(thread_pool->curr_count < thread_pool->max_count - 1)
			thread_id = thread_pool->id_pool[thread_pool->id_pool_idx--];
		pthread_mutex_unlock(&thread_pool->lock);

		if(thread_id >= 0){
		
			memset(&tmp_thread, 0, sizeof(thread_t));
			
			tmp_thread.id = thread_id;
			tmp_thread.priv = priv;
			tmp_thread.hook = hook;
			tmp_thread.pool = thread_pool;
			retval = pthread_create(&pthread, NULL, thread_loop, &tmp_thread);
			if(retval < 0){
						
				pthread_mutex_lock(&thread_pool->lock);
				if(thread_pool->curr_count < thread_pool->max_count - 1)
					thread_pool->id_pool[++thread_pool->id_pool_idx] = thread_id;
				pthread_mutex_unlock(&thread_pool->lock);
			
				goto error;
			}
			
			pthread_mutex_lock(&thread_pool->lock);
			while(!thread_pool->condition)
				pthread_cond_wait(&thread_pool->cond, &thread_pool->lock);
			thread_pool->condition = 0;
			pthread_mutex_unlock(&thread_pool->lock);
#ifdef __THREAD_POOL_DEBUG__
			printf("thread [%d] created.\n", tmp_thread.id);
#endif
		}else{
			goto error;
		}
	}else{
#ifdef __THREAD_POOL_DEBUG__
		printf("thread [%d] reused.\n", thread->id);
#endif		
		thread->priv = priv;
		thread->hook = hook;
		pthread_mutex_lock(&thread->lock);
		thread->condition = 1;
		pthread_cond_signal(&thread->cond);
		pthread_mutex_unlock(&thread->lock);
	}
	
	return 0;
	
error:
	return -1;

}

void thread_pool_normal_size_set(thread_pool_t* thread_pool, int normal_size){
	thread_pool->normal_count = normal_size;
}

thread_pool_t* thread_pool_create(int normal_size, int max_size){
	int iter;
	thread_pool_t* thread_pool;
	
	thread_pool = calloc(1, sizeof(thread_pool_t));
	if(thread_pool == NULL)
		goto error0;
		
	thread_pool->normal_count = normal_size;
	thread_pool->max_count = max_size;
	thread_pool->run = thread_run;
	thread_pool->id_pool = malloc(sizeof(uint32_t)*max_size);
	if(thread_pool->id_pool == NULL)
		goto error1;
	
	for(iter = 0; iter < max_size; iter++)
		thread_pool->id_pool[iter] = max_size - iter - 1;
	thread_pool->id_pool_idx = max_size - 1;

	thread_pool->queue = queue_create(1024, WAIT_QUEUE_PROD_WAIT);
	if(thread_pool->queue == NULL)
		goto error2;
	
	return thread_pool;
	
error2:
	free(thread_pool->id_pool);
error1:
	free(thread_pool);
error0:
	return NULL;

}

