#  Smart Home Energy Monitor + Custom RTOS (STM32F4)

This project implements a **student-built Real-Time Operating System (RTOS)** on an **STM32F446RE microcontroller**, paired with an **ESP8266-based WiFi dashboard** for real-time monitoring and control of household loads such as LED lamps and fans.  

It features:
- A custom round-robin RTOS  
- Dual INA219 power sensors  
- Relay-controlled load switching  
- A mobile-friendly web UI  
- Live power graphs and device control  

---

#  Hardware Components

### **Microcontroller**
- **STM32F446RE (Nucleo-F446RE)**  
  - Runs custom RTOS  
  - Reads INA219 sensors  
  - Controls relay  

### **Sensors**
- **INA219 High-Side Power Monitor x2**
  - Address **0x40** → Fan circuit  
  - Address **0x41** → Lamp circuit  
  - Measures voltage, current, power  

### **Actuator**
- **1-channel relay module**
  - Controlled by STM32 GPIO  
  - Switches LED lamp safely  
  - Active LOW  

### **Wireless UI**
- **ESP8266 NodeMCU**
  - WiFi dashboard  
  - Real-time graphing  
  - On/off control buttons  
  - Reads both INA219 sensors  

### **Loads**
- USB-powered LED strip (Lamp)  
- Fan (5V USB fan)

---

#  System Architecture

The system is composed of **two cooperating subsystems**:

## **1. STM32 + Custom RTOS (Core embedded system)**  
The STM32 runs:
- A **fully custom RTOS**  
- A cooperative scheduler  
- 1 kHz timing  
- Multiple periodic tasks  
- Automatic voltage-based control  
- Heartbeat LED indicator  

###  RTOS Task Summary
| Task | Rate | Purpose |
|------|------|---------|
| `Task_Sense` | 200 ms | Reads voltage from INA219 |
| `Task_Control` | 200 ms | Automatically toggles relay |
| `Task_Heartbeat` | 500 ms | LED blink showing RTOS alive |

### ✔ RTOS Features Implemented
- Custom task scheduler  
- SysTick-driven timing  
- Context-style task switching  
- Inter-task shared variables  
- Interrupts integrated with scheduler  

---

## **2. NodeMCU WiFi Dashboard (IoT Interface)**

The ESP8266 provides:
- Username/password login  
- Modern dashboard UI  
- JSON `/status` endpoint  
- Power graphs updating every 5 seconds  
- Lamp ON/OFF relay control  

### UI Features  
- Fan voltage/current/power display  
- Lamp voltage/current/power display  
- Status dot showing ON/OFF  
- Graph menu with historical readings  
- Automatic refresh  

---

#  Wiring Overview

### **INA219 (Fan @ 0x40)**

5V USB → INA219 VIN+
INA219 VIN− → Fan + input
Fan GND → Common GND

### **INA219 (Lamp @ 0x41) + Relay**

USB 5V → Relay COM
Relay NO → INA219 VIN+
INA219 VIN− → LED strip +
All grounds shared

### **Relay Coil (Control Line)**

STM32 PA0 → Relay IN
5V → Relay VCC
GND → Relay GND


### **Shared Wiring**
- SCL → PB8  
- SDA → PB9  
- 5V → VIN of INA and relay  
- GND → all modules  

---

#  Software Interaction Flow

### STM32 Responsibilities
- Custom RTOS scheduling  
- INA219 data acquisition  
- Relay logic (threshold-based)  
- Heartbeat LED  
- 1 kHz timing control  

### NodeMCU Responsibilities
- Read INA219 sensors  
- Serve dashboard UI  
- Control relay via HTTP  
- Provide JSON API  
- Display graphs  

---

#  Build & Run Instructions

### **STM32 (RTOS firmware)**
1. Open project in STM32CubeIDE  
2. Build → Flash to board  
3. LED (LD2) will blink every 500 ms  
4. INA219 readings run continuously  

### **ESP8266 (Dashboard)**
1. Open `.ino` in Arduino IDE  
2. Select NodeMCU 1.0 board  
3. Upload firmware  
4. Open Serial Monitor @ **115200 baud**  
5. Copy IP address  
6. Open in browser → Log in → Dashboard loads  

---


