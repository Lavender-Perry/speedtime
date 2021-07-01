#ifndef UTILS_H
#define UTILS_H

void putsNoNewline(const char* str);

struct threadInfo {
    bool do_thread;
    pthread_mutex_t* mtx_ptr;
};

#endif // UTILS_H
