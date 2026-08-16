import firebase_admin
from firebase_admin import credentials, db

cred = credentials.Certificate("teleguardian-fyp-firebase-adminsdk-fbsvc-b8d97abac8.json")
firebase_admin.initialize_app(cred, {
    "databaseURL": "https://teleguardian-fyp-default-rtdb.firebaseio.com/:"
})

def validate_reading(data):
    required = {"rssi", "latency_ms", "packet_loss_pct", "x", "y", "timestamp", "fault_label"}
    if not required.issubset(data.keys()):
        raise ValueError(f"Missing fields: {required - data.keys()}")
    if not isinstance(data["rssi"], (int, float)):
        raise ValueError("rssi must be a number")
    return True

def push_reading(data):
    validate_reading(data)   # checks the data BEFORE sending it
    ref = db.reference("readings")
    ref.push(data)

def get_all_readings():
    ref = db.reference("readings")
    return ref.get()