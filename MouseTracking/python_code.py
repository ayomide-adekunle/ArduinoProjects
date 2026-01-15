import serial
import time
from pynput.mouse import Controller, Button

# ---------------- Settings ----------------
PORT = "/dev/cu.usbmodem14101"          # Windows example: COM5
# PORT = "/dev/ttyACM0"  # Linux example
# PORT = "/dev/cu.usbmodemXXXX"  # macOS example
BAUD = 9600

#install the following plugins
#python3 -m pip install pyserial pynput
# or python -m pip install pyserial pynput
# if you get pip error, then you need to install "pip"


# Extra scaling on PC side (optional)
SCALE = 1.0

# If True: click once on press edge
EDGE_CLICK = True

# ---------------- Runtime ----------------
mouse = Controller()

def main():
  prevL = 0
  prevR = 0

  with serial.Serial(PORT, BAUD, timeout=1) as ser:
    # Let Arduino reset after serial open
    time.sleep(2.0)
    print("Listening... (Ctrl+C to quit)")

    while True:
      line = ser.readline().decode(errors="ignore").strip()
      if not line:
        continue

      # Expect: dx,dy,L,R
      parts = line.split(",")
      if len(parts) != 4:
        continue

      try:
        dx = int(parts[0])
        dy = int(parts[1])
        L  = int(parts[2])
        R  = int(parts[3])
      except ValueError:
        continue

      # Move mouse
      if dx != 0 or dy != 0:
        mouse.move(int(dx * SCALE), int(dy * SCALE))

      # Click handling
      if EDGE_CLICK:
        # click on rising edge
        if L == 1 and prevL == 0:
          mouse.click(Button.left, 1)
        if R == 1 and prevR == 0:
          mouse.click(Button.right, 1)
      else:
        # press/hold mode (drag support)
        if L != prevL:
          (mouse.press(Button.left) if L else mouse.release(Button.left))
        if R != prevR:
          (mouse.press(Button.right) if R else mouse.release(Button.right))

      prevL, prevR = L, R

if __name__ == "__main__":
  try:
    main()
  except KeyboardInterrupt:
    print("\nStopped.")
