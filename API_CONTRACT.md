# TeleGuardian API Contract

GET /diagnosis?probe_id=1
→ returns: {"fault": "congestion", "confidence": 0.82}

GET /heatmap?room_id=1
→ returns: a PNG image URL of the coverage map

POST /readings
→ accepts: a single reading matching SCHEMA.md, used by Person A's firmware
