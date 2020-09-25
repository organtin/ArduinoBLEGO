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
address = ("9DBBC745-EFEE-4C52-91EB-98720EA35889") # for macOS

# do not change this - UUID for the custom characteristics
CHARACTERISTIC_UUID = "555a0002-0020-467a-9538-01f0652c74e8"

def notification_handler(sender, data):
    """Simple notification handler which prints the data received."""
    array = bytearray(data)

    # here we decode the custom characteristics
    t  = struct.unpack('f', array[0:4])
    ax = struct.unpack('f', array[4:8])
    ay = struct.unpack('f', array[8:12])
    az = struct.unpack('f', array[12:16])
    gx = struct.unpack('f', array[16:20])
    gy = struct.unpack('f', array[20:24])
    gz = struct.unpack('f', array[24:28])
    print(f'{t[0]}, {ax[0]}, {ay[0]}, {az[0]}, {gx[0]}, {gy[0]}, {gz[0]}')

async def run(address):
    async with BleakClient(address) as client:
        x = await client.is_connected()
        
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        await asyncio.sleep(460.0)
        await client.stop_notify(CHARACTERISTIC_UUID)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(run(address))
