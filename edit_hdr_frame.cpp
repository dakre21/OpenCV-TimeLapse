#include "capture.h"
#include <sys/utsname.h>
#include <fstream>

// Global shared data
extern unsigned int frame_count;
extern char *curr_file;
extern struct timespec frame_time;
// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;
extern bool motion_detected;

// Sleep attributes
// struct timespec sleep_time_hdr = {0, 900000000}; // 964ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec sleep_time_hdr = {0, 850000000}; // 964ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_hdr = {0, 0};
// Time attributes
struct timespec start_time_hdr = {0, 0}; // Start timestamp for log
struct timespec stop_time_hdr = {0, 0}; // Stop timestamp for log

void *EDIT_HDR_FRAME(void *thread_id)
{
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    FILE *file;
    while(1)
    {
        idleState(sleep_time_hdr, remaining_time_hdr, start_time_hdr, stop_time_hdr, thread_id);
        getStartTimeLog(start_time_hdr, thread_id);
        if (motion_detected == true)
        {
            // Lock, modify frame, unlock
            pthread_mutex_lock(&sem_frame);
            file = fopen(curr_file, "a"); // Tried "r+" to pre-append data to header.. did not work
            if (file != NULL)
            {
                //fseek(file, 0, SEEK_SET);
                fprintf(file, "# Frame @ %u sec, %u msec, %lu nsec\n", (unsigned)frame_time.tv_sec, (unsigned)frame_time.tv_sec / 1000000, (unsigned long)frame_time.tv_nsec);
                fprintf(file, "# %s\n", hostname);
                fclose(file);
            }
            pthread_mutex_unlock(&sem_frame);
        }
        getStopTimeLog(stop_time_hdr, thread_id);
    }
    return NULL;
}
