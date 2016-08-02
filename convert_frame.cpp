#include "capture.h"

// Global shared data
extern IplImage* frame;
Mat rgb_frame;

// Global Mutex declarations
extern pthread_mutex_t sem_frame;
extern pthread_mutexattr_t mutex_attr;

void *CONVERT_FRAME(void *thread_id)
{
    Mat bgr_frame;
    // Lock, modify frame, unlock
    pthread_mutex_lock(&sem_frame);
    bgr_frame = cvarrToMat(frame);
    cvtColor(bgr_frame, rgb_frame, CV_BGR2RGB); // BGR to RGB PPM ASCII 
    pthread_mutex_unlock(&sem_frame);

    return NULL;
}
