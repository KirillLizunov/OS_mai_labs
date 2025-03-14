#include <zmq.h>
#include <stdio.h>

int main() {
    void *context = zmq_ctx_new();
    if (context) {
        printf("ZeroMQ context created successfully!\n");
        zmq_ctx_destroy(context);
    } else {
        printf("Failed to create ZeroMQ context.\n");
    }
    return 0;
}