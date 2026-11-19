
# AURA — Adaptive Urban Road Lighting Architecture

> Intelligent, low-cost street lighting infrastructure for Indian cities — built on LoRa mesh networking, real-time per-pole monitoring, and adaptive energy control.

---

## 🔦 The Problem

Hyderabad's 5.5 lakh LED streetlights consume **₹102 crore/year** in electricity, yet the city receives ~600 fault complaints **every single day**. The root cause: existing Command Control and Monitoring Systems (CCMS) issue ON/OFF commands to groups of 100–200 poles with no feedback. When a pole fails, no one knows until a citizen complains. The result — over **20,450 non-functional streetlights** in a city where 25,860 out of 26,000 CCMS units are considered "operational."

**Core gaps in the current system:**
- No per-pole status feedback — commands sent, never verified
- No pole-level energy metering — billing fraud and power theft go undetected
- Faults discovered reactively through complaints, not automated detection
- Lights burn at 100% brightness at 3 AM on empty roads
- Blanket replacement of all 5.5 lakh LEDs despite many having 1–2 years of life remaining

---

## ✅ What AURA Does

AURA retrofits onto existing streetlight infrastructure and turns every pole into an intelligent, communicating node — without SIM cards, without rewiring, and at a fraction of competitor costs.

| Capability | Detail |
|---|---|
| **Real-time monitoring** | Per-pole status checked every 10 minutes; faults reported with exact ID and location |
| **Automated fault detection** | Faults identified within **15 minutes** and maintenance team notified automatically |
| **Adaptive brightness control** | Alternate poles switched during low-traffic hours (11 PM – 5 AM), reducing active lights by 50% |
| **Energy auditing** | Per-pole consumption data to detect theft, leakage, and billing discrepancies |
| **Predictive maintenance** | Continuous health monitoring enables preventive servicing before failures occur |
| **Selective replacement** | Replace only defective poles — not entire networks — based on live health data |

**Estimated impact:** ~30% energy savings over an 11.5-hour operating cycle → **₹30 crore annual savings** for Hyderabad alone.

---

## 🏗️ System Architecture

```
[Pedestrian / Vehicle Activity]
         │
         ▼
 ┌──────────────────┐
 │  Detection Node  │  ← Activity sensor, manages ~15 nearby poles
 └────────┬─────────┘
          │  433 MHz LoRa Mesh
          ▼
 ┌──────────────────┐
 │   Relay Nodes    │  ← ~14 per detection node, execute phased ON/OFF
 └────────┬─────────┘
          │
          ▼
 ┌──────────────────┐
 │  Gateway Node    │  ← Aggregates ~100 poles, forwards to cloud
 └────────┬─────────┘
          │  Internet / GSM
          ▼
 ┌──────────────────────────────┐
 │  Central Monitoring Dashboard│  ← Live status, energy analytics, fault alerts
 └──────────────────────────────┘
```

**Communication:** 433 MHz LoRa mesh — no SIM card or internet required at each pole. A single gateway channel reliably handles up to 100 poles with minimal collision.

---

## 🏫 Campus Prototype — GCET

AURA will be  first deployed on the **GCET campus** to optimize 50 high-power floodlights (200 W each) that previously ran at full intensity throughout the night.

- Precision scheduling based on actual sunrise/sunset times
- Adaptive phased switching: alternate floodlights OFF during 1 AM – 5 AM
- Estimated annual saving: **5,000–5,500 kWh (~22%)** → ₹40,000–44,000/year for 30 floodlights
- Real-time fault detection via current monitoring on every controller node



