#include "capture.h"

void getStartTimeLog(struct timespec start_time) 
{
	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("Start Sec:%ld, Nsec:%ld\n", start_time.tv_sec, start_time.tv_nsec);
}

void getStopTimeLog(struct timespec stop_time) 
{
	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("Stop Sec:%ld, Nsec:%ld\n", stop_time.tv_sec, stop_time.tv_nsec);
}

void getDelta(struct timespec start_time, struct timespec stop_time) 
{
	struct timespec start;
	struct timespec stop;
	start = start_time;
	stop = stop_time;
	int diff_nsec = stop.tv_nsec - start.tv_nsec;
	if (diff_nsec < 0) {
		diff_nsec = 1000000000 + diff_nsec;
	}
	printf("Delta nanosec: %ld\n", diff_nsec);
	printf("Delta microsec: %ld\n", diff_nsec / 1000);
	printf("Delta millisec: %ld\n\n", diff_nsec / 1000000);
}

void idleState(struct timespec sleep_time, 
                    struct timespec remaining_time, 
                    struct timespec start_time,
                    struct timespec stop_time) 
{
	getStartTimeLog(start_time);
	nanosleep(&sleep_time, &remaining_time);
	getStopTimeLog(stop_time);
	getDelta(start_time, stop_time);
}
