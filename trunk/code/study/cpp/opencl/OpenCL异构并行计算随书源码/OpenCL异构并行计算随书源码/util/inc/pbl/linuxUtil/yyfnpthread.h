/**
 * @file
 * @author yyfn
 *
 * @brief wrap for linux pthread function
 */
#pragma once
#include <pthread.h>
#include "../CUtil.h"

#define Pthread_create(thread, attr, func, arg) {\
	if(0 != pthread_create(thread, attr, func, arg)){\
		printError("pthread_create failed\n");\
		exit(0);\
	}\
}

#define Pthread_join(tid, ret) {\
	if(0 != pthread_join(tid, ret)){\
		printError("pthread_join failed\n");\
	}\
}

#define Pthread_detach(tid) {\
	if(0 != pthread_detach(tid)){\
		printError("can\'t detach\n");\
	}\
}

#define Pthread_barrier_init(barrier, attr, value) {\
	int ret = pthread_barrier_int(barrier, attr, value);\
	if(EAGAIN == ret){\
		printError("The system lacks the necessary resources to initialize another barrier\n");\
	}else if(EBUSY == ret){\
		printError("Attempt to reinitialize a barrier while it's in use\n");\
	}else if(EFAULT == ret){\
		printError("A fault occurred when the kernel tried to access barrier or attr\n");\
	}else if(EINVAL == ret){\
		printError("Invalid value specified by attr\n");\
	}else if(ENOMEM == ret){\
		printError("There is no enough memory to init\n");\
	}\
}

#define Pthread_barrier_destroy(barrier) {\
	int ret = pthread_barrier_int(barrier);\
	if(EAGAIN == ret){\
		printError("The system lacks the necessary resources to initialize another barrier\n");\
	}else if(EBUSY == ret){\
		printf("Attempt to reinitialize a barrier while it's in use\n");\
	}else if(EFAULT == ret){\
		printError("A fault occurred when the kernel tried to access barrier or attr\n";\
	}else if(EINVAL == ret){\
		printError("Invalid value specified by attr\n");\
	}else if(ENOMEM == ret){\
		printError("There is no enough memory to init\n");\
	}\
}

#define Pthread_barrier_wait(barrier) {\
	int ret = pthread_barrier_wait(barrier);\
	if(EINVAL == ret){\
		printError("The value specified by barrier does not refer to an initialized barrier object\n");\
	}\
}

#define Pthread_mutex_init(mutex, ttr) {\
	if(0 != pthread_mutex_init(mutex, ttr)){\
		printError("init failed\n");\
	}\
}

#define Pthread_mutex_destroy(mutex) {\
	if(0 != pthread_mutex_destroy(mutex)){\
		printError("destroy failed\n");\
	}\
}

#define Pthread_mutex_lock(mutex) {\
	if(0 != pthread_mutex_lock(mutex)){\
		printError("lock failed\n");\
	}\
}

#define Pthread_mutex_unlock(mutex) {\
	if(0 != pthread_mutex_unlock(mutex)){\
		printError("unlock failed\n");\
	}\
}

#define Pthread_cond_init(cond, ttr) {\
	if(0 != pthread_cond_init(cond, ttr)){\
		printError("init failed\n");\
	}\
}

#define Pthread_cond_destroy(cond) {\
	if(0 != pthread_cond_destroy(cond)){\
		printError("destroy failed\n");\
	}\
}

#define Pthread_cond_wait(cond, mutex) {\
	if(0 != pthread_cond_wait(cond, mutex)){\
		printError("wait failed\n");\
	}\
}

#define Pthread_cond_signal(cond) {\
	if(0 != pthread_cond_signal(cond)){\
		printError("signal faile\n");\
	}\
}
