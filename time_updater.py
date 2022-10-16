# This file can be used with a laptop to set the KaramelTraktatie clock to the right current datetime without the need to specify each bytes one by one.
# Make sure you have pyserial library installed, and set the correct com port to communicate with.

# python -m serial.tools.list_ports -v
from datetime import datetime
str_to_send = datetime.now().strftime("SET_%y%m%d0%w_%H%M%S")
str_to_send = str_to_send.encode('utf-8')
print(f"Sending ==> {str_to_send}")

import serial
import time

with serial.Serial('COM6', 9600, timeout=1) as ser:
    ser.write(str_to_send)
    time.sleep(3)
    s = ser.read(400)
    print(s)
	
# Example output when setting time is succesful:
# b'Received a SET TIME command.\r\nParameters: \r\n22\r\n10\r\n16\r\n0\r\n20\r\n4\r\n29\r\n\r\n'
