import serial
import csv
from datetime import datetime       # for timestamps

PORT = "/dev/cu.usbserial-0001"     # ESP32 port
BAUD = 115200                       # matching the Adruino code
OUTPUT_FILE = "rssi_data.csv"       # saving data for analysis

ser = serial.Serial(PORT, BAUD, timeout=1)      # open serial connection

with open(OUTPUT_FILE, "a", newline="") as f:
    writer = csv.writer(f)

    # Optional header if file is empty
    if f.tell() == 0:
        writer.writerow(["timestamp_iso", "millis", "rssi", "delta"])

    print(f"Logging data to {OUTPUT_FILE}... Press Ctrl+C to stop.")

    try:
        while True:
            # read line from ESP32
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                print(line)     # print to terminal
                parts = line.split(",")

                if len(parts) == 3:
                    millis, rssi, delta = parts

                    # save with real timestamp + data
                    writer.writerow([datetime.now().isoformat(), millis, rssi, delta])
                    f.flush()

    except KeyboardInterrupt:
        print("\nStopped logging.")

    finally:
        ser.close()     # close serial connection