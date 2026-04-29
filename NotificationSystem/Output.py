import pandas as pd

INPUT_FILE = "calibrated_rssi_data.csv"
OUTPUT_FILE = "motion_alerts.csv"

DELTA_THRESHOLD = 10.0
COOLDOWN_SECONDS = 8

# Leer CSV
df = pd.read_csv(INPUT_FILE)

# Convertir tipos
df["time"] = pd.to_datetime(df["time"], errors="coerce")
df["millis"] = pd.to_numeric(df["millis"], errors="coerce")
df["raw_rssi"] = pd.to_numeric(df["raw_rssi"], errors="coerce")
df["delta"] = pd.to_numeric(df["delta"], errors="coerce")

# Eliminar filas malas
df = df.dropna(subset=["time", "millis", "raw_rssi", "delta"])

alerts = []
last_alert_time = None

for _, row in df.iterrows():
    ts = row["time"]
    millis = int(row["millis"])
    raw_rssi = int(row["raw_rssi"])
    delta = float(row["delta"])

    if abs(delta) >= DELTA_THRESHOLD:
        if last_alert_time is None or (ts - last_alert_time).total_seconds() >= COOLDOWN_SECONDS:
            alerts.append({
                "time": ts.isoformat(),
                "millis": millis,
                "raw_rssi": raw_rssi,
                "delta": delta,
                "alert": "MOTION DETECTED"
            })
            last_alert_time = ts

alerts_df = pd.DataFrame(alerts)
alerts_df.to_csv(OUTPUT_FILE, index=False)

print(f"Total alerts detected: {len(alerts_df)}")
print()
print(alerts_df.head(20))
print()
print(f"Saved alerts to: {OUTPUT_FILE}")