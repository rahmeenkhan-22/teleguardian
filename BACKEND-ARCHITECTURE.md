# Backend Architecture & System Implementation

## 1. Overview

The backend is the middle layer of TeleGuardian's three-layer pipeline: it receives raw telemetry from the physical probe, stores and validates it, computes the two core outputs (fault diagnosis and coverage mapping), and exposes all of this to the mobile application through a REST API. It was implemented in Python using Flask, backed by Firebase Realtime Database for storage.

*(Insert Figure: the pipeline flowchart — Physical Sensing & Telemetry Capture → Cloud Storage & Data Validation → Fault Classification → Spatial Interpolation & Coverage Mapping → Mobile Diagnostics Dashboard.)*

## 2. Data Storage and Schema

Telemetry is stored in Firebase Realtime Database under a `readings` node, following a fixed schema agreed early in development:

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

Firebase Realtime Database is schema-less by design, so this structure is enforced at two independent layers rather than assumed:

- **Application-level validation** (`validate_reading()` in `firebase_config.py`) checks every reading for required fields and correct types before it is written, called automatically by `push_reading()`.
- **Database-level security rules**, published directly to Firebase, enforce the same constraints (required fields, numeric types, valid ranges for `packet_loss_pct`) independently of the backend code. This matters because the physical probe is designed to write directly to Firebase in the full deployment (Section 5.1), bypassing the Python backend entirely — so validation at the database level is the only guarantee that malformed firmware output cannot silently corrupt stored data.

*(Insert Figure: Firebase console screenshot showing the `_schema` reference node and a populated `readings` node.)*

## 3. Fault-Telemetry Pipeline

A lightweight dummy-data generator (`dummy_data.py`) was built early in development to simulate probe readings, decoupling backend development from the hardware team's progress. It produces schema-compliant fake readings at a fixed interval, allowing every downstream component — validation, storage, the API, and later the mobile app — to be developed and tested before real hardware telemetry existed.

*(Insert Figure: terminal output showing live pushed readings alongside the corresponding live Firebase console view.)*

## 4. Fault Classification Endpoint

A `/diagnosis` endpoint currently returns a stubbed response in the same format the final classifier will use:

```json
{"fault": "congestion", "confidence": 0.82}
```

This stub allows the mobile application and the rest of the API surface to be developed against a stable contract ahead of the trained model becoming available (Objective 3, dependent on the AI/Data workstream). Once a Decision Tree, Random Forest, and Gradient Boosting classifier have been trained and compared (Section 5.2 of the proposal), the best-performing model will be loaded into this same endpoint without changing its external interface.

## 5. Spatial Interpolation and Coverage Mapping

An Inverse Distance Weighting (IDW) module (`idw.py`) estimates signal strength at unmeasured points from a set of known (x, y, RSSI) readings, weighting nearer points more heavily. A second module (`heatmap.py`) evaluates this function across a full grid and renders the result as a color-coded image using Matplotlib, exposed through a `/heatmap` endpoint that regenerates the map on demand from whatever readings currently exist in Firebase.

*(Insert Figure: rendered heatmap output from live Firebase data.)*

### 5.1 Validation Against Public Data

Before real hardware-collected readings were available, the IDW module was validated against the public UJIIndoorLoc dataset (Torres-Sospedra et al., 2014), which contains real indoor RSSI fingerprints collected across multiple university buildings. Two independent access points from a single building and floor were tested using an 80/20 known/held-out split: for each held-out point, the module's predicted RSSI was compared against the actual measured value.

| Access Point | Usable Points | Test Points | Mean Absolute Error |
|---|---|---|---|
| WAP155 | 522 | 105 | 5.87 dBm |
| WAP156 | 514 | 103 | 5.76 dBm |

Both runs produced consistent results, averaging approximately **5.8 dBm mean absolute error**, within the single-digit dBm range reported in prior indoor RF interpolation literature (El-friakh et al., 2018; Jan et al., 2015). This consistency across two independent access points gives reasonable confidence that the module generalizes rather than performing well on one favorable case.

## 6. REST API Summary

| Endpoint | Method | Purpose |
|---|---|---|
| `/` | GET | Health check |
| `/readings` | POST | Accepts and validates a new telemetry reading |
| `/readings` | GET | Returns all stored readings |
| `/heatmap` | GET | Returns a freshly rendered coverage map image |
| `/diagnosis` | GET | Returns the current fault diagnosis (stubbed pending classifier integration) |

Each endpoint was tested independently using Postman before any dependency on the mobile application, following the API contract shared with the App/Documentation lead at the start of development (`API_CONTRACT.md`).

## 7. Version Control and Reproducibility

All backend code, the validation dataset, and this chapter's supporting results are tracked in the team's private GitHub repository. Firebase credentials are excluded via `.gitignore` and were confirmed never to have been committed.

## 8. Summary

The backend implementation covers Objectives 2, 3 (partially, pending classifier handoff), and 4 in full: a validated, schema-enforced telemetry pipeline; a working, real-data-validated coverage-mapping module; and a complete REST API surface ready to receive the trained classifier once it is available from the AI/Data workstream.
