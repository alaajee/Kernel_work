#pragma once

#include <linux/workqueue.h>
#include <net/sock.h>

struct workqueue_struct *client_wq;

int client_handle(struct socket *client_socket);