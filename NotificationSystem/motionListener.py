# motion_listener.py
from flask import Flask, request
from datetime import datetime
import subprocess

app = Flask(__name__)

@app.route('/motion', methods=['POST'])
def motion():
    data = request.get_json()
    print(f"[{datetime.now()}] Motion! delta={data['delta']} dB  rssi={data['rssi']}")
    
    # Trigger the texting script (non-blocking)
    subprocess.Popen(['python', 'send_text.py', str(data['delta'])])
    return {'status': 'received'}, 200

if __name__ == '__main__':
    # 0.0.0.0 means listen on all network interfaces, not just localhost
    app.run(host='0.0.0.0', port=5000)