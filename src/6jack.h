
#ifndef __SIXJACK_H__
#define __SIXJACK_H__ 1

typedef struct Context_ {
    bool initialized;
    Filter filter;
} Context;

Context *get_sixjack_context(void);
void free_sixjack_context(void);

#endif
