#ifndef EVENT_HUB_CONFIG_H
#define EVENT_HUB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char command[256];
    char eh_host[256];
    char eh_key_name[256];
    char eh_key[512];
    char eh_name[256];
    int use_dev_emulator;
    char eh_publisher[256];
    size_t msg_count;

    // Partition receiver configuration
    char eh_consumer_group[256];
    char eh_partition_id[32];
    char eh_offset[256];      // Starting offset (e.g., "0", "-1" for oldest, "@latest" for newest)
    int64_t eh_enqueued_time; // Offset by enqueued time (epoch time in seconds)
} EventHubConfig;

// Function declarations for eh_sender.c and eh_receiver.c
int eh_sender(EventHubConfig config);
int eh_receiver(EventHubConfig config);

#ifdef __cplusplus
}
#endif

#endif // EVENT_HUB_CONFIG_H