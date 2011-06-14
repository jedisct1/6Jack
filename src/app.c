
#include "common.h"
#include "utils.h"
#include "msgpack-extensions.h"
#include "id-name.h"
#include "filter.h"

AppContext *sixjack_get_context(void)
{
    static AppContext context;
    
    if (context.initialized != 0) {
        return &context;
    }
    setenv("SIXJACK_BYPASS", "BYPASS", 0);    
    context.pid = getpid();
    context.log_fd = -1;
    if (hooks_init() != 0) {
        return NULL;
    }
    Filter * const filter = malloc(sizeof *context.filter);
    if (filter == NULL) {
        return NULL;
    }
    context.filter = filter;
    int ret;
    char *argv[] = { (char *) "example-filter", NULL };

    upipe_init(&filter->upipe_stdin);
    upipe_init(&filter->upipe_stdout);
    
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions,
                                     filter->upipe_stdin.fd_read, 0);
    posix_spawn_file_actions_adddup2(&file_actions,
                                     filter->upipe_stdout.fd_write, 1);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdin.fd_read);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdin.fd_write);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdout.fd_read);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdout.fd_write);
    const char * const filter_script = getenv("SIXJACK_FILTER");
    if (filter_script == NULL) {
        fputs("No filters specified\n", stderr);
        exit(1);
    }
    ret = posix_spawn(&filter->pid, filter_script,
                      &file_actions, NULL, argv, environ);
    unsetenv("SIXJACK_BYPASS");
    if (ret != 0) {
        errno = ret;
        fprintf(stderr, "Unable to start filter [%s]: [%s]\n", filter_script,
                strerror(errno));
        exit(1);
    }
    posix_spawn_file_actions_destroy(&file_actions);    
    filter->msgpack_sbuffer = msgpack_sbuffer_new();
    filter->msgpack_packer = msgpack_packer_new(filter->msgpack_sbuffer,
                                                msgpack_sbuffer_write);
    msgpack_unpacker_init(&filter->msgpack_unpacker,
                          FILTER_UNPACK_BUFFER_SIZE);
    msgpack_unpacked_init(&filter->message);
    context.initialized = 1;
    atexit(sixjack_free_context);
    
    return &context;
}

void sixjack_free_context(void)
{
    AppContext * const context = sixjack_get_context();
    Filter * const filter = context->filter;
    assert(filter != NULL);
    
    if (filter->msgpack_sbuffer != NULL) {
        msgpack_sbuffer_free(filter->msgpack_sbuffer);
        filter->msgpack_sbuffer = NULL;
    }
    if (filter->msgpack_packer != NULL) {
        msgpack_packer_free(filter->msgpack_packer);
        filter->msgpack_packer = NULL;
    }
    msgpack_unpacker_destroy(&filter->msgpack_unpacker);   
    msgpack_unpacked_destroy(&filter->message);
    
    if (context->log_fd != -1) {
        close(context->log_fd);
        context->log_fd = -1;
    }
    free(context->filter);
    context->filter = NULL;
    
    context->initialized = 0;
}
