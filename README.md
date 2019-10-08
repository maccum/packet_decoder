This repository contains a simple packet decoder. 

It takes a stream of bytes,
decodes the bytes into valid packets, and calls some callback function each time it
successfully decodes a packet.

# Execution

The `example.cpp` file contains a few tests using the library.

To run the tests:

> g++ -std=c++11 example.cpp pkt_decoder.cpp  -o example.o

> ./example.o

Alternatively, there is a make file. To use the make file, build with 
`make target` and run with `make run`. 

# Packet Decoder

The packet decoder interface can be found in `pkt_decoder.h`. The
implementation is in `pkt_decoder.cpp`.

The packet decoder stores the necessary state for processing streams
of packets in a pkt_decoder struct. 

There are three main functions for interacting with the packet decoder:

1. `pkt_decoder_create` : 

    - Creates a new packet decoder.
    - Arguments:
        - `callback` - a function that is called each time a complete packet is decoded
        - `callback_ctx` - a pointer to some context that will be passed to `callback`

2. `pkt_decoder_destroy`

    - Cleans up memory used by the decoder.
    - Arguments:
        - `decoder` - decoder to destruct

3. `pkt_decoder_write_bytes`

    - Decodes packets from a byte stream. The function can be called repeatedly to 
    process multiple streams.
    - Arguments:
        - `decoder` - the packet decoder currently being used
        - `len` - length of the byte stream to be decoded
        - `data` - pointer to the byte stream to be decoded
    - Notes:
        - Silently drops invalid packets. 
        
# Packets

Packets are expected to come in a stream of bytes.

Packets have the following constraints to be considered valid:
- Packets must start with 0x02
- Packets must end with 0x03
- Packets must be no longer than 512 bytes, including the starting and ending 
byte.
- The 0x10 byte is treated as an escape character. The byte that follows it
is in the format `0x20 | value` where `value` is the actual byte value that 
we want to add to our decoded packet. This byte value will be decoded with
the operation `value & ~0x20` and added to the decoded packet. 

Invalid packets are discarded. The packet decoder will continue processing
additional bytes in the stream after discarding an invalid packet. 