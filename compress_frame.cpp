#include "capture.h"
#include <zlib.h>
#include <limits.h>

#define BUFF_SIZE 1024
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
    gzFile out_file;
    char buff[BUFF_SIZE] = { 0 };
    char new_file_name[96] = { 0 };
    size_t bytes_read = 0;
    int ret;

    while(1)
    {
        idleState(sleep_time_comp, remaining_time_comp, start_time_comp, stop_time_comp, thread_id);
        // Lock, modify file, unlock
        pthread_mutex_lock(&sem_frame);
        if (curr_file != NULL) 
        {
            comp_file = fopen(curr_file, "r");
            if (comp_file != NULL)
            {
                sprintf(new_file_name, "%s.gz", curr_file);
                out_file = gzopen(new_file_name, "wb");
                if (out_file != NULL)
                {
                    bytes_read = fread(buff, 1, BUFF_SIZE, comp_file);
                    while(bytes_read > 0)
                    {
                        int num_bytes_written = gzwrite(out_file, buff, bytes_read);
                        if (num_bytes_written == 0)
                        {
                            break;
                        }
                        bytes_read = fread(buff, 1, BUFF_SIZE, comp_file);
                    }
                    ret = remove(curr_file);
                    if (ret != 0)
                    {
                        printf("ERROR: Could not remove ppm file\n");
                    }
                }  
            }
        }
        fclose(comp_file);
        gzclose(out_file);
        pthread_mutex_unlock(&sem_frame);
    }
    return NULL;
}
