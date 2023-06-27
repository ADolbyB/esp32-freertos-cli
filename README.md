# ESP32 FreeRTOS CLI

This is a Command Line Interface for the ESP32 using the ESP32 multicore port of FreeRTOS.
This project started in the [`ADolbyB/rtos-esp32-examples`](https://github.com/ADolbyB/rtos-esp32-examples) 
repository as `04-LED-CLI`, but it took on a life of its own and it turned into this project.

Currently, the CLI can perform the following on the ESP32 Thing Plus C from SparkFun.
 - Command the RGB LED connected to `GPIO_2`.
 - The Blue LED connected to `GPIO_13`.
 - [In Progress] The on-board SD Card Slot connected to `GPIO_5`.

This is a work in progress.

Reference:

 - Read the [ESP32 Arduino Core Documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/index.html).

 - Read the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html).

 - The board I used for practice is an [ESP32 Thing Plus C](https://www.sparkfun.com/products/18018) from [SparkFun](https://www.sparkfun.com/).

 ## Status:

 ![GitHub repo size](https://img.shields.io/github/repo-size/ADolbyB/esp32-freertos-cli?label=Repo%20Size&logo=Github)
