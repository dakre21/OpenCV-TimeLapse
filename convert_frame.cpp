#include "capture.h"

// Global shared data
extern IplImage* frame;
extern int frame_count;
Mat rgb_frame;
char *curr_file;

// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

// Sleep attributes
struct timespec sleep_time_conv = {0, 963000000}; // 963ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_conv = {0, 0};
// Time attributes
struct timespec start_time_conv = {0, 0}; // Start timestamp for log
struct timespec stop_time_conv = {0, 0}; // Stop timestamp for log

void *CONVERT_FRAME(void *thread_id)
{
    while(1) 
    {
        idleState(sleep_time_conv, remaining_time_conv, start_time_conv, stop_time_conv);
        if (frame != NULL)
        {
            cout << frame << endl;
            Mat bgr_frame;
            // Lock, modify frame, unlock
            pthread_mutex_lock(&sem_frame);
            // Add ppm file path
            char *extension = ".ppm";
            char file_name[96] = "storage/time_lapse-";
            char file_count_str[32];
            sprintf(file_count_str, "%d", frame_count);
            strcat(file_name, file_count_str);
            strcat(file_name, extension);
            // Convert frame from bgr to rgb
            bgr_frame = cvarrToMat(frame);
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PXM_BINARY);
            compression_params.push_back(0);
            cvtColor(bgr_frame, rgb_frame, CV_BGR2RGB); // BGR to RGB PPM ASCII 
            imwrite(file_name, rgb_frame, compression_params);
            curr_file = file_name;
            pthread_mutex_unlock(&sem_frame);
            cout << frame_count << endl;
        }
    }
    return NULL;
}
