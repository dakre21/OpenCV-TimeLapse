/*
 *  Lab 6: Motion Detection Time Lapse 
 *  Author: David Akre 
 *  Date: 7/31/16
 *  Note: Default execution includes motion detection and compression functionality.
 *  To get 1Hz predictable response uncomment out sleep times, motion detection
 *  and compression services; as well as flipping the global motion detected flag to
 *  false
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
#define NUM_THREADS 7

// Forward declaration of thread attributes 
pthread_t threads[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
struct sched_param rt_param[NUM_THREADS];
int rt_max_prio, rt_min_prio;

// Foward declaration of timespec struct
struct timespec frame_time;
double curr_frame_time, prev_frame_time;

// Sleep attributes
//struct timespec sleep_time_cap = {0, 768000000}; // 962.5ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec sleep_time_cap = {0, 668000000}; // 962.5ms (~30 sec for fps to drop, jitter about +-15ms)
struct timespec remaining_time_cap = {0, 0};
// Time attributes
struct timespec start_time_cap = {0, 0}; // Start timestamp for log
struct timespec stop_time_cap = {0, 0}; // Stop timestamp for log

// Set up affinity info
int num_of_cpus;
cpu_set_t cpu_set; // Set of cpu sets
cpu_set_t thread_cpu; 

// Global shared data
Mat bgr_frame;
unsigned int frame_count;

// Global Mutex declarations
pthread_mutex_t sem_frame;
pthread_mutexattr_t mutex_attr;

// Entry point for capture frame service = S1 (highest prio)
void *CAPTURE_FRAME(void *thread_id)
{
    // used to compute running averages for single camera frame rates
    double ave_framedt = 0.0, ave_frame_rate = 0.0, fc = 0.0, framedt = 0.0;
    frame_count = 0;

    // Set up image capture service
    cvNamedWindow("Motion Detection Time Lapse", CV_WINDOW_AUTOSIZE);
    CvCapture* capture = cvCreateCameraCapture(0); 
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    IplImage* frame;
    while(1)
    {
        getStartTimeLog(start_time_cap, thread_id);
        // Lock, acquire frame, unlock
        pthread_mutex_lock(&sem_frame);
        frame = cvQueryFrame(capture);
        // Convert frame from bgr to rgb
        bgr_frame = cvarrToMat(frame);
        cout << frame_count << endl;
        pthread_mutex_unlock(&sem_frame);
        if(!frame) return NULL;
        else {
           // Start getting real time timestamps for fps
           clock_gettime(CLOCK_REALTIME, &frame_time);
           curr_frame_time = ((double)frame_time.tv_sec * 1000.0) + 
                                ((double)((double)frame_time.tv_nsec / 1000000.0));
           frame_count++;

           if(frame_count > 2) {
                 fc = (double)frame_count;
                 ave_framedt = ((fc-1.0)*ave_framedt + framedt)/fc;
                 ave_frame_rate = 1.0/(ave_framedt/1000.0);
           }
        }

        cvShowImage("Motion Detection Time Lapse", frame);
        printf("Frame @ %u sec, %lu nsec, dt=%5.2lf msec, avedt=%5.2lf msec, rate=%5.2lf fps\n", (unsigned)frame_time.tv_sec, (unsigned long)frame_time.tv_nsec, framedt, ave_framedt, ave_frame_rate);
        getStopTimeLog(stop_time_cap, thread_id);
        char c = cvWaitKey(10);
        idleState(sleep_time_cap, remaining_time_cap, start_time_cap, stop_time_cap, thread_id);
        framedt = curr_frame_time - prev_frame_time;
        prev_frame_time = curr_frame_time;
        if( c == 27 ) return NULL;
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow("Motion Detection Time Lapse");
    return NULL;
}

// Entry point for executive service to kick off services
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

        if (rc) 
        {
            printf("ERROR: pthread_create() rc is %d\n", rc);
            perror(NULL);
            exit(-1);
        }
    }
    return NULL;
}

// Helper function to set affinity for CPU processors
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

// Main entry point to Time Lapse program-- spawn executive service
int main(void)
{
    // Forward declaration of return code
    int rc, i; 
    
    // Initialize mutex
    pthread_mutex_init(&sem_frame, NULL); // Initialize mutex

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

    // Error checking for pthread creation
    if (rc) 
    {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror(NULL);
       exit(-1);
    }

    // Loop through exeucting threads to join in execution upon program exit
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
