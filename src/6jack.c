
#include "common.h"
#include "utils.h"
#include "msgpack-extensions.h"
#include "id-name.h"
#include "filter.h"
#include "6jack.h"

Context *sixjack_get_context(void)
{
    static Context context;
    if (context.initialized != 0) {
        return &context;
    }
    
    Filter * const filter = &context.filter;
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
    ret = posix_spawn(&filter->pid, "./example-filter.rb",
                      &file_actions, NULL, argv, environ);
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn");
        exit(1);
    }
    printf("Child = %u\n", (unsigned int) filter->pid);
        
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
    Context * const context = get_sixjack_context();
    Filter * const filter = &context->filter;

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
    
    context->initialized = 0;
}

#ifdef USE_INTERPOSERS
const struct { void *hook_func; void *real_func; } sixjack_interposers[]
__attribute__ ((section("__DATA, __interpose"))) = {
    { INTERPOSE(socket), socket }
};
#endif

