all:
	@gcc queue.c thread_pool.c example.c -o example -lpthread -w -D__THREAD_POOL_DEBUG__

clean:
	@rm example
