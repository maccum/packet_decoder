#ifndef PKT_DECODER_H_INCLUDED
#define PKT_DECODER_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// maximum packet length, including starting and ending bytes
#define MAX_DECODED_PACKET_LEN (512)

// Packet decoder
typedef struct pkt_decoder pkt_decoder_t;

// Function signature for a callback used by pkt_decoder_create
typedef void (*pkt_read_fn_t)(void *ctx, size_t len, const uint8_t *data);

// Constructor for a pkt_decoder
pkt_decoder_t *pkt_decoder_create(pkt_read_fn_t callback, void *callback_ctx);

// Destructor for a pkt_decoder
void pkt_decoder_destroy(pkt_decoder_t *decoder);

// Called on incoming, un-decoded bytes to be translated into packets
void pkt_decoder_write_bytes(pkt_decoder_t *decoder, size_t len,
                             const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif  // PKT_DECODER_H_INCLUDED
