# AURIX TC334 CAN Transmission Demo

A CAN bus transmission project built on the Infineon AURIX TC334 Lite Kit, using the on-chip CAN peripheral (IfxCan) to send data over the CAN bus and verify it via an internal loopback test and an external USB-CAN-FD-B analyzer.

## Overview

This project transmits a single byte (`'a'`) over CAN from the AURIX TC334 and confirms successful transmission in two stages:

1. **Loopback test** — the TC334's CAN node is configured with `busLoopbackEnabled = TRUE`, so it transmits and immediately receives its own frame internally, with no external wiring required. This isolates whether the CAN peripheral itself is functioning correctly, independent of external bus wiring, termination, or a connected receiver.
2. **External bus verification** — once loopback confirms the peripheral works, the board is wired to a USB-CAN-FD-B analyzer running CANFDToolPro to confirm the frame is actually visible on the physical bus.

An onboard LED provides visual feedback:
- **3 fast blinks, repeating** → loopback succeeded, frame was transmitted and received correctly, CAN peripheral works
- **1 slow blink, repeating** → loopback failed, CAN peripheral/init issue to debug further

## Hardware Used

| Component | Role |
|---|---|
| Infineon AURIX TC334 Lite Kit | Main MCU, CAN0 peripheral |
| TLE9251VSI CAN transceiver | Physical layer CAN transceiver |
| USB-CAN-FD-B (with CANFDToolPro) | External CAN bus analyzer/monitor |
| Onboard LED | Visual pass/fail indicator |

## Pin Mapping

| Signal | Pin |
|---|---|
| LED | P00.6 |
| CAN TX | P20.8 |
| CAN RX | P20.7 |
| Transceiver STB (standby) | P20.9 |

## CAN Configuration

- Baud rate: 500 kbps
- Sample point: 80%
- Frame type: Classic CAN, standard ID
- Message ID: `0x100`
- Data length: 1 byte

## How It Works

`Can_init()` configures CAN0, Node 0, with dedicated TX/RX buffers and loopback mode enabled. `Can_sendAndReceive()` transmits a byte and then polls the RX buffer for the same message ID, returning the received byte. The main loop compares the received byte against the sent byte and drives the LED pattern accordingly.

## Debugging Notes

An earlier version relied on a raw send-timeout (`Can_sendByte`) that exited the wait loop after ~100,000 cycles regardless of whether an ACK was received. This meant the LED blinking only confirmed that code execution passed the send call — **not** that a frame was successfully placed on the bus, since CAN requires a receiver to ACK the frame. The loopback test was added specifically to get an unambiguous, wiring-independent confirmation that the TC334's CAN peripheral was transmitting and receiving correctly, before spending further time on external bus wiring, grounding, and termination.

## Current Status

- [x] Loopback test implemented and passing
- [ ] External bus wiring (GND, 120Ω termination) confirmed with USB-CAN-FD-B / CANFDToolPro
- [ ] Screenshot of received frame in CANFDToolPro

## Build Instructions

1. Open the project in AURIX Development Studio.
2. Build using the TASKING compiler toolchain (default for AURIX Development Studio).
3. Flash to the TC334 Lite Kit.
4. Observe the onboard LED for the loopback result.
5. (Optional) Connect to a USB-CAN-FD-B analyzer, start Channel 1 in CANFDToolPro with 120Ω termination enabled, and verify frames with ID `0x100` appear on the bus.

## Files

```
aurix-tc334-can-transmit/
├── README.md
├── Cpu0_Main.c
├── Can.c
└── Can.h
```
