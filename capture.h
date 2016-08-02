#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

// Services callback functions declarations
void *CAPTURE_FRAME(void *thread_id);
void *MOTION_DETECTION(void *thread_id);
void *CONVERT_FRAME(void *thread_id);
void *EDIT_HDR_FRAME(void *thread_id);
void *COMPRESS_FRAME(void *thread_id);
void *SAVE_FRAME(void *thread_id);

// Helper time keeper
void idleState(struct timespec sleep_time, 
                    struct timespec remaining_time, 
                    struct timespec start_time,
                    struct timespec stop_time);

