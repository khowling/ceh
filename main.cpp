#include <iostream>
#include <cstring>
#include "event_hub_config.h"


static void print_usage(void) {
    printf("Usage: ceh  [send | receive | both] [options]\n");
    printf("Options:\n");
    printf("  --host xxx.servicebus.windows.net  Event Hub host (default: 127.0.0.1)\n");
    printf("  --key-name KEY_NAME                SAS key name (default: RootManageSharedAccessKey)\n");
    printf("  --key KEY                          SAS key value\n");
    printf("  --name HUB_NAME                    Event Hub name (default: eh1)\n");
    printf("  --emulator VALUE                   Use development emulator (1=yes, 0=no, default: 1)\n");
    printf("  --publisher ID                     Publisher ID (default: test_publisher)\n");

    printf("  --count COUNT                      Sender: Number of messages to send (default: 1)\n");

    printf("  --consumer-group NAME              Receiver: Consumer group name (default: cg1)\n");
    printf("  --partition ID                     Receiver: Partition ID to listen to (default 1)\n");
    printf("  --offset OFFSET                    Receiver: Starting offset (@latest, -1 for oldest, or specific offset)\n");
    printf("  --enqueued-time TIMESTAMP          Receiver: Start from specified enqueued time (unix timestamp)\n");

    printf("  --help                             Display this help message\n");
}


static void set_default_config(EventHubConfig* config) {
    strcpy(config->command, "both");
    strcpy(config->eh_host, "127.0.0.1");
    strcpy(config->eh_key_name, "RootManageSharedAccessKey");
    strcpy(config->eh_key, "SAS_KEY_VALUE");
    strcpy(config->eh_name, "eh1");
    config->use_dev_emulator = 1;
    strcpy(config->eh_publisher, "test_publisher");
    config->msg_count = 1;

    // Set partition receiver defaults
    strcpy(config->eh_consumer_group, "cg1");
    strcpy(config->eh_partition_id, "1");
    strcpy(config->eh_offset, "@latest");
    config->eh_enqueued_time = -1; // No filter by time by default
}

static int parse_command_line(int argc, char** argv, EventHubConfig* config) {

    set_default_config(config);

    if (argc == 1) {
        print_usage();
        return -1;
    } else {
        if (strcmp(argv[1], "send") == 0 || strcmp(argv[1], "receive") == 0 || strcmp(argv[1], "both") == 0) {
            strcpy(config->command, argv[1]);
        } else {
            printf("Unknown command: %s\n", argv[1]);
            print_usage();
            return -1;
        }
    }
    
    for (int i = 2; i < argc; i++) {
        
        if (strcmp(argv[i], "--help") == 0) {
            print_usage();
            return -1;
        } else if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            config->use_dev_emulator = 0;
            strcpy(config->eh_host, argv[++i]);
        } else if (strcmp(argv[i], "--key-name") == 0 && i + 1 < argc) {
            strcpy(config->eh_key_name, argv[++i]);
        } else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
            strcpy(config->eh_key, argv[++i]);
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            strcpy(config->eh_name, argv[++i]);
        } else if (strcmp(argv[i], "--emulator") == 0 && i + 1 < argc) {
            config->use_dev_emulator = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--publisher") == 0 && i + 1 < argc) {
            strcpy(config->eh_publisher, argv[++i]);
        } else if (strcmp(argv[i], "--count") == 0 && i + 1 < argc) {
            config->msg_count = (size_t)atoi(argv[++i]);
        }  // Add the new partition receiver options
        else if (strcmp(argv[i], "--consumer-group") == 0 && i + 1 < argc) {
            strcpy(config->eh_consumer_group, argv[++i]);
        } else if (strcmp(argv[i], "--partition") == 0 && i + 1 < argc) {
            strcpy(config->eh_partition_id, argv[++i]);
        } else if (strcmp(argv[i], "--offset") == 0 && i + 1 < argc) {
            strcpy(config->eh_offset, argv[++i]);
        } else if (strcmp(argv[i], "--enqueued-time") == 0 && i + 1 < argc) {
            config->eh_enqueued_time = atoll(argv[++i]);
        }else {
            printf("Unknown option: %s\n", argv[i]);
            print_usage();
            return -1;
        }
    }
    
    // print to console the configuration
    printf("Configuration:\n");
    printf("  Command: %s\n", config->command); 
    printf("  Event Hub Host: %s\n", config->eh_host);
    printf("  Event Hub Key Name: %s\n", config->eh_key_name);

    printf("  Event Hub Name: %s\n", config->eh_name);
    printf("  Use Dev Emulator: %d\n", config->use_dev_emulator);
    printf("  Event Hub Publisher: %s\n", config->eh_publisher);
    printf("  Message Count: %zu\n", config->msg_count);
    printf("  Consumer Group: %s\n", config->eh_consumer_group);
    printf("  Partition ID: %s\n", config->eh_partition_id);
    printf("  Offset: %s\n", config->eh_offset);
    printf("  Enqueued Time: %ld\n", config->eh_enqueued_time);


    return 0;
}

int main(int argc, char** argv) {
    std::cout << "Event Hub Tester!" << std::endl;

    EventHubConfig config;
    if (parse_command_line(argc, argv, &config) != 0) {
        return -1;
    }

    if (strcmp(config.command, "send") == 0 || strcmp(config.command, "both") == 0) {
        eh_sender(config);
    }

    if (strcmp(argv[1], "receive") == 0 || strcmp(config.command, "both") == 0) {
        eh_receiver(config);
    }

    return 0;
 
}