#include "capture.h"

// Global shared data
extern Mat bgr_frame;
// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

// Sleep attributes
struct timespec sleep_time_md = {0, 962700000}; // 965ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_md = {0, 0};
// Time attributes
struct timespec start_time_md = {0, 0}; // Start timestamp for log
struct timespec stop_time_md = {0, 0}; // Stop timestamp for log

Mat cached_frame;
void *MOTION_DETECTION(void *thread_id)
{
    Mat gray_cached, gray_new, cp_bgr_frame;
    while(1)
    {
        idleState(sleep_time_md, remaining_time_md, start_time_md, stop_time_md);
        // Lock, modify file, unlock
        pthread_mutex_lock(&sem_frame);
        if (!bgr_frame.empty() && !cached_frame.empty()) 
        {
            // Convert frames to grayscale
            cp_bgr_frame = bgr_frame;
            cvtColor(cached_frame, gray_cached, CV_BGR2GRAY);
            cvtColor(cp_bgr_frame, gray_new, CV_BGR2GRAY);
            // Take difference and threshold between mid range color scale
            Mat result = gray_new - gray_cached;
            int count = countNonZero(result);
            //double threshold_value = threshold(result, result, 50, 255, CV_THRESH_BINARY);
            // Calculate if diff is greater than 2%, if so motion detected
            double diff = count / 307200;
            if (diff >= 0.02)
            {
                printf("Motion Detected\n"); // set global flag here
            }
        }
        // Set cached frame to bgr frame
        cached_frame = bgr_frame;
        pthread_mutex_unlock(&sem_frame);
    }
    return NULL;
}
