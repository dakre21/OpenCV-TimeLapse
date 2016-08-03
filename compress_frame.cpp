#include "capture.h"

// Global shared data
extern char *curr_file;
// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

// Sleep attributes
struct timespec sleep_time_comp = {0, 965000000}; // 965ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_comp = {0, 0};
// Time attributes
struct timespec start_time_comp = {0, 0}; // Start timestamp for log
struct timespec stop_time_comp = {0, 0}; // Stop timestamp for log

void *COMPRESS_FRAME(void *thread_id)
{
    FILE *comp_file;
    while(1)
    {
        idleState(sleep_time_comp, remaining_time_comp, start_time_comp, stop_time_comp, thread_id);
        // Lock, modify file, unlock
        pthread_mutex_lock(&sem_frame);

        pthread_mutex_unlock(&sem_frame);
    }
    return NULL;
}
