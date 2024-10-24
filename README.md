# 🎧 Arduino Audio Controller with PT2258
This project is a custom audio controller using Arduino and the PT2258 chip to control audio channels in a 5.1 surround sound system. It uses the I2C protocol to adjust individual channels and the master volume, featuring both a rotary encoder and push-button menus for easy navigation and control.

### Features
* Individual Channel Control: Adjust the volume of each channel (5.1 surround) separately.
* Master Volume Control: Control all channels together.
* PT2258: Digital 6-channel volume control via I2C.
* Push-Button Menus: Navigate through the settings and controls using push buttons.
* 7-Segment Display: Shows current volume levels and selected channels.
* 16x2 LCD Display: Displays additional information like current mode and settings.
* Rotary Encoder: Fine-tune volume and navigate through menus easily.
* EEPROM Support: Automatically saves your settings, so they persist after power-off.

### Components
* Arduino: Manages I2C communication and system controls.
* PT2258: Digital volume control for six audio channels.
* TM1628: Controls the 7-segment display.
* 16x2 LCD: Provides detailed information on settings and modes.
* Rotary Encoder: For adjusting volume and navigating menus.
* Push Buttons: Allows quick selection of menu options and control modes.
* EEPROM: Saves settings and volume levels for the next session.

