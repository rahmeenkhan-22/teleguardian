from firebase_config import db

def save_schema_reference():
    schema_doc = {
        "rssi": "number, dBm, e.g. -65",
        "latency_ms": "number, milliseconds",
        "packet_loss_pct": "number, 0-100",
        "x": "number, grid coordinate",
        "y": "number, grid coordinate",
        "timestamp": "string, ISO 8601",
        "fault_label": "string or null"
    }
    ref = db.reference("_schema")
    ref.set(schema_doc)
    print("Schema saved to Firebase!")

if __name__ == "__main__":
    save_schema_reference()