# TeleGuardian Data Schema

Every probe reading sent to Firebase follows this exact structure:

```json
{
  "rssi": -65,
  "latency_ms": 45,
  "packet_loss_pct": 2.1,
  "x": 3,
  "y": 5,
  "timestamp": "2026-08-15T14:30:00",
  "fault_label": null
}
```

## Field definitions

| Field | Type | Meaning |
|---|---|---|
| rssi | number | Signal strength in dBm (negative, closer to 0 = stronger) |
| latency_ms | number | Ping delay in milliseconds |
| packet_loss_pct | number | Percentage of packets lost (0-100) |
| x | number | Grid X-coordinate of where the reading was taken |
| y | number | Grid Y-coordinate of where the reading was taken |
| timestamp | string | ISO 8601 format, e.g. 2026-08-15T14:30:00 |
| fault_label | string or null | null for live readings; filled in later with "interference", "congestion", "distance", or "none" during controlled data collection |
