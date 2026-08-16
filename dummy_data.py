import random
import time
from datetime import datetime
from firebase_config import push_reading

def generate_fake_reading():
    return {
        "rssi": random.randint(-90, -40),
        "latency_ms": random.randint(10, 300),
        "packet_loss_pct": round(random.uniform(0, 25), 1),
        "x": random.randint(0, 10),
        "y": random.randint(0, 10),
        "timestamp": datetime.now().isoformat(),
        "fault_label": None
    }

if __name__ == "__main__":
    while True:
        reading = generate_fake_reading()
        push_reading(reading)
        print("Pushed:", reading)
        time.sleep(5)