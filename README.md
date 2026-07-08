# Fire-Fighting Robot

An autonomous Arduino-based robot capable of detecting a flame, navigating toward it, and automatically activating a water pump to extinguish the fire. This project demonstrates the fundamentals of embedded systems, robotics, sensor integration, and autonomous decision-making.

---

## Overview

The Fire-Fighting Robot is a proof-of-concept prototype designed to detect small flames using three infrared flame sensors. Based on the detected flame direction, the robot navigates autonomously using a differential drive system. Once it reaches a suitable distance, it stops and activates a water pump while adjusting a servo-mounted nozzle to spray water toward the flame.

The project combines sensing, control, mobility, and actuation into a single embedded robotic platform.

---

## Features

- 🔥 Autonomous flame detection
- 📍 Direction estimation using three flame sensors
- 🚗 Automatic navigation toward the fire
- 💧 Water pump activation for fire suppression
- 🎯 Servo-controlled water nozzle
- ⚙️ Arduino UNO based control system
- 🔋 Battery-powered mobile platform

---

## 🛠 Hardware Components

- Arduino UNO
- 3 × IR Flame Sensors
- L298N Motor Driver
- 4WD Robot Chassis
- 4 DC Gear Motors
- SG90 Servo Motor
- Mini Water Pump
- TIP120 Transistor (Pump Switching)
- 18650 Battery Pack
- On/Off Switch
- Jumper Wires

---

## ⚡ System Architecture

The robot is divided into four functional subsystems:

- **Detection System** – Three flame sensors continuously scan for fire.
- **Control System** – Arduino UNO processes sensor inputs and makes navigation decisions.
- **Mobility System** – L298N motor driver controls the four DC motors.
- **Suppression System** – Water pump and servo extinguish the detected flame.

---

## 🔄 Control Logic

The robot follows the control sequence below:

```text
Start system

Initialize sensors, motors, pump, and servo

Loop:

Read left, center, and right flame sensors

If no flame is detected:
    Stop motors or keep scanning

Else if center sensor detects flame:
    Move forward

Else if left sensor detects flame:
    Turn left

Else if right sensor detects flame:
    Turn right

If suitable distance is reached:
    Stop motors
    Activate pump
    Adjust servo/nozzle if needed
    Spray water toward flame

End loop
```

---

## 🚀 How It Works

1. Power on the robot.
2. The flame sensors continuously scan the surroundings.
3. The Arduino determines where the flame is located.
4. The robot turns toward the flame.
5. The robot moves forward.
6. Once close enough, the robot stops.
7. The water pump activates.
8. The servo directs the nozzle toward the flame.
9. Water is sprayed until the extinguishing sequence is complete.

---

## 📈 Possible Improvements

- Thermal camera integration
- Ultrasonic/LiDAR obstacle avoidance
- Smoke and gas sensors
- Wi-Fi or Bluetooth monitoring
- Automatic distance measurement
- PID motor control
- Rechargeable battery management
- ROS implementation
- ESP32 or Raspberry Pi upgrade
- Machine vision for flame localization

---

## 📚 Project Report

A complete technical report describing the system design, hardware architecture, software algorithm, experimental testing, and future improvements is included in this repository.

**File:**

`Fire Fighting Robot.pdf`
