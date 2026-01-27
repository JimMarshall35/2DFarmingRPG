/*
  Match making server

  - connects using the same netcode udp protcol as peers dof

  # game server
  - game servers connect to matchmaking server and maintain an open connection
  - they send a message containing:
      - username
      - number of client slots
  - upon recieving this message matchmaking is initiated, and matchmaking is run every 100ms to account for packet loss
  - if a game client is found it is then pending for the game server connection, 
  the client is sent a package with the joinable servers IP and port
  - once a client joins, the server sends a packet to the server acknowledging it (will change this to be a constant stream)
  - once the matchmaking is complete for a server it is booted from the matchmaking server
*/

#include "netcode.h"
#include "Network.h"
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <array>
#include <bitset>
#include <assert.h>
#include <iostream>
static volatile int quit = 0;

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea, 
                                                  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4, 
                                                  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
                                                  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };

#define ZeroBytes(dst, size) memset(dst, 0, size)
struct GameServerConnection
{
    int openSlots;
    char name[64];
};

struct GameClientConnection
{
    char name[64];
    bool pending;
};

class ClientsArray
{
public:
    ClientsArray(struct netcode_server_t* server)
      :m_server(server)
    {
        m_bitset.reset();
    }

    void SetConnected(size_t i, bool bConnected)
    {
        m_clientsArray[i].m_client = bConnected ? static_cast<int>(i) : -1;
        m_bitset[i] = bConnected;
        if(!bConnected)
        {
            m_clientsArray[i].pendingClientConnections.clear();
            m_clientsArray[i].m_type = ConnectionType::Undefined;
        }
    }

    bool GetConnected(size_t i)
    {
        return m_bitset[i];
    }

    void MatchMake()
    {
        for(int i=0; i < NETCODE_MAX_CLIENTS; i++)
        {
            MatchMake(i, m_server);
        }
    }

    void MakeIntoClient(size_t i, const char* clientName)
    {
        if(m_bitset[i])
        {
            m_clientsArray[i].m_type = ConnectionType::Client;
            strcpy(m_clientsArray[i].data.client.name, clientName);
            m_clientsArray[i].data.client.pending = false;
        }
        else
        {
            printf("Client %i not connected!\n", i);
        }
    }

    void MakeIntoServer(size_t i, const char* serverName, int numSlots)
    {
        if(m_bitset[i])
        {
            m_clientsArray[i].m_type = ConnectionType::Server;
            strcpy(m_clientsArray[i].data.server.name, serverName);
            m_clientsArray[i].data.server.openSlots = numSlots;
        }
        else
        {
            printf("Client %i not connected!\n", i);
        }
    }

    void PeerJoined(size_t i)
    {
        if(m_clientsArray[i].m_type == ConnectionType::Server)
        {
            m_clientsArray[i].data.server.openSlots--;
            if(m_clientsArray[i].data.server.openSlots == 0)
            {
                netcode_server_disconnect_client(m_server, i);
                m_clientsArray[i].m_client = -1;
                m_clientsArray[i].m_type = ConnectionType::Undefined;
                m_bitset[i] = false;
            }
            for(int j : m_clientsArray[i].pendingClientConnections)
            {
              m_clientsArray[j].m_client = -1;
              m_clientsArray[j].m_type = ConnectionType::Undefined;
              m_bitset[j] = false;
              netcode_server_disconnect_client(m_server, j);
            }
        }
        else
        {
            printf("INTERNAL ERROR 2\n");
        }
    }

    void ResendPendingPackets()
    {
      for(int i=0; i<m_clientsArray.size(); i++)
      {
        if(m_bitset[i])
        {
          if(m_clientsArray[i].m_type == ConnectionType::Server)
          {
            if(m_clientsArray[i].pendingClientConnections.size())
            {
              for(int c: m_clientsArray[i].pendingClientConnections)
              {
                SendGameConnectionPacket(c, i);
              }
            }
          }
        }
      }
    }

    void SendGameConnectionPacket(int clientI, int serverI)
    {
        Connection& server = m_clientsArray[serverI];
        Connection& client = m_clientsArray[clientI];
        
        MatchMakingMessage msg;
        msg.type = MM_GameConnection;
        netcode_address_t* pAddr = netcode_server_client_address(m_server, serverI);
        pAddr->data.ipv4;
        sprintf(&msg.data.peer.address[0], "%i.%i.%i.%i:%i", 
          pAddr->data.ipv4[0],
          pAddr->data.ipv4[1],
          pAddr->data.ipv4[2],
          pAddr->data.ipv4[3],
          pAddr->port
        );

        strcpy(msg.data.peer.username, server.data.server.name);
        
        netcode_server_send_packet(m_server, clientI, (const uint8_t*)&msg, sizeof(MatchMakingMessage));
    }

private:
    enum class ConnectionType
    {
        Server,
        Client,
        Undefined
    };
    struct Connection
    {   
        int m_client = -1;
        
        ConnectionType m_type = ConnectionType::Undefined;

