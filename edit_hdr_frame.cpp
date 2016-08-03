#include "capture.h"
#include <fstream>

// Global shared data
extern int frame_count;
extern char *curr_file;
extern struct timespec frame_time;
// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

// Sleep attributes
struct timespec sleep_time_hdr = {0, 964000000}; // 964ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_hdr = {0, 0};
// Time attributes
struct timespec start_time_hdr = {0, 0}; // Start timestamp for log
struct timespec stop_time_hdr = {0, 0}; // Stop timestamp for log

// File attributes
FILE *file;

void *EDIT_HDR_FRAME(void *thread_id)
{
    while(1)
    {
        idleState(sleep_time_hdr, remaining_time_hdr, start_time_hdr, stop_time_hdr);
        // Lock, modify frame, unlock
        pthread_mutex_lock(&sem_frame);
        file = fopen(curr_file, "r+");
        if (file != NULL)
        {
            fseek(file, 0, SEEK_SET);
            fprintf(file, "# Frame @ %u sec, %u msec, %lu nsec\n\n", (unsigned)frame_time.tv_sec, (unsigned)frame_time.tv_sec / 1000000, (unsigned long)frame_time.tv_nsec);
            fclose(file);
        }
        pthread_mutex_unlock(&sem_frame);
    }
    return NULL;
}
