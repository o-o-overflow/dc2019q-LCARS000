#include "msg.h"
#include "mon.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

msg_t *query_msg(app_t *app, int from, int type) {
    for (msg_t *msg = app->msg; msg != NULL; msg = msg->next) {
        if ((from == -1 || from == msg->from) &&
                (type == -1 || type == msg->type)) {
            return msg;
        }
    }
    return NULL;
}

msg_t *delete_msg(app_t *app, int from, int type) {
    for (msg_t *msg = app->msg, *prev = NULL; msg != NULL; msg = msg->next) {
        if ((from == -1 || from == msg->from) &&
                (type == -1 || type == msg->type)) {
            if (prev == NULL) {
                app->msg = msg->next;
            } else {
                prev->next = msg->next;
            }
            return msg;
        }
        prev = msg;
    }
    return NULL;
}

void append_msg(app_t *app, msg_t *msg) {
    if (app->msg == NULL) {
        app->msg = msg;
        return ;
    }
    for (msg_t *tail = app->msg; ; tail = tail->next) {
        if (tail->next == NULL) {
            tail->next = msg;
            return ;
        }
    }
}

int accept_msg(app_t *app) {
    if (app->cur_req.no != REQ_WAIT || app->state != STATE_BUSY) {
        return -1;
    }
    int from = app->cur_req.a;
    int type = app->cur_req.b;
    msg_t *msg = delete_msg(app, from, type);
    if (msg == NULL) {
        // no messages to reply
        return -1;
    }
    int len = app->cur_req.d;
    if (len > 0x10) {
        len = 0x10;
    }
    memcpy(PARAM_FOR(app->id) + app->cur_req.c, &msg->from, len);
    free(msg);
    app->cur_req.no = -1;
    app->state = STATE_IDLE;
    int res = 0;
    if (write(app->tx, &res, sizeof(res)) != sizeof(res)) {
        perror("write");
        return -1;
    }
    return 0;
}
