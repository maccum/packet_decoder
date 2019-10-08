#include <stdio.h>
#include <cstddef>

#include "pkt_decoder.h"

void start_new_packet(pkt_decoder_t *decoder);
void end_packet(pkt_decoder_t *decoder);
void add_byte_to_packet(pkt_decoder_t *decoder, uint8_t value);
void reset_packet(pkt_decoder_t *decoder);

/**
 * Packet decoder.
 * Stores the part of the packet that we have decoded so far.
 */
typedef struct pkt_decoder {
  // Constructor
  pkt_decoder(pkt_read_fn_t cbk, void *cbk_ctx) {
    callback = cbk;
    callback_ctx = cbk_ctx;
    current_packet = new uint8_t[MAX_DECODED_PACKET_LEN];
    packet_idx = 0;
    stx = false;
    dle = false;
  }

  // Destructor
  ~pkt_decoder() {
    // clean up
    delete[] current_packet;
  }

  pkt_read_fn_t callback;   // function to call after packet is decoded
  void *callback_ctx;       // context to be passed to the callback function
  uint8_t *current_packet;  // store current packet being decoded
  int packet_idx;           // store index of next place to insert in packet
  bool stx;  // signifies if we are currently in the middle of decoding a packet
  bool dle;  // signifies if previous value in packet was the escape value 0x10

} pkt_decoder_t;

/**
 * Constructs a new packet decoder.
 *
 * @param callback - function to be called after a packet has been completely
 * decoded
 * @param callback_ctx - context to be passed to the callback function
 * @return - the packet decoder
 */
pkt_decoder_t *pkt_decoder_create(pkt_read_fn_t callback, void *callback_ctx) {
  pkt_decoder *decoder = new pkt_decoder(callback, callback_ctx);
  return decoder;
}

/**
 * Calls the deconstructor for the decoder struct.
 * @param decoder - packet decoder to be deconstructed
 */
void pkt_decoder_destroy(pkt_decoder_t *decoder) { delete decoder; }

/**
 * Decodes a packet of bytes.
 *
 * @param decoder - stores the decoded packet
 * @param len - length of the bytes to be decoded
 * @param data - bytes to be decoded
 */
void pkt_decoder_write_bytes(pkt_decoder_t *decoder, size_t len,
                             const uint8_t *data) {
  // print the original bytes
  printf("Packet:               (%zd bytes) <", len);
  for (size_t i = 0; i < len; i++) {
    printf(" %02x", data[i]);
  }
  printf(" >\n");

  // decode the bytes
  for (size_t i = 0; i < len; i++) {
    if (decoder->dle) {
      // previous byte was an escape byte
      // use & ~0x20 to reverse the bitwise or
      add_byte_to_packet(decoder, data[i] & ~0x20);
      decoder->dle = false;
    } else if (data[i] == 0x02) {
      // start of a new packet
      start_new_packet(decoder);
    } else if (data[i] == 0x03) {
      // end of a packet
      end_packet(decoder);
    } else if (data[i] == 0x10) {
      // escape sequence, should look for next byte at index i+1
      if ((i + 1) < len) {
        // use & ~0x20 to reverse the bitwise or
        add_byte_to_packet(decoder, data[i + 1] & ~0x20);
        i++;
      } else {
        /* We're at the end of the current set of bytes, but we should
        remember that we saw an escape byte, so that we can
        properly process the next stream of bytes. */
        decoder->dle = true;
      }
    } else {
      add_byte_to_packet(decoder, data[i]);
    }
  }
}

/**
 * Processes a start byte 0x02 for the decoded packet.
 *
 * @param decoder - stores the decoded packet.
 */
void start_new_packet(pkt_decoder_t *decoder) {
  // start new packet by setting next index to 0
  decoder->packet_idx = 0;
  decoder->stx = true;
  decoder->dle = false;
}
/**
 * Processes a termination byte 0x03 for the decoded packet.
 *
 * If the packet is invalid (did not start with 0x02), then
 * reset the decoder.
 *
 * If the packet is valid, call the callback function on the
 * decoded contents and reset the decoder.
 *
 * @param decoder - stores the decoded packet
 */
void end_packet(pkt_decoder_t *decoder) {
  if (!decoder->stx) {
    // invalid packet: packet doesn't start with 0x02
    reset_packet(decoder);
    return;
  }

  // call the callback function on the decoded packet
  decoder->callback(decoder->callback_ctx, decoder->packet_idx,
                    decoder->current_packet);

  // reset packet
  reset_packet(decoder);
}

/**
 * Adds a byte to the contents of the decoded packet, if the packet
 * is valid.
 *
 * @param decoder - stores the decoded packet
 * @param value - byte to add to the packet
 */
void add_byte_to_packet(pkt_decoder_t *decoder, uint8_t value) {
  if (!decoder->stx) {
    // invalid packet: packet doesn't start with 0x02
    return;
  }

  if (decoder->packet_idx == MAX_DECODED_PACKET_LEN - 2) {
    // invalid packet: packet is too long
    reset_packet(decoder);
    return;
  }

  // add byte to the packet
  decoder->current_packet[decoder->packet_idx] = value;
  decoder->packet_idx++;
}

/**
 * Clears any info stored in the decoder about the previous packet.
 *
 * @param decoder - stores the decoded packet
 */
void reset_packet(pkt_decoder_t *decoder) {
  decoder->packet_idx = 0;
  decoder->stx = false;
  decoder->dle = false;
}