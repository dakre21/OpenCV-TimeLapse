#include "capture.h"

// Global shared data
extern int frame_count;

// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

// Sleep attributes
struct timespec sleep_time_hdr = {0, 962500000}; // 962.5ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_hdr = {0, 0};
// Time attributes
struct timespec start_time_hdr = {0, 0}; // Start timestamp for log
struct timespec stop_time_hdr = {0, 0}; // Stop timestamp for log

void *EDIT_HDR_FRAME(void *thread_id)
{
    while(1)
    {
        idleState(sleep_time_hdr, remaining_time_hdr, start_time_hdr, stop_time_hdr);
        
    }
    return NULL;
}
