# DistributedFenceMonitor

Repository for the Embry-Riddle Aeronautical University Distributed Fence Monitor Capstone.

*In Association with ICARUS and ERP.*

## How to Use

- The node source file is 'src/dfm_mkr1310.cpp'.
- The central-node source file is 'src/dfm_recv_mkr1310.cpp'.
- The central server file is 'interpreter/dfm_recv.m'
- The "main.cpp" is set up so that PlatformIO can compile and upload multiple sources separately, depending on the environment.

### Project Members

- Briellyn Braithwaite
- Jack Ramsay
- Lucas Marin
- Mike Paff
- Renzo Mena
- Zachary Couch

### Advisors

- Dr. Zapata
- Dr. Aranzazu
- Dr. Bruder
- Dr. Dorfling
- Dr. Sevic

## Configuration Features

All values settable in *dfm_mkr1310.h*

### System

- Node Identifier
- Peripheral Connection Reattempts (default: 5)
- Debug Interface on/off (default: on)
- Debug Interface Speed (default: 115200)
- Debug Interface Timeout (default: 10 sec)
- ADC Bit Depth (default: 10, max: 12)

### Motion Sensing

- Sample Rate (default: 100 Hz, max: 400 Hz)
- Sensitivity (default: 2g)
- Activity Threshold (default: 00007, min: 00002)
- Inactive Timer (default: 0.8 sec, min: 0.8)
- Severity Thresholds
- Resolution (default: 10 bit @ 256 LSB/g)

### Timing

- Reporting Interval (default: 15 sec, min: 5)
- Maximum Readings to Take (default: 2x)
- Sampling Duration (default: 500 samples, max: 2000)
- Data Timeout (default: 50 ms)

### Calibration

- Calibration Interval (default: 15 minutes, min: 1, max: 15)
- Calibration Duration (default: 300 samples)
- Calibration Time Slice (default: 0.5 sec, min: 0.5, max: 2.0)

### Low-Pass Filter

- Cutoff Frequency (default: 8 Hz)
- Impulse Response Length (default: 16)

### PCB Wiring Configuration

- Pinout
  - Mode pin (default: 2)
  - Status pin (default: 6)
  - Error pin (default: 3)
  - Battery pin (default: A1)
  - Switch pin (default: A5)
  - CS pin for ADXL1 (default: A3)
  - CS pin for ADXL2 (default: A4)
  - Interrupts pin (default: 7)
  - Temperature pin (default: 4)
- Resistor Voltage Divider Values (default: 680k/330k)

### LoRa Configuration

- Transmission Frequencies (default: 914.9 MHz)
- Transmission Power (default: 4dB, min: 2, max: 17)
- Acknowledgement Timeout (default: 1 sec)
- Reject CRC Failures (default: false)
- Bandwidth (default: 125kHz, max: 250kHz)
- Spreadfactor (default: 7, min: 7, max: 12)
- Syncword (default: 0012)
