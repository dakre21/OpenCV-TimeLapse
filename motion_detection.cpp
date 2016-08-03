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
    Mat gray_cached, gray_new;
    while(1)
    {
        idleState(sleep_time_md, remaining_time_md, start_time_md, stop_time_md, thread_id);
        // Lock, modify file, unlock
        pthread_mutex_lock(&sem_frame);
        if (!bgr_frame.empty() && !cached_frame.empty()) 
        {
            // Convert frames to grayscale
            cvtColor(cached_frame, gray_cached, CV_BGR2GRAY);
            cvtColor(bgr_frame, gray_new, CV_BGR2GRAY);
            // Take difference and threshold between mid range color scale
            Mat result = gray_new - gray_cached;
            int count = countNonZero(result);
            // Calculate if diff is greater than 2%, if so motion detected
            double diff = count / sizeof(result);
            if (diff >= 0.02)
            {
                printf("Motion Detected\n"); // set global flag here
            }
        }
        else if (bgr_frame.empty())
        {
            // bgr_frame not allocated.. continue
            continue;
        }
        else 
        {
            // Set cached frame to bgr frame
            cached_frame = bgr_frame.clone();
        }
        pthread_mutex_unlock(&sem_frame);
    }
    return NULL;
}
