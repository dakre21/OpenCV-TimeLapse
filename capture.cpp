/*
 *  Lab 6: Motion Detection Time Lapse 
 *  Author: David Akre 
 *  Date: 7/31/16
 */
#include "capture.h"

using namespace cv;
using namespace std;

// Define resolution of captured image frame
#define HRES 640
#define VRES 480
// Change these for num of threads & service names
#define EXECUTIVE_SERVICE_ID 1
#define CAPTURE_FRAME_ID 2
#define MOTION_DETECTION_ID 3
#define CONVERT_FRAME_ID 4
#define EDIT_HDR_FRAME_ID 5
#define COMPRESS_FRAME_ID 6
#define SAVE_FRAME_ID 7
#define NUM_THREADS 7

// Forward declaration of thread attributes 
pthread_t threads[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
struct sched_param rt_param[NUM_THREADS];
int rt_max_prio, rt_min_prio;

// Foward declaration of timespec struct
struct timespec frame_time;
double curr_frame_time, prev_frame_time;

// Set up affinity info
int num_of_cpus;
cpu_set_t cpu_set; // Set of cpu sets
cpu_set_t thread_cpu; 

void *CAPTURE_FRAME(void *thread_id)
{
    cvNamedWindow("Motion Detection Time Lapse", CV_WINDOW_AUTOSIZE);
    CvCapture* capture = cvCreateCameraCapture(0);
    IplImage* frame;

    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

    while(1)
    {
        frame = cvQueryFrame(capture);
     
        if(!frame) break;

        cvShowImage("Motion Detection Time Lapse", frame);

        char c = cvWaitKey(33);
        if( c == 27 ) break;
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow("Motion Detection Time Lapse");
    return NULL;
}

void *EXECUTIVE_SERVICE(void *thread_id)
{
    int i, core_id, rc;
    for (i = 0; i < (NUM_THREADS - 1); i++) 
    {
        CPU_ZERO(&thread_cpu);
        if (i >= num_of_cpus)
        {
            core_id = (i % num_of_cpus) + 1; // Unimpede Core 0 (highest prio service)
        } 
        else 
        {
            core_id = i % num_of_cpus;
        }
        printf("Setting thread %d to core %d\n", i, core_id);
        CPU_SET(core_id, &thread_cpu); // Add cpu to thread_cpu struct
        rc = pthread_attr_init(&rt_sched_attr[i]);
        rc = pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
        rc = pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO); 
        rc = pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &thread_cpu);

        if (rc) 
        {
            printf("ERROR: Setting up pthread attributes rc is %d\n", rc);
            perror(NULL);
            exit(-1);
        }

        rt_param[i].sched_priority = rt_max_prio -i - 1;
        pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);
        
        if (i == 0)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       CAPTURE_FRAME, // thread function entry point
                       (void *)CAPTURE_FRAME_ID); // parameters to pass in
        }
        else if (i == 1)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       MOTION_DETECTION, // thread function entry point
                       (void *)MOTION_DETECTION_ID); // parameters to pass in
        }
        else if (i == 2)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       CONVERT_FRAME, // thread function entry point
                       (void *)CONVERT_FRAME_ID); // parameters to pass in
        }
        else if (i == 3)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       EDIT_HDR_FRAME, // thread function entry point
                       (void *)EDIT_HDR_FRAME_ID); // parameters to pass in
        }
        else if (i == 4)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       COMPRESS_FRAME, // thread function entry point
                       (void *)COMPRESS_FRAME_ID); // parameters to pass in
        }
        else if (i == 5)
        {
            rc = pthread_create(&threads[i],   // pointer to thread descriptor
                       &rt_sched_attr[0],     // use default attributes
                       SAVE_FRAME, // thread function entry point
                       (void *)SAVE_FRAME_ID); // parameters to pass in
        }

        if (rc) 
        {
            printf("ERROR: pthread_create() rc is %d\n", rc);
            perror(NULL);
            exit(-1);
        }
    }
    return NULL;
}

void set_up_affinity(void)
{
    // Forward declaration of core affinity 
    int i;

    CPU_ZERO(&cpu_set); // Clears cpu_set_t 
    num_of_cpus = get_nprocs_conf(); // Set num of CPUs

    for(i = 0; i < num_of_cpus; i++)
    {
        CPU_SET(i, &cpu_set); // Add cpu to set
    }
        
}

int main(void)
{
    // Forward declaration of return code
    int rc, i; 
    
    set_up_affinity(); // Function call pin cpu & set affinity
    // Create executive service thread for SCHED_FIFO to kick off tasks
    rt_max_prio = sched_get_priority_max(SCHED_FIFO); // Give ES max prio
    rt_min_prio = sched_get_priority_min(SCHED_FIFO); // Give ES min prio

    pthread_attr_init(&rt_sched_attr[0]); // Function initializes thread attributes
    pthread_attr_setinheritsched(&rt_sched_attr[0], PTHREAD_EXPLICIT_SCHED); // Function sets inherit-scheduler attributes setup by attr object
    pthread_attr_setschedpolicy(&rt_sched_attr[0], SCHED_FIFO); // Function sets scheduling policy attribute of thread attributes - SCHED_FIFO

    rt_param[0].sched_priority = sched_get_priority_max(SCHED_FIFO); // Get the max sched prio
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]); // Function sets the scheduling parameter attributes of the thread attributes object of attr
    rc = pthread_create(&threads[0],
                   &rt_sched_attr[0], 
                   EXECUTIVE_SERVICE, 
                   (void *)EXECUTIVE_SERVICE_ID
                   ); // Create new thread

    if (rc) 
    {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
    }

    for(i = 0; i < NUM_THREADS; i++)
    {
        if(pthread_join(threads[i], NULL) == 0) 
        {
          printf("Process thread done\n");
        }
        else 
        {
          perror("Process thread error\n");
        }
    }

    pthread_exit(NULL);
};
