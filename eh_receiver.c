// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"

#include "azure_c_shared_utility/socketio.h"


#include "azure_uamqp_c/uamqp.h"

#include "event_hub_config.h"  // Include the shared header

/* This sample connects to an Event Hub, authenticates using SASL PLAIN (key name/key) and then it received all messages for partition 0 */
/* Replace the below settings with your own.*/



static AMQP_VALUE on_message_received(const void* context, MESSAGE_HANDLE message)
{
    (void)context;
    (void)message;

    // printf message body using message_get_body_amqp_data_in_place
    AMQP_VALUE body_amqp_data;
    BINARY_DATA binary_data;

    int result = message_get_body_amqp_data_in_place(message, 0, &binary_data);
    // convert binary_data to string and printf
    if (result == 0)
    {
        (void)printf("Received message: %.*s\r\n", (int)binary_data.length, (char*)binary_data.bytes);
    }
    else
    {
        (void)printf("Failed to retrieve message data\r\n");
    }

    return messaging_delivery_accepted();
}

int eh_receiver(EventHubConfig config)
{
    int result;

    XIO_HANDLE sasl_io = NULL;
    CONNECTION_HANDLE connection = NULL;
    SESSION_HANDLE session = NULL;
    LINK_HANDLE link = NULL;
    MESSAGE_RECEIVER_HANDLE message_receiver = NULL;

    if (platform_init() != 0)
    {
        result = -1;
    }
    else
    {
        size_t last_memory_used = 0;
        AMQP_VALUE source;
        AMQP_VALUE target;
        SASL_PLAIN_CONFIG sasl_plain_config;
        SASL_MECHANISM_HANDLE sasl_mechanism_handle;

        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        XIO_HANDLE tls_io;
        XIO_HANDLE socket_io;
        SASLCLIENTIO_CONFIG sasl_io_config;

        gballoc_init();

        /* create SASL plain handler */
        sasl_plain_config.authcid = config.eh_key_name;
        sasl_plain_config.authzid = NULL;
        sasl_plain_config.passwd = config.eh_key;

        sasl_mechanism_handle = saslmechanism_create(saslplain_get_interface(), &sasl_plain_config);

        // KH - using emulators, we don't need to use TLS
        TLSIO_CONFIG tls_io_config = { config.eh_host , 5671 };
        // Use socketio instead of tlsio for non-TLS connections
        SOCKETIO_CONFIG socket_io_config = { config.eh_host, 5672, NULL };


        if (config.use_dev_emulator != 1) {
            // KH - using emulators, we don't need to use TLS
            /* create the TLS IO */
            tlsio_interface = platform_get_default_tlsio();
            tls_io = xio_create(tlsio_interface, &tls_io_config);

            /* create the SASL client IO using the TLS IO */
            sasl_io_config.underlying_io = tls_io;
        } else {

            /* create the Socket IO for non-TLS connection */
            const IO_INTERFACE_DESCRIPTION* socketio_interface = socketio_get_interface_description();
            socket_io = xio_create(socketio_interface, &socket_io_config);

            /* create the SASL client IO using the Socket IO */
            sasl_io_config.underlying_io = socket_io;


        }

        sasl_io_config.sasl_mechanism = sasl_mechanism_handle;
        sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config);

        /* create the connection, session and link */
        connection = connection_create(sasl_io, config.eh_host, "whatever", NULL, NULL);
        session = session_create(connection, NULL, NULL);

        /* set incoming window to 100 for the session */
        session_set_incoming_window(session, 100);

        /* listen only on partition 0 */
        char source_address[256];
        if (config.use_dev_emulator == 1) {
            snprintf(source_address, sizeof(source_address), "amqp://%s/%s/ConsumerGroups/%s/Partitions/%s", config.eh_host, config.eh_name, config.eh_consumer_group, config.eh_partition_id);
        } else {
            snprintf(source_address, sizeof(source_address), "amqps://%s/%s/ConsumerGroups/%s/Partitions/%s", config.eh_host, config.eh_name, config.eh_consumer_group, config.eh_partition_id);
        }
        printf("Connecting to %s\n", source_address);
        source = messaging_create_source(source_address);
        target = messaging_create_target("ingress-rx");
        link = link_create(session, "receiver-link", role_receiver, source, target);
        link_set_rcv_settle_mode(link, receiver_settle_mode_first);
        amqpvalue_destroy(source);
        amqpvalue_destroy(target);

        /* create a message receiver */
        message_receiver = messagereceiver_create(link, NULL, NULL);
        if ((message_receiver == NULL) ||
            (messagereceiver_open(message_receiver, on_message_received, message_receiver) != 0))
        {
            (void)printf("Cannot open the message receiver.");
            result = -1;
        }
        else
        {
            bool keep_running = true;
            while (keep_running)
            {
                size_t current_memory_used;
                size_t maximum_memory_used;
                connection_dowork(connection);

                current_memory_used = gballoc_getCurrentMemoryUsed();
                maximum_memory_used = gballoc_getMaximumMemoryUsed();

                if (current_memory_used != last_memory_used)
                {
                    (void)printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
                    last_memory_used = current_memory_used;
                }
            }

            result = 0;
        }

        messagereceiver_destroy(message_receiver);
        link_destroy(link);
        session_destroy(session);
        connection_destroy(connection);
        platform_deinit();

        (void)printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        (void)printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();
    }

    return result;
}