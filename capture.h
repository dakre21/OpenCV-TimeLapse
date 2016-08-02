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

// Services callback functions declarations
void *CAPTURE_FRAME(void *thread_id);
void *MOTION_DETECTION(void *thread_id);
void *CONVERT_FRAME(void *thread_id);
void *EDIT_HDR_FRAME(void *thread_id);
void *COMPRESS_FRAME(void *thread_id);
void *SAVE_FRAME(void *thread_id);