        /* todo: change to the C++ union that allows this to be put in the "server" struct */
        double pendingTimer = 0;
        std::vector<int> pendingClientConnections;
        union
        {
            struct GameServerConnection server;
            struct GameClientConnection client;
        }data;
    };

private:
    void JoinServer(int clientI, int serverI)
    {
        Connection& server = m_clientsArray[serverI];
        Connection& client = m_clientsArray[clientI];
        if(server.pendingClientConnections.size() || client.data.client.pending)
        {
            printf("Server has pending client connections\n");
            return;
        }
        client.data.client.pending = true;
        server.pendingClientConnections.push_back(clientI);
        netcode_address_t* pAddr = netcode_server_client_address(m_server, serverI);
        if(pAddr->type == NETCODE_ADDRESS_IPV4)
        {
            SendGameConnectionPacket(clientI, serverI);
        }
        else
        {
          printf("UNSUPPORTED IPV6 ADDRESS\n");
        }
    }

    void MatchMake(size_t i, struct netcode_server_t * server)
    {
        if(!m_bitset[i])
        {
            return;
        }
        switch(m_clientsArray[i].m_type)
        {
        case ConnectionType::Server:
            if(m_clientsArray[i].data.server.openSlots <= 0)
            {
                printf("INTERNAL ERROR 1\n");
                return;
            }
            /* hoover up any unconnected clients */
            for(int j=0; j<NETCODE_MAX_CLIENTS; j++)
            {
                if(j == i)
                {
                    continue;
                }
                Connection& con = m_clientsArray[j];
                if(m_bitset[j] && con.m_type == ConnectionType::Client)
                {
                    JoinServer(j, i);
                }
            }
            break;
        case ConnectionType::Client:
            /* try and join a server */
            for(int j=0; j<NETCODE_MAX_CLIENTS; j++)
            {
                if(j == i)
                {
                    continue;
                }
                Connection& con = m_clientsArray[j];
                if(m_bitset[j] && con.m_type == ConnectionType::Server && con.data.server.openSlots > 0)
                {
                    JoinServer(j, i);
                }
            }
            break;
        }
    }

    std::array<Connection, NETCODE_MAX_CLIENTS> m_clientsArray;
    std::bitset<NETCODE_MAX_CLIENTS> m_bitset;
    struct netcode_server_t* m_server = nullptr;
};

int main( int argc, char ** argv )
{
    std::cout << "Matchmaking server version" << std::endl;
    (void) argc;
    (void) argv;

    if ( netcode_init() != NETCODE_OK )
    {
        printf( "error: failed to initialize netcode\n" );
        return 1;
    }

    netcode_log_level( NETCODE_LOG_LEVEL_INFO );

    double time = 0.0;
    double delta_time = 1.0 / 60.0;

    printf( "[server]\n" );

    #define TEST_PROTOCOL_ID 0x1122334455667788

    char * server_address = "127.0.0.1:40000";
    if ( argc == 2 )
        server_address = argv[1];

    struct netcode_server_config_t server_config;
    netcode_default_server_config( &server_config );
    server_config.protocol_id = TEST_PROTOCOL_ID;
    memcpy( &server_config.private_key, private_key, NETCODE_KEY_BYTES );

    struct netcode_server_t * server = netcode_server_create( server_address, &server_config, time );

    if ( !server )
    {
        printf( "error: failed to create server\n" );
        return 1;
    }

    netcode_server_start( server, NETCODE_MAX_CLIENTS );

    uint8_t packet_data[NETCODE_MAX_PACKET_SIZE];
    int i;
    for ( i = 0; i < NETCODE_MAX_PACKET_SIZE; ++i )
        packet_data[i] = (uint8_t) i;

    ClientsArray clients{server};

    double counter = 0.0;

    while ( !quit )
    {
        netcode_server_update( server, time );

        for(int i=0; i< NETCODE_MAX_CLIENTS; i++)
        {
            clients.SetConnected(i, netcode_server_client_connected( server, i ));
        }

        int client_index;
        for ( client_index = 0; client_index < NETCODE_MAX_CLIENTS; ++client_index )
        {
            while ( 1 )             
            {
                int packet_bytes;
                uint64_t packet_sequence;
                void * packet = netcode_server_receive_packet( server, client_index, &packet_bytes, &packet_sequence );
                if ( !packet )
                    break;
                (void) packet_sequence;
                assert( packet_bytes == NETCODE_MAX_PACKET_SIZE );
                assert( memcmp( packet, packet_data, NETCODE_MAX_PACKET_SIZE ) == 0 );
                
                MatchMakingMessage* pMsg = static_cast<MatchMakingMessage*>(packet);

                switch(pMsg->type)
                {
                case MM_ClientSeekServer:
                    {
                        clients.MakeIntoServer(client_index, pMsg->data.clientInfo.playerInfo.username, pMsg->data.serverInfo.availableSlots);
                    }
                    break;
                case MM_ServerSeekClient:
                    {
                        clients.MakeIntoClient(client_index, pMsg->data.clientInfo.playerInfo.username);
                    }
                    break;
                case MM_ServerPeerJoined:
                    {
                        clients.PeerJoined(client_index);
                    }
                    break;
                case MM_GameConnection:
                    assert(false);
                    break;
                }

                netcode_server_free_packet( server, packet );
            }
        }

        netcode_sleep( delta_time );

        time += delta_time;
        counter += delta_time;
        if(counter > 0.1)
        {
          /* any packets lost, just matchmake again */
            clients.MatchMake();
            clients.ResendPendingPackets();
        }
    }

    if ( quit )
    {
        printf( "\nshutting down\n" );
    }

    netcode_server_destroy( server );

    netcode_term();
    
    return 0;
}
