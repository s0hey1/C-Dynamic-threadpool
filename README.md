# C-Dynamic-threadpool
A simple dynamic / lazy thread pool in C. this is not a workqueue, not based on task. 
the pool size will grow in high load and shrink in low load.

## Usage
That is very straightforward:

    #include "thread_pool.h"
    ...
    thread_pool_t *thread_pool = thread_pool_create((int)min_thread_count, (int)max_thread_count);
    thread_pool->run(thread_pool, (void*)function_p, (void*)private_data_p);

## compile and run example
For Compile:
  
    make

For run:  

    ./example
