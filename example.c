#include "thread_pool.h"
int dummy_loop(int id, void* priv){

	printf("thread [%d] running.................\n", id);
	while(1){
			
		sleep(1);
	
	}

}

int dummy_func(int id, void* priv){
	printf("thread [%d] call function.........\n", id);
	usleep(1000000);
}


int main(){
	int retval;
	thread_pool_t* thread_pool;
	
	thread_pool = thread_pool_create(7500, 10000);
	
	int iter;
	for(iter =0; iter < 10000; iter++){
		thread_pool->run(thread_pool, dummy_func, NULL);
	}
	sleep(1);
	printf("thread pool curr size : %d\n", thread_pool->curr_count);
	printf("thread pool active size : %d\n", thread_pool->active_count);

	
	sleep(3);
	
	for(iter =0; iter < 10000; iter++){
		thread_pool->run(thread_pool, dummy_loop, NULL);
	}
	
	printf("thread pool curr size : %d\n", thread_pool->curr_count);
	printf("thread pool active size : %d\n", thread_pool->active_count);

	while(1) sleep(1);
	
	
	return 0;
}
