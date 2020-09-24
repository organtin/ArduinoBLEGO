"""
This file has been stolen from James Lewis software to read the MP730026 DMM
whose documentation is on element14 site
"""
import asyncio
from bleak import discover
import time

async def run():
    devices = await discover()
    for d in devices:
        print(d)

while(1):
	loop = asyncio.get_event_loop()
	loop.run_until_complete(run())
	time.sleep(1)
