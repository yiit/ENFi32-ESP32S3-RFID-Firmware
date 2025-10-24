# ENFi32 ESP32-S3 RFID Firmware

A comprehensive ESPEasy-based IoT solution for ESP32-S3 microcontroller with LVGL touchscreen interface and RFID capabilities.

## 🌟 Features

- **ESP32-S3 Platform**: 16MB Flash, 8MB LittleFS, USB CDC support
- **LVGL Graphics**: Advanced touchscreen interface with Turkish font support
- **RFID Integration**: USB HID RFID reader functionality
- **IoT Connectivity**: Full ESPEasy ecosystem support
- **High Code Quality**: Comprehensive static analysis fixes and optimizations

## 🛠️ Technical Specifications

### Hardware
- **Microcontroller**: ESP32-S3
- **Flash Memory**: 16MB
- **File System**: 8MB LittleFS
- **Display**: LVGL-compatible touchscreen
- **Communication**: USB CDC, WiFi, Bluetooth

### Software Stack
- **Framework**: ESPEasy IoT Platform
- **Graphics**: LVGL v9.x with custom Turkish fonts
- **Libraries**:
  - ArduinoJson 6.21.5
  - LovyanGFX 1.2.7
  - ESP32_USB_Host_HID (latest)

## 🔧 Build Configuration

### PlatformIO Environment
```ini
[env:custom_ESP32s3_16M8M_LittleFS_CDC]
extends = esp32s3_base
board = esp32-s3-devkitc-1
build_flags = -fno-lto  ; Resolves LovyanGFX ODR warnings
```

### Key Build Features
- **Link Time Optimization**: Disabled to resolve library conflicts
- **Memory Layout**: Optimized for large applications
- **USB Support**: Native USB CDC for debugging and communication

## 🚀 Quick Start

1. **Clone Repository**
   ```bash
   git clone https://github.com/yiit/ENFi32-ESP32S3-RFID-Firmware.git
   cd ENFi32-ESP32S3-RFID-Firmware
   ```

2. **Install Dependencies**
   ```bash
   pio lib install
   ```

3. **Build Firmware**
   ```bash
   pio run -e custom_ESP32s3_16M8M_LittleFS_CDC
   ```

4. **Upload to Device**
   ```bash
   pio run -e custom_ESP32s3_16M8M_LittleFS_CDC -t upload
   ```

## 📁 Project Structure

```
├── src/                    # Main source code
│   ├── custom/            # Custom LVGL configurations
│   ├── generated/         # LVGL GUI generated files
│   ├── usb/              # USB HID RFID implementation
│   └── src/              # ESPEasy core source
├── lib/                   # Project libraries
│   ├── lvgl/             # LVGL graphics library
│   ├── pubsubclient3/    # MQTT client
│   └── ...
├── static/               # Web interface assets
├── boards/               # Custom board definitions
└── tools/                # Build and utility scripts
```

## 🎨 User Interface

The device features a comprehensive LVGL-based touchscreen interface with:

- **Multi-language Support**: Turkish font integration
- **Custom Graphics**: Logo, icons, and status indicators
- **Touch Controls**: Intuitive button and navigation system
- **WiFi Status**: Visual connection indicators
- **Settings Management**: Complete configuration interface

## 🔐 Security Features

- **Static Analysis**: Fixed critical va_start security vulnerability
- **Code Quality**: 2,017+ code quality issues resolved
- **Memory Safety**: Comprehensive null pointer checks
- **Type Safety**: Improved casting and variable scoping

## 🔄 Recent Improvements

### Code Quality Enhancements
- ✅ Fixed critical `va_start` security vulnerability in StringConverter.cpp
- ✅ Resolved 2,017 static analysis issues across 514 files
- ✅ Added comprehensive null pointer checks
- ✅ Improved constructor initializations
- ✅ Enhanced variable scoping and type safety

### Build Optimizations
- ✅ Resolved LovyanGFX ODR warnings with -fno-lto flag
- ✅ Optimized memory usage and performance
- ✅ Streamlined compilation process

## 📚 Documentation

- **ESPEasy Wiki**: [ESPEasy Documentation](https://espeasy.readthedocs.io/)
- **LVGL Docs**: [LVGL Graphics Library](https://docs.lvgl.io/)
- **ESP32-S3**: [Espressif ESP32-S3 Datasheet](https://www.espressif.com/en/products/socs/esp32-s3)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project includes components from ESPEasy, which is licensed under the GPL-3.0 License. See individual files for specific licensing information.

## 🔗 Links

- **Repository**: [GitHub](https://github.com/yiit/ENFi32-ESP32S3-RFID-Firmware)
- **Issues**: [Bug Reports & Feature Requests](https://github.com/yiit/ENFi32-ESP32S3-RFID-Firmware/issues)
- **ESPEasy**: [Original Project](https://github.com/letscontrolit/ESPEasy)

---

**Note**: This firmware has been optimized for production use with comprehensive code quality improvements and security fixes. The backup includes all critical fixes applied through static analysis and build optimization processes.