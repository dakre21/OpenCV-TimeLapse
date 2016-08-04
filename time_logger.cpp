#include "capture.h"

void getStartTimeLog(struct timespec start_time, void *thread_id) 
{
	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("Service %d Start Sec:%ld, Nsec:%ld\n", (int) thread_id, start_time.tv_sec, start_time.tv_nsec);
}

void getStopTimeLog(struct timespec stop_time, void *thread_id) 
{
	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("Service %d Stop Sec:%ld, Nsec:%ld\n", (int) thread_id, stop_time.tv_sec, stop_time.tv_nsec);
}

void idleState(struct timespec sleep_time, 
                    struct timespec remaining_time, 
                    struct timespec start_time,
                    struct timespec stop_time,
                    void *thread_id) 
{
        printf("Service %d yielding CPU\n", (int)thread_id);
	nanosleep(&sleep_time, &remaining_time);
}
