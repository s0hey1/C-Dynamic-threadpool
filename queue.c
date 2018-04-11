
// ======================================================================
// Simple thread safe wait queue.
//
// Copyright 2018 Soheyl Shahverdi
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>

#include "queue.h"

queue_t* queue_create(uint32_t queue_len, uint8_t wait_policy_flag){
	int retval;
	queue_t* queue;
	
	if(!is_power_of_two(queue_len))
		goto error0;
	
	queue = calloc(1, sizeof(queue_t));
	if(queue == NULL)
		goto error0;
	
	retval = sem_init(&queue->produce_semaphor, 0, queue_len-2);
	if(retval != 0)
		goto error1;
	retval = sem_init(&queue->consume_semaphor, 0, 0);
	if(retval != 0)
		goto error2;
	queue->wait_policy_flag = wait_policy_flag;

	queue->array = calloc(1,sizeof(void*)*queue_len);
	if(queue->array == NULL)
		goto error3;

	queue->queue_len = queue_len;

	queue->producer.front = 0;
	queue->producer.rear = 0;

	queue->consumer.rear = 0;
	queue->consumer.front = 0;

	return queue;
error3:
	sem_destroy(&queue->consume_semaphor);
error2:
	sem_destroy(&queue->produce_semaphor);
error1:
	free(queue);
error0:
	return NULL;
}

void queue_destroy(queue_t* queue){
	sem_destroy(&queue->consume_semaphor);
	sem_destroy(&queue->produce_semaphor);
	free(queue->array);
	free(queue);
}

int queue_push(queue_t* queue, void* obj){

	uint32_t producer_front;
	uint32_t consumer_rear;
	uint32_t free_entry_count;
	uint32_t next_producer_front;

	if((queue->wait_policy_flag&WAIT_QUEUE_PROD_WAIT)==WAIT_QUEUE_PROD_WAIT)
		sem_wait(&queue->produce_semaphor);

	pthread_mutex_lock(&queue->produce_lock);

	producer_front = queue->producer.front;
	consumer_rear = queue->consumer.rear;
	next_producer_front = producer_front + 1;
	free_entry_count = (queue->queue_len-1) + consumer_rear - producer_front;

	if(free_entry_count == 0){
		pthread_mutex_unlock(&queue->produce_lock);
		return -1;
	}

	queue->producer.front = next_producer_front;

	queue->array[(queue->queue_len-1)&queue->producer.front] = obj;

	queue->producer.rear = next_producer_front;

	pthread_mutex_unlock(&queue->produce_lock);

	if((queue->wait_policy_flag&WAIT_QUEUE_CONS_WAIT)==WAIT_QUEUE_CONS_WAIT)
		sem_post(&queue->consume_semaphor);
		
	return 0;

}

int queue_pop(queue_t* queue, void** obj){
	uint32_t consumer_front;
	uint32_t producer_rear;
	uint32_t entry_count;
	uint32_t next_consumer_front;

	if((queue->wait_policy_flag&WAIT_QUEUE_CONS_WAIT)==WAIT_QUEUE_CONS_WAIT)
		sem_wait(&queue->consume_semaphor);
	
	pthread_mutex_lock(&queue->consume_lock);

	consumer_front = queue->consumer.front;
	producer_rear = queue->producer.rear;
	next_consumer_front = consumer_front + 1;
	entry_count = producer_rear - consumer_front;

	if(entry_count == 0){
		pthread_mutex_unlock(&queue->consume_lock);
		return -1;
	}

	queue->consumer.front = next_consumer_front;

	*obj = queue->array[(queue->queue_len-1)&queue->consumer.front];

	queue->consumer.rear = next_consumer_front;

	pthread_mutex_unlock(&queue->consume_lock);
	
	if((queue->wait_policy_flag&WAIT_QUEUE_PROD_WAIT)==WAIT_QUEUE_PROD_WAIT)
		sem_post(&queue->produce_semaphor);
		
	return 0;

}

