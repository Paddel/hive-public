from switchbot import Switchbot
from bleak import BleakScanner
import asyncio, sys

async def main(mac):
 ble_device = await BleakScanner.find_device_by_address(mac, timeout=20)
 device = Switchbot(ble_device)
 await device.turn_on()

if __name__ == "__main__":
 if len(sys.argv) != 2:
  print("No MAC-Address specified")
  sys.exit(1)

 mac = sys.argv[1].upper()
 asyncio.run(main(mac))