# Wi-Fi Latency Tracker

![Raspberry Pi Pico W](https://www.raspberrypi.com/documentation/microcontrollers/images/picow-pinout.svg)

A real-time Wi-Fi latency monitoring system built with the Raspberry Pi Pico W. This project continuously measures network latency to your router, tracks RP2040 temperature variations, and sends measurement data to InfluxDB.

## Features

- **Real-time Latency Monitoring**: Measures round-trip time (RTT) to your router using custom ICMP implementation
- **Advanced Statistics**: Tracks min/max/average RTT, jitter, and packet loss
- **Temperature Monitoring**: Uses the Pico W's ADC to measure temperature following datasheet formula
- **Time Series Storage**: Stores metrics in InfluxDB for historical analysis
- **Robust Error Handling**: Implements exponential backoff for failed operations
- **Auto-Recovery**: Self-healing Wi-Fi connection with automatic reconnection

## Technical Details

### Hardware
- Raspberry Pi Pico W
- Wi-Fi capabilities via CYW43439

### Software Stack
- **Language**: C
- **SDK**: Raspberry Pi Pico SDK
- **Network Stack**: lwIP
- **Build System**: CMake (project template is generated via Raspberry Pi Pico W Extension in VS Code)
- **Time Series DB**: InfluxDB

### Key Components

#### Network Monitoring (`ping.c`)
- Custom ICMP echo implementation
- Microsecond-precision timing
- Configurable ping count and timeout
- Statistical analysis (RTT, jitter, packet loss)

#### Data Management (`influxdb.c`)
- HTTP client for InfluxDB communication
- Line protocol formatting
- Retry mechanism with exponential backoff
- Error handling and recovery

#### Wi-Fi Management (`wifi.c`)
- Station mode configuration
- Connection monitoring
- Automatic reconnection

## Building and Running

I used Raspberry Pi Pico W VS Code extension to generate an empty Pico project from template and then updated `CMakeLists.txt` and `pico_sdk_import.cmake` for my needs. Since I don't have SWD probe, the only type of debugging available for me is via printf messages. In order to upload firmware to RP2040 I use BOOTSEL mode and copy uf2 file to  the target. 

## Configuration

Edit `config.h` to set:
- Wi-Fi credentials
- Router IP address
- InfluxDB connection details
- Measurement intervals
- Retry parameters

## Data Visualization

### InfluxDB Schema
```
measurement: wifi_measurements
tags:
  - host: PicoW
fields:
  - rtt_avg (microseconds)
  - rtt_min (microseconds)
  - rtt_max (microseconds)
  - jitter (microseconds)
  - loss (percentage)
  - temperature (Celsius)
```

### Example Grafana Dashboard
[Grafana Dashboard Example](https://dashboard.mykola-ablapokhin.dev/d/35latfvs1zc3f6f/wi-fi-latency-meter?orgId=1&from=now-24h&to=now&timezone=browser&refresh=30s)

## Technical Implementation

### ICMP Echo Implementation
```c
typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
    uint32_t timestamp;
} ICMP_EchoHeader_t;
```

The project uses raw sockets to implement ICMP echo requests, calculating checksums and handling responses with microsecond precision.

### Error Handling
- Exponential backoff for failed operations
- Maximum retry limits
- Automatic Wi-Fi reconnection