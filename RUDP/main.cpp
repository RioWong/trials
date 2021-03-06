#include <string>
#include "izSampleKit.h"
#include "izSystem.h"
#include "izThreadModel.h"

#include <enet/enet.h>

// http://enet.bespin.org/Tutorial.html

IzMain(0, 0)
{
    ::enet_initialize();

    ENetAddress addr;
    addr.host = ENET_HOST_ANY;
    addr.port = 8000;

    auto server = ::enet_host_create(
        &addr,  // the address to bind the server host to
        32,     // allow up to 32 clients and/or outgoing connections
        2,      // allow up to 2 channels to be used, 0 and 1
        0,      // assume any amount of incoming bandwidth
        0);     // assume any amount of outgoing bandwidth
    IZ_ASSERT(server != NULL);

    ENetEvent event;

    for (;;) {
        auto result = ::enet_host_service(server, &event, 0);

        if (result > 0) {
            switch (event.type) {
            case ENetEventType::ENET_EVENT_TYPE_NONE:
                break;
            case ENetEventType::ENET_EVENT_TYPE_CONNECT:
                IZ_PRINTF(
                    "A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                break;
            case ENetEventType::ENET_EVENT_TYPE_RECEIVE:
                IZ_PRINTF(
                    "A packet of length (%u) containing [%s] was received from [%s] on channel (%u).\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.peer->data,
                    event.channelID);

                // Clean up the packet now that we're done using it.
                ::enet_packet_destroy(event.packet);
                break;
            case ENetEventType::ENET_EVENT_TYPE_DISCONNECT:
                // Reset the peer's client information.
                event.peer->data = NULL;
                break;
            }
        }
    }
    
    ::enet_host_destroy(server);
    ::enet_deinitialize();

    return 0;
}
