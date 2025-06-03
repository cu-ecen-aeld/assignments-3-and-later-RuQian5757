#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    // Cast the thread parameter to thread_data structure
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Wait for the specified time before obtaining the mutex
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // Attempt to lock the mutex
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Failed to lock mutex: %d", rc);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Wait for the specified time while holding the mutex
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // Unlock the mutex
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        ERROR_LOG("Failed to unlock mutex: %d", rc);
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Mark the thread as successfully completed
    thread_func_args->thread_complete_success = true;

    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    // Allocate memory for thread_data structure
    struct thread_data* thread_data = (struct thread_data*) malloc(sizeof(struct thread_data));
    if (thread_data == NULL) {
        ERROR_LOG("Failed to allocate memory for thread_data");
        return false;
    }

    // Initialize thread_data fields
    thread_data->mutex = mutex;
    thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data->wait_to_release_ms = wait_to_release_ms;
    thread_data->thread_complete_success = false;

    // Create the thread, passing threadfunc as the entry point and thread_data as the argument
    int rc = pthread_create(thread, NULL, threadfunc, (void*) thread_data);
    if (rc != 0) {
        ERROR_LOG("Failed to create thread: %d", rc);
        free(thread_data); // Free allocated memory on failure
        return false;
    }

    return true;
}
