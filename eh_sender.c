
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/azure_base64.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tickcounter.h"

#include "azure_c_shared_utility/socketio.h"


#include "azure_uamqp_c/uamqp.h"

#include "event_hub_config.h"  // Include the shared header

#if _WIN32
#include "windows.h"
#endif

/* This sample connects to an Event Hub, authenticates using SASL MSSBCBS (SAS token given by a put-token) and sends 1 message to the EH specifying a publisher ID */
/* The SAS token is generated based on the policy name/key */
/* Replace the below settings with your own.*/


static unsigned int sent_messages = 0;
static bool auth = false;

static void on_message_send_complete(void* context, MESSAGE_SEND_RESULT send_result, AMQP_VALUE delivery_state)
{
    (void)send_result;
    (void)context;
    (void)delivery_state;

    sent_messages++;
}

int eh_sender(EventHubConfig config)
{
    int result;

    if (platform_init() != 0)
    {
        result = -1;
    }
    else
    {
        XIO_HANDLE sasl_io;
        CONNECTION_HANDLE connection;
        SESSION_HANDLE session;
        LINK_HANDLE link;
        MESSAGE_SENDER_HANDLE message_sender;
        MESSAGE_HANDLE message;
        SASL_PLAIN_CONFIG sasl_plain_config = {  config.eh_key_name, config.eh_key, NULL };

        // KH - using emulators, we don't need to use TLS
        TLSIO_CONFIG tls_io_config = { config.eh_host , 5671 };
        // Use socketio instead of tlsio for non-TLS connections
        SOCKETIO_CONFIG socket_io_config = { config.eh_host, 5672, NULL };

        const IO_INTERFACE_DESCRIPTION* tlsio_interface;
        SASLCLIENTIO_CONFIG sasl_io_config;        
        AMQP_VALUE source;
        AMQP_VALUE target;

        size_t last_memory_used = 0;
        SASL_MECHANISM_HANDLE sasl_mechanism_handle = saslmechanism_create(saslplain_get_interface(), &sasl_plain_config);
        XIO_HANDLE tls_io;
        XIO_HANDLE socket_io;
       
        unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
        BINARY_DATA binary_data;

        gballoc_init();

        
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
        connection = connection_create(sasl_io, config.eh_host, "some", NULL, NULL);
        session = session_create(connection, NULL, NULL);
        session_set_incoming_window(session, 2147483647);
        session_set_outgoing_window(session, 65536);


        source = messaging_create_source("ingress");
        char target_address[256];
        if (config.use_dev_emulator == 1) {
            // using the event hub emulator, the target should be the event hub name
            snprintf(target_address, sizeof(target_address), "amqp://%s/%s/publishers/%s", config.eh_host, config.eh_name, config.eh_publisher);
            
        } else {
            snprintf(target_address, sizeof(target_address), "amqps://%s/%s/publishers/%s", config.eh_host, config.eh_name, config.eh_publisher);
        }
        target = messaging_create_target(target_address);
        link = link_create(session, "sender-link", role_sender, source, target);
        link_set_snd_settle_mode(link, sender_settle_mode_settled);
        (void)link_set_max_message_size(link, 65536);

        amqpvalue_destroy(source);
        amqpvalue_destroy(target);

        message = message_create();

        binary_data.bytes = hello;
        binary_data.length = sizeof(hello);
        message_add_body_amqp_data(message, binary_data);

        /* create a message sender */
        message_sender = messagesender_create(link, NULL, NULL);
        if (messagesender_open(message_sender) == 0)
        {
            uint32_t i;
            bool keep_running = true;
            tickcounter_ms_t start_time;
            TICK_COUNTER_HANDLE tick_counter = tickcounter_create();

            if (tickcounter_get_current_ms(tick_counter, &start_time) != 0)
            {
                (void)printf("Error getting start time\r\n");
            }
            else
            {
                for (i = 0; i < config.msg_count; i++)
                {
                    /* timeout if it takes longer than 10s */
                    (void)messagesender_send_async(message_sender, message, on_message_send_complete, message, 10000);
                }

                message_destroy(message);

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

                    if (sent_messages == config.msg_count)
                    {
                        break;
                    }
                }

                {
                    tickcounter_ms_t end_time;
                    if (tickcounter_get_current_ms(tick_counter, &end_time) != 0)
                    {
                        (void)printf("Error getting end time\r\n");
                    }
                    else
                    {
                        (void)printf("Send %u messages in %lu ms: %.02f msgs/sec\r\n", (unsigned int)config.msg_count, (unsigned long)(end_time - start_time), (float)config.msg_count / ((float)(end_time - start_time) / 1000));
                    }
                }
            }

            tickcounter_destroy(tick_counter);
        }

        messagesender_destroy(message_sender);
        link_destroy(link);
        session_destroy(session);
        connection_destroy(connection);

        xio_destroy(sasl_io);

        if (config.use_dev_emulator == 1) {
            xio_destroy(socket_io);
        } else {
            xio_destroy(tls_io);
        }


        saslmechanism_destroy(sasl_mechanism_handle);
        platform_deinit();

        (void)printf("Max memory usage:%lu\r\n", (unsigned long)gballoc_getCurrentMemoryUsed());
        (void)printf("Current memory usage:%lu\r\n", (unsigned long)gballoc_getMaximumMemoryUsed());

        gballoc_deinit();

        result = 0;
    }

    return result;
}
