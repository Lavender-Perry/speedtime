#ifndef STRUCTS_H
#define STRUCTS_H

struct threadInfo {
    bool do_thread;
    pthread_mutex_t* mtx_ptr;
};

#endif // STRUCTS_H
