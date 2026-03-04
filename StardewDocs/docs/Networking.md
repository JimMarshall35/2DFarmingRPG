# Networking

Work on this feature is currently underway.

## Architecture

Aims to implement a peer to peer networking architecture in which one player is the server.

I hope to implement it in a layered and game agnostic way in the engine library.
The hope is i could one day implement a completely new _type_ of game as a gameframeworklayer (and most likely set of rendering functions) and the lowest level networking library would still be useful, as I hope would everything in the src/core/ and src/input directories and the overall game framework.

The base layer of the implementation is a core engine system that establishes a game instance as a server, client, or single player through command line arguments. Servers can run as a normal game and accept client connections at any time. The clients connect to a running server. The base layer spawns a network thread (if it's not single player) that facilitates connections, the sending and recieveing of packets and an initial level of packet processing and filtering. 

Packets can be sent reliably or unreliable.
- unreliable 
    - these must be < 1200 bytes for now
- reliable
    - these can be any size
    - these will get put into the rx queue of their recipient exactly once
    - if they are > 1200 the game thread will send them as a series of reliable packets that get reassembled
    - just like martins geode, reliable packets must be acknowledged and will be continually resent until they are
        - a circular buffer of recently acknowledged packets IDs if maintained, if one arrives after it's already been acknowledged it should be ignored 

The network thread presents the game with in interface through which it can transmit and recieve these reliable and unreliable packets -  a set of threadsafe queues. It does the work of reassembling fragmented packets and implementing the reliable packet acknowledgement and resending.

Network.c presents the game thread with library through which to interact with the queues, there's a tx (transmit), rx (recieve) and connectionEvents queue, the latter queues events when clients connect and disconnect.

On top of this the Game2DLayer.c adds a higher level networking protocol. I am working out the details of this and will document them at a later stage

- The overall idea for the networking of this particular game:
    - The server will continually send updates of the world state to all clients
    - wherever it creates the appearance of a smooth game for the client, they'll be trusted on the server but that will maintain ultimate authority
        - clients trusted for their position and movement, perhaps even the result of actions like farming, but the server ensures consistency of the game world   
        - server assigns network ids to all entities, but if a client creates an entity it can assign it its best guess of a network ID and inform the server, which may correct the ID at a later time if there's a collision
            - the network ID is a number that counts up and the client has recieved its game state from the server so its quite likely its guess of the network ID will be correct
        - some of this will be on the game level and some on the engine level - it will make sense

## Core networking code core/Network.c, core/Network.c

This uses the library [netcode](https://github.com/mas-bandwidth/netcode) to maintain a persistent UDP connection

At a low level you must push data into the transmitting queue in the form of a shared pointer

Data recieved is a malloc'd pointer.

It is up to the function that dequeues the data to free it or decrement its reference, unless its a reliable packet in which case it will only be freed once its been acknowledged. The shared pointer ensures that big packets are only freed once all fragments have been acknowledged.