#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE  0x30U // 48 bytes
#define RESPONSE_SOF 0x34U // '4'
#define RESPONSE_EOF 0x0DU // '\r'

#define ASCII_TO_HEX(_BYTE_) (uint8_t)((_BYTE_) - '0')

// https://en.wikipedia.org/wiki/OBD-II_PIDs
typedef enum __OBD_SERVICE_e {
  OBD_SERVICE_01 = 0x01, // Show current data
  OBD_SERVICE_02 = 0x02, // Show freeze frame data
  OBD_SERVICE_03 = 0x03, // Show stored Diagnostic Trouble Codes
  OBD_SERVICE_04 = 0x04, // Clear Diagnostic Trouble Codes and stored values
  OBD_SERVICE_05 = 0x05, // Test results, oxygen sensor monitoring (non CAN only)
  OBD_SERVICE_06 = 0x06, // Test results, other component/system monitoring (Test results, oxygen sensor monitoring for CAN only)
  OBD_SERVICE_07 = 0x07, // Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
  OBD_SERVICE_08 = 0x08, // Control operation of on-board component/system
  OBD_SERVICE_09 = 0x09, // Request vehicle information
  OBD_SERVICE_10 = 0x0A, // Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
} obd_service_t;

bool is_valid_obd_service(uint8_t byte) {
  return ((ASCII_TO_HEX(byte) >= OBD_SERVICE_01) && (ASCII_TO_HEX(byte) <= OBD_SERVICE_10));
}

// Function to decode the response buffer
bool decode_response(const uint8_t *rsp, size_t rsp_size, char *decoded, size_t decoded_size) {
  int si = -1;
  int ei = -1;

  // Find the first sequence of RESPONSE_SOF followed by a valid OBD service
  for (int i = 0; i < rsp_size - 1; i++) {
    if (rsp[i] == RESPONSE_SOF && is_valid_obd_service(rsp[i + 1])) {
      // Found a RESPONSE_SOF followed by a valid OBD service, mark the starting index
      si = i;

      // Find the first sequence of RESPONSE_EOF
      for (int j = i + 1; j < rsp_size; j++) {
        if (rsp[j] == RESPONSE_EOF) {
          // Found a RESPONSE_EOF, mark the ending index
          ei = j;
          break;
        }
      }

      // Stop after finding the first valid sequence
      break;
    }
  }

  // Check if a valid sequence was found
  if ((si != -1) && (ei != -1)) {
    // Ensure the decoded buffer is large enough to hold the sequence
    uint16_t sequence_length = ei - si + 1;
    if (sequence_length > decoded_size) {
      printf("Decoded buffer is too small to hold the sequence.\n");
      return false;
    }

    // Copy the sequence into the decoded buffer
    memcpy(decoded, &rsp[si], sequence_length);

    // Ensure the string is null-terminated
    decoded[sequence_length] = '\0';
    return true;
  }

  // No valid sequence found
  return false;
}

int main(void) {
  uint8_t cmd[BUFFER_SIZE] = { 0x30, 0x31, 0x30, 0x63, 0x0D };
  uint8_t rsp[BUFFER_SIZE] = { 0x53, 0x45, 0x41, 0x52, 0x43, 0x48, 0x49, 0x4E, 0x47, 0x2E, 0x2E, 0x2E, 0x0D, 0x34, 0x31, 0x20, 0x30, 0x44, 0x20, 0x30, 0x30, 0x20, 0x0D, 0x0D, 0x3E };

  // Print the command buffer
  // printf("Command Buffer:\n");
  // for (int i = 0; i < BUFFER_SIZE; i++) {
  //   printf("0x%02X ", cmd[i]);
  // }
  // printf("\n");

  // Print the response buffer
  // printf("Response Buffer:\n");
  // for (int i = 0; i < BUFFER_SIZE; i++) {
  //   printf("0x%02X ", rsp[i]);
  // }
  // printf("\n");

  // Decode the response buffer
  char decoded[BUFFER_SIZE] = {'\0'};
  if (decode_response(rsp, BUFFER_SIZE, decoded, sizeof(decoded))) {
    printf("Decoded sequence: %s\n", decoded);
  } else {
    printf("No valid sequence found.\n");
  }

  return 0;
}