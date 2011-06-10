
#ifndef __SIXJACK_H__
#define __SIXJACK_H__ 1

typedef struct Context_ {
    bool initialized;
    Filter filter;
} Context;

Context *sixjack_get_context(void);
void sixjack_free_context(void);

#endif
