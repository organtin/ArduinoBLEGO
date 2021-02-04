#!/usr/bin/python

# Modified by giovanni organtini (giovanni.organtini@uniroma1.it)
# Modified by james lewis (@baldengineer)
# MIT License
# 2020
# Script to connect to an Arduino BLE board
# Based on bleak example as modified by James Lewis

import asyncio
import platform
import struct

from bleak import BleakClient

# Change this to your Arduino's address
#address = ("A5:B3:C2:24:15:16") # for Windows and Linux
address = ("A8D7A4F1-3D94-4A7D-9D3C-3D17607C3515")

# do not change this - UUID for the custom characteristics
CHARACTERISTIC_UUID = "555a0002-0020-467a-9538-01f0652c74e8"

def notification_handler(sender, data):
    """Simple notification handler which prints the data received."""
    array = bytearray(data)

    # here we decode the custom characteristics
    t  = struct.unpack('f', array[0:4])
    p = struct.unpack('f', array[4:8])
    T = struct.unpack('f', array[8:12])
    print(f'{t[0]}, {p[0]}, {T[0]}')
    f = open('pT.csv', 'a')
    f.write(f'{t[0]}, {p[0]}, {T[0]}\n')
    f.flush()
    f.close()

async def run(address):
    async with BleakClient(address) as client:
        x = await client.is_connected()
        
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        await asyncio.sleep(460.0)
        await client.stop_notify(CHARACTERISTIC_UUID)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(address))
