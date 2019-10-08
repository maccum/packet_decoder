#include "pkt_decoder.h"

#include <stdio.h>
#include <vector>

using namespace std;

/**
 * Print the decoded packet.
 * @param ctx - context
 * @param len - length of decoded packet
 * @param data - decoded bytes
 */
static void pkt_printer(void *ctx, size_t len, const uint8_t *data) {
  (void)ctx;
  printf("Decoded Packet:       (%zd bytes) <", len);
  for (size_t i = 0; i < len; i++) {
    printf(" %02x", data[i]);
  }
  printf(" >\n");
}

/**
 * Compare the decoded packets with the expected values.
 * @param packets - decoded bytes
 * @param expectedPackets - expected bytes
 */
void compare_packets(vector<vector<uint8_t>> packets,
                     vector<vector<uint8_t>> expectedPackets) {
  if (packets != expectedPackets) {
    printf("FAILED TEST.\n\n");
  } else {
    printf("PASSED TEST.\n\n");
  }
}

/**
 * Add a new packet to the vector of packets stored at ctx.
 * @param ctx - pointer to vector of packets
 * @param len - length of new packet
 * @param data - new packet
 */
void output_packet(void *ctx, size_t len, const uint8_t *data) {
  vector<vector<uint8_t>> *packets = ((vector<vector<uint8_t>> *)ctx);
  vector<uint8_t> packet;
  for (int i = 0; i < len; i++) {
    packet.push_back(data[i]);
  }
  packets->push_back(packet);
  pkt_printer(nullptr, len, data);
}

/**
 * Test if the packet decoder produces the expected output for the given input.
 * @param len - length of incoming bytes
 * @param input - bytes of incoming packet
 * @param expectedOutput - expected output of the decoded packet(s)
 */
void test(size_t len, const uint8_t *input,
          vector<vector<uint8_t>> expectedOutput) {
  vector<vector<uint8_t>> output;

  pkt_decoder_t *decoder = pkt_decoder_create(output_packet, &output);
  pkt_decoder_write_bytes(decoder, len, input);
  pkt_decoder_destroy(decoder);
  compare_packets(output, expectedOutput);
}

/**
 * Test if the packet decoder produces the expected output for two given
 * inputs of bytes.
 * @param len1 - length of first incoming set bytes
 * @param input1 - first incoming set of bytes
 * @param len2 - length of second incoming set of bytes
 * @param input2 - second set of bytes
 * @param expectedOutput - expected output of the decoded packet(s)
 */
void test_two_packets(size_t len1, const uint8_t *input1, size_t len2,
                      const uint8_t *input2,
                      vector<vector<uint8_t>> expectedOutput) {
  vector<vector<uint8_t>> output;

  pkt_decoder_t *decoder = pkt_decoder_create(output_packet, &output);
  pkt_decoder_write_bytes(decoder, len1, input1);
  pkt_decoder_write_bytes(decoder, len2, input2);
  pkt_decoder_destroy(decoder);

  compare_packets(output, expectedOutput);
}

int main() {
  printf("\nRunning tests: \n\n");

  // test basic packet
  const uint8_t pkt1[] = {0x02, 0xFF, 0x10, 0x22, 0x45, 0x03};
  vector<vector<uint8_t>> expectedPackets1 = {{0xFF, 0x02, 0x45}};
  test(sizeof(pkt1), pkt1, expectedPackets1);

  // test basic packet
  const uint8_t pkt2[] = {0x02, 0xFF, 0x03};
  vector<vector<uint8_t>> expectedPackets2 = {{0xFF}};
  test(sizeof(pkt2), pkt2, expectedPackets2);

  // test escape sequence
  const uint8_t pkt3[] = {0x02, 0x10, 0x22, 0x03};
  vector<vector<uint8_t>> expectedPackets3 = {{0x02}};
  test(sizeof(pkt3), pkt3, expectedPackets3);

  // test malformed packet (missing end)
  const uint8_t pkt4[] = {0x02, 0xFF, 0x02, 0xFE, 0x03};
  vector<vector<uint8_t>> expectedPackets4 = {{0xFE}};
  test(sizeof(pkt4), pkt4, expectedPackets4);

  // test two packets in a single stream
  const uint8_t pkt5[] = {0x02, 0xFF, 0x10, 0x30, 0x03, 0x02, 0xFE, 0x03};
  vector<vector<uint8_t>> expectedPackets5 = {{0xFF, 0x10}, {0xFE}};
  test(sizeof(pkt5), pkt5, expectedPackets5);

  // test two separate streams containing one packet
  const uint8_t pkt6a[] = {0x02, 0xFF, 0x10};
  const uint8_t pkt6b[] = {0x22, 0x03};
  vector<vector<uint8_t>> expectedPackets6 = {{0xFF, 0x02}};
  test_two_packets(sizeof(pkt6a), pkt6a, sizeof(pkt6b), pkt6b,
                   expectedPackets6);

  // test a packet that is too long
  uint8_t pkt7[MAX_DECODED_PACKET_LEN + 4] = {0};
  pkt7[0] = 0x02;
  pkt7[MAX_DECODED_PACKET_LEN] = 0x03;
  // followed by a small packet
  pkt7[MAX_DECODED_PACKET_LEN + 1] = 0x02;
  pkt7[MAX_DECODED_PACKET_LEN + 2] = 0x20;
  pkt7[MAX_DECODED_PACKET_LEN + 3] = 0x03;
  vector<vector<uint8_t>> expectedPackets7 = {{0x20}};
  test(sizeof(pkt7), pkt7, expectedPackets7);

  return 0;
}
