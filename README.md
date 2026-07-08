# AURIX TC334 Keypad-to-CAN Transmitter

Reads key presses from a matrix keypad on the Infineon AURIX TC334 Lite Kit and transmits the corresponding digit over CAN, with onboard LEDs used as heartbeat/status indicators.

## Overview

On each keypress, the number is sent as a CAN frame (ID `0x123`, standard frame, 8-byte data field, first byte = ASCII digit). This builds on an earlier loopback-verified CAN transmit base — the CAN peripheral itself was already confirmed working via internal loopback before this keypad integration was added.

## Hardware Used

| Component | Role |
|---|---|
| Infineon AURIX TC334 Lite Kit | Main MCU, CAN0 peripheral |
| TLE9251VSI CAN transceiver | Physical layer CAN transceiver |
| 4x4 matrix keypad | Digit input |
| USB-CAN-FD-B (with CANFDToolPro) | External CAN bus analyzer/monitor |
| 2 onboard LEDs | Heartbeat + TX-attempt indicators |

## Pin Mapping

| Signal | Pin |
|---|---|
| LED 1 (heartbeat) | P00.5 |
| LED 2 (TX attempted) | P00.6 |
| CAN TX | P20.8 |
| CAN RX | P20.7 |
| Transceiver STB (standby) | P20.9 |
| Keypad Row 0 | P02.0 |
| Keypad Row 1 | P02.1 |
| Keypad Col 0 | P02.2 |
| Keypad Col 1 | P02.3 |

## CAN Configuration

- Baud rate: 500 kbps
- Sample point: 80%
- Frame type: transmit only, Classic CAN, standard ID
- Message ID: `0x123`
- Data length: 8 bytes (first byte carries the ASCII digit, rest zeroed)
- 1 dedicated TX buffer

## How It Works

`CAN_init()` wakes the TLE9251VSI transceiver by driving `CAN_STB` (P20.9) low, then configures CAN0 Node 0 as a transmit-only node with one dedicated TX buffer at message ID `0x123`.

`Keypad_scan()` drives each row low in turn and reads the column pins (active-low, pulled up) to detect which key is pressed, returning `KEY_NONE` if nothing is pressed.

In the main loop, whenever `Keypad_scan()` returns a key other than `KEY_NONE`:
1. The key value is converted to its ASCII digit (`'0' + keyPressed`) and placed in the TX data buffer.
2. LED 1 toggles as a heartbeat.
3. `IfxCan_Can_sendMessage()` is called, retrying up to 1000 times if the buffer is busy.
4. LED 2 toggles to indicate a transmit attempt was made.
5. A ~500 ms debounce delay follows before the next scan.

## Current Status / Known Limitations

- The keypad hardware is a 4x4 matrix, but the current `Keypad_scan()` implementation only drives 2 rows and reads 2 columns, so only 4 keys (`1`, `2`, `4`, `5`) are currently mapped and scanned. Extending to the full 4x4 grid (4 rows × 4 columns) is a straightforward next step.
- CAN transmission uses a retry-count timeout rather than blocking on ACK, consistent with the earlier loopback-verified base.
- External bus verification via USB-CAN-FD-B / CANFDToolPro (GND connection, 120Ω termination, Channel 1 started) should be confirmed and screenshotted before considering this feature-complete.

## Build Instructions

1. Open the project in AURIX Development Studio.
2. Build using the TASKING compiler toolchain.
3. Flash to the TC334 Lite Kit.
4. Press keys on the keypad and observe LED 1 (heartbeat) and LED 2 (TX attempted) toggling.
5. Connect a USB-CAN-FD-B analyzer, start Channel 1 in CANFDToolPro with 120Ω termination enabled, and verify frames with ID `0x123` appear on the bus when keys are pressed.

## Files

```
aurix-tc334-keypad-can/
├── README.md
├── Cpu0_Main.c
├── Can.c / Can.h
├── Keypad.c / Keypad.h
```
