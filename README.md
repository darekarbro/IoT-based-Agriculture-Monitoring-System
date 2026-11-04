# IoT-Based Smart Agriculture Monitoring System

An Internet of Everything (IoE) prototype for monitoring agricultural conditions
with an ESP32 and a browser-based decision-support dashboard. The system reads
temperature, humidity, soil wetness, and a raw gas-sensor signal, then presents
live readings and configurable recommendations on the local network.

This README is based on the latest project report,
[`IOE_Sem7_Project_Report-v2.pdf`](./IOE_Sem7_Project_Report-v2.pdf), and the
implementation currently supplied in this repository.

## Features

- Live DHT11 temperature and relative-humidity readings.
- Averaged analog readings from a resistive soil probe and MQ7 module.
- ESP32 Wi-Fi HTTP server with a JSON endpoint at `/data`.
- Responsive standalone dashboard with:
  - connection status and live sensor cards;
  - short in-session trend traces;
  - dry-soil, waterlogging, heat, humidity, and gas advisories;
  - editable calibration values and warning limits;
  - optional next-12-hour weather information from OpenWeatherMap.
- Rain-aware irrigation guidance: likely rain can suppress an irrigation
  recommendation when the soil is currently dry.
- Browser settings persisted with `localStorage`.
- Reduced-motion support for users who prefer less animation.

## System architecture

```text
DHT11 ───────────────┐
Soil moisture probe ─┼─> ESP32 ── Wi-Fi/HTTP ──> Browser dashboard
MQ7 module ──────────┘             /data
                                                   └─> OpenWeatherMap (optional)
```

The ESP32 performs sensor acquisition and serves the latest cached readings.
The browser performs calibration, threshold evaluation, visualization, and
weather-aware recommendations. No cloud database or automatic actuator control
is included.

## Repository contents

| File | Purpose |
| --- | --- |
| [`smart_agri_dashboard_v2.html`](./smart_agri_dashboard_v2.html) | Standalone HTML, CSS, and JavaScript dashboard |
| [`esp32_web_server/esp32_web_server.ino`](./esp32_web_server/esp32_web_server.ino) | ESP32 firmware and HTTP server |
| [`IOE_Sem7_Project_Report-v2.pdf`](./IOE_Sem7_Project_Report-v2.pdf) | Latest project report |
| [`IOE_Sem7_Project_Report-v2.docx`](./IOE_Sem7_Project_Report-v2.docx) | Editable copy of the latest report |

## Hardware

- ESP32 development board
- DHT11 temperature and humidity sensor
- Resistive soil-moisture probe/module
- MQ7 gas-sensor module
- Jumper wires and a suitable power/USB connection

### Pin mapping

| Component | ESP32 pin | Firmware meaning |
| --- | --- | --- |
| DHT11 data | GPIO 4 | Digital temperature/humidity input |
| Soil probe analog output | GPIO 34 | ADC1 soil reading |
| MQ7 analog output | GPIO 35 | ADC1 gas reading |

GPIO 34 and GPIO 35 are input-only ADC1 pins. Ensure that every module shares
ESP32 ground and that analog outputs do not exceed the ESP32 input voltage.
The firmware comment specifically requires a voltage divider if the MQ7 module
is operated at 5 V.

## Software requirements

- Arduino IDE or another ESP32-compatible Arduino build environment.
- ESP32 board support package.
- Arduino libraries:
  - `DHT sensor library` by Adafruit
  - `Adafruit Unified Sensor`
- A modern browser with JavaScript enabled.
- Optional: an OpenWeatherMap API key for forecast-aware advice.

## Setup

### 1. Configure and upload the firmware

1. Open [`esp32_web_server.ino`](./esp32_web_server/esp32_web_server.ino) in
   Arduino IDE.
2. Set `WIFI_SSID` and `WIFI_PASS` for the local 2.4 GHz Wi-Fi network.
3. Confirm the sensor wiring and pin assignments.
4. Select the connected ESP32 board and port, then upload the sketch.
5. Open Serial Monitor at `115200` baud.
6. Wait for the printed local IP address.

Do not publish Wi-Fi credentials or API keys in a repository. The values
currently present in the firmware should be treated as exposed and rotated
before deployment.

### 2. Open the dashboard

Open [`smart_agri_dashboard_v2.html`](./smart_agri_dashboard_v2.html) directly
in a browser. Enter the ESP32 IP address in the top bar and select
**Connect**. The browser polls the ESP32 every three seconds by default.

If the browser blocks requests from a local `file:` page, serve the project
directory with any simple static HTTP server and open the resulting local URL.
The ESP32 endpoint enables CORS for this browser-based setup.

### 3. Enable weather (optional)

In **Settings, calibration and limits**:

1. Enter a city.
2. Enter an OpenWeatherMap API key.
3. Connect to the ESP32.

The dashboard uses the forecast endpoint to inspect four three-hour forecast
samples. Sensor monitoring continues to work when weather is not configured or
temporarily unavailable.

## HTTP API

`GET http://<ESP32-IP>/data`

Example response:

```json
{
  "temperature": 28.4,
  "humidity": 67.0,
  "soil": 2700,
  "gas": 420,
  "dhtOk": true
}
```

| Field | Description |
| --- | --- |
| `temperature` | DHT11 temperature in degrees Celsius |
| `humidity` | DHT11 relative humidity percentage |
| `soil` | Averaged raw soil-probe ADC reading; higher means drier |
| `gas` | Averaged raw MQ7 ADC reading; higher means more gas response |
| `dhtOk` | Whether the latest DHT11 reading was valid |

`GET /` returns a simple health message. The endpoint is intended for a
trusted local network and is not an authenticated public API.

## Calibration and decision rules

The dashboard converts raw soil input to an indicative percentage:

```text
soilPct = clamp(0, 100, (dryReading - rawSoil) / (dryReading - wetReading) * 100)
```

Default dashboard values are:

| Setting | Default |
| --- | ---: |
| Dry soil ADC reading | 3200 |
| Wet soil ADC reading | 1400 |
| Irrigate below | 30% |
| Waterlogging warning above | 85% |
| Heat warning / critical | 32 / 38 °C |
| Gas warning / critical | 1500 / 2500 raw ADC |
| Sensor refresh interval | 3 seconds |

These are starting points, not crop-specific or laboratory-validated limits.
Calibrate the dry and wet values using the installed probe and adjust the
thresholds for the crop and growing conditions.

## Limitations and future work

- Soil percentage is a relative calibrated index, not volumetric water content.
- MQ7 output is raw ADC data, not certified carbon-monoxide concentration.
- DHT11 provides general ambient monitoring rather than high-precision
  environmental measurement.
- No permanent sensor-history database or remote authenticated access exists.
- The system provides advice only; it does not operate pumps, fans, or other
  actuators.
- Weather information requires browser internet access and an API key.
- Future improvements include validated sensor calibration, secure credential
  handling, persistent history, authenticated remote access, and actuator
  integration with appropriate safety controls.

## Testing status

The latest report documents functional validation using the ESP32 Serial
Monitor, the local dashboard, and the weather panel on the prototype hardware.
The report does not claim a calibrated reference dataset, measured water
savings, or independently measured end-to-end latency.

## Project report

For the complete objectives, methodology, architecture, test cases, results,
and future-work discussion, see
[`IOE_Sem7_Project_Report-v2.pdf`](./IOE_Sem7_Project_Report-v2.pdf).
