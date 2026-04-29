import pandas as pd
import time

INPUT_FILE = "calibrated_rssi_data.csv"
DELTA_THRESHOLD = 10.0
COOLDOWN_SECONDS = 8

df = pd.read_csv(INPUT_FILE)
df.columns = df.columns.str.strip()

df["time"] = pd.to_datetime(df["time"], errors="coerce")
df["millis"] = pd.to_numeric(df["millis"], errors="coerce")
df["raw_rssi"] = pd.to_numeric(df["raw_rssi"], errors="coerce")
df["delta"] = pd.to_numeric(df["delta"], errors="coerce")

df = df.dropna(subset=["time", "millis", "raw_rssi", "delta"])

last_alert_time = None

for _, row in df.iterrows():
    ts = row["time"]
    raw_rssi = int(row["raw_rssi"])
    delta = float(row["delta"])

    if abs(delta) >= DELTA_THRESHOLD:
        if last_alert_time is None or (ts - last_alert_time).total_seconds() >= COOLDOWN_SECONDS:
            print(f"[{ts}] 🔴 MOTION DETECTED | RSSI={raw_rssi} | delta={delta:.2f}")
            last_alert_time = ts
        else:
            print(f"[{ts}] 🟠 motion ongoing | RSSI={raw_rssi} | delta={delta:.2f}")
    else:
        print(f"[{ts}] 🟠 stable | RSSI={raw_rssi} | delta={delta:.2f}")

    time.sleep(0.08)