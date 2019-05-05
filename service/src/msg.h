#ifndef _MSG_H
#define _MSG_H

#include <stdint.h>
#include "common.h"

typedef struct app_struct app_t;

msg_t *query_msg(app_t *app, int from, int type);
msg_t *delete_msg(app_t *app, int from, int type);
void append_msg(app_t *app, msg_t *msg);
int accept_msg(app_t *app);

#endif
