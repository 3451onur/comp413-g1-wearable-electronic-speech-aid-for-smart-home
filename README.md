# comp413-g1-wearable-electronic-speech-aid-for-smart-home
Gesture Recognition and Smart Control System Powered by TinyML
Overview
This project introduces a state-of-the-art wearable device designed for real-time gesture recognition and smart home control. Leveraging resistive sensors and TinyML, the system translates hand gestures into letters and words, enabling seamless communication and interaction with smart devices. A novel resistance-to-time-to-digital conversion technique ensures fast, energy-efficient data acquisition without reliance on traditional ADCs, making the system highly adaptable and scalable.
The core components of the system include:
•	Two Sender Units: Capture resistance changes from finger movements and transmit the data wirelessly.
•	One Receiver Unit: Processes the data using a TinyML model, predicts letters, and combines them into meaningful words.
•	Web Interface: Provides a real-time dashboard for data visualization, predicted letters, and finalized words.
•	Smart Home Control: Allows users to control IoT devices, such as LEDs, with intuitive gestures.
________________________________________
Hardware Requirements
Essential Components
1.	ESP32 Development Boards (3 units)
o	Two for data collection (sender units).
o	One for data processing and control (receiver unit).
2.	Potentiometers (10 units)
o	Measure finger movements via resistance changes.
3.	LED Indicators (3 units: Green, Red, Blue)
o	Simulate smart home device control.
4.	Breadboard and Jumper Wires
o	For circuit prototyping and connections.
5.	Power Source
o	USB or battery-powered for ESP32 boards.
________________________________________


System Architecture
1.	Data Acquisition:
o	The sender units read resistance values from potentiometers representing finger positions.
o	Resistance values are converted to time intervals using a resistance-to-time-to-digital converter for optimized power consumption.
2.	Data Processing:
o	The receiver ESP32 uses a pre-trained TinyML model to predict letters based on incoming data.
o	Letters are dynamically combined into words, finalized with a reset gesture.
3.	Control and Visualization:
o	The system interfaces with a web-based dashboard, displaying real-time resistance data, predicted letters, and finalized words.
o	Smart home control functionality is integrated via GPIO-connected LEDs, triggered by specific words.
________________________________________
Setup and Installation
Hardware Setup
1.	Connect potentiometers (R1–R10) to the digital pins of the sender ESP32 boards.
2.	Wire LEDs to the receiver ESP32 GPIO pins for control simulation.
3.	Ensure all ESP32 boards are powered and connected to the same Wi-Fi network.
Software Setup
1.	Development Environment:
o	Install the Arduino IDE and ESP32 board support package.
o	Install required libraries:
	ESPAsyncWebServer
	ArduinoJson
	Edge Impulse TinyML Libraries
2.	Code Deployment:
o	Upload the sender codes to the two ESP32 sender units, configured with Wi-Fi credentials.
o	Upload the receiver code to the ESP32 receiver unit.

3.	Dashboard Access:
o	Enter the receiver ESP32’s IP address in a web browser to access the real-time dashboard.
________________________________________
Key Features
•	TinyML-Powered Gesture Recognition: Predicts letters with 96% accuracy, dynamically forming words.
•	Energy Efficiency: Utilizes a resistance-to-time-to-digital conversion mechanism for low-power data acquisition.
•	Real-Time Visualization: Displays live sensor data, predictions, and finalized words on a web-based interface.
•	Smart Home Control: Allows gestures to trigger predefined actions, such as controlling LEDs.
•	User Adaptability: Accommodates different hand sizes and finger configurations with TinyML training.
________________________________________
Future Enhancements
1.	3D Motion Tracking: Integrating sensors to enhance gesture recognition precision.
2.	AR Integration: Pairing with smart glasses for real-time visual feedback of words and gestures.
3.	Multi-Language Support: Expanding datasets to include gestures from different sign languages.
4.	IoT Ecosystem Expansion: Enabling broader control of smart home devices.
5.	Battery Optimization: Improving portability and runtime with enhanced power management.
________________________________________
Video Demonstration Link
https://drive.google.com/file/d/1nVtKswSbq18www2I7AMtYHJBAkyaB2xD/view?usp=sharing

