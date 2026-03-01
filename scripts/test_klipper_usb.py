#!/usr/bin/env python3
# Test Klipper USB Serial/JTAG protocol on ESP32-C3
import serial, struct, time, sys

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyACM0"

def crc16_ccitt(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
        crc &= 0xFFFF
    return crc

def make_message(seq, *payload_bytes):
    payload = bytes(payload_bytes)
    msglen = len(payload) + 5  # header(2) + payload + crc(2) + sync(1)
    buf = bytes([msglen, seq]) + payload
    crc = crc16_ccitt(buf)
    return buf + bytes([crc >> 8, crc & 0xFF, 0x7E])

# identify offset=0 count=40: cmd_id=1, arg0=0, arg1=40(0x28)
identify_msg = make_message(0x10, 0x01, 0x00, 0x28)
print(f"identify msg: {identify_msg.hex()}")

with serial.Serial(PORT, baudrate=115200, timeout=3.0) as s:
    s.reset_input_buffer()
    s.write(b'\x7e' * 5 + identify_msg)
    s.flush()
    print(f"sent, waiting for response...")
    deadline = time.time() + 3.0
    buf = b''
    while time.time() < deadline:
        chunk = s.read(256)
        if chunk:
            buf += chunk
            print(f"received {len(chunk)} bytes: {chunk.hex()}")
            if b'\x7e' in chunk:
                break
    if not buf:
        print("no response received")
    else:
        print(f"total received: {buf.hex()}")
