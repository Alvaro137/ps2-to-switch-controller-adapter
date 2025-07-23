# PS2 → Nintendo Switch Adapter

Este proyecto permite leer las entradas de un mando PlayStation 2 (botones, D‑Pad y ejes analógicos) y traducirlas a comandos de un gamepad de Nintendo Switch usando un ESP32 con soporte OTG (probado en un ESP32‑S3). Recomendable usar cualquier volante con pedales para PS2, pues son muy baratos de segunda mano, a diferencia de los volantes y pedales compatibles con la Switch. 

---

## Descripción

- Se conecta un mando PS2 a los pines GPIO del ESP32.
- Se usan las librerías **PsxNewLib** y **switch_ESP32** para emular un dispositivo HID compatible con Nintendo Switch.  
- Lógica de mapeo analógico para volante, freno, acelerador y embrague.  
- Sistema de logging condicional para depuración.

---

## Requisitos

### Hardware
- ESP32 con soporte USB OTG (por ejemplo, ESP32‑S3).  
- Mando PS2 (volante y pedales).
- Cable USB-C a USB-C o dock.

---

## Recursos útiles
- [esp32beans/switch_ESP32](https://github.com/esp32beans/switch_ESP32)  
- [lefmarna/NintendoSwitchControlLibrary](https://github.com/lefmarna/NintendoSwitchControlLibrary)  
- [ricardoquesada/bluepad32](https://github.com/ricardoquesada/bluepad32)  
- [Bluepad32 Documentation](https://bluepad32.readthedocs.io/en/latest/)  
- [nullstalgia/UARTSwitchCon](https://github.com/nullstalgia/UARTSwitchCon)  
- [alfilipe/SwitchCon](https://github.com/alfilipe/SwitchCon/tree/master)

---

## Instalación

1. Clona este repositorio:
   ```bash
   git clone https://github.com/Alvaro137/ps2-to-switch-controller-adapter.git
   cd ps2-to-switch-controller-adapter
2. Conecta hardware:

    PS2_DAT, PS2_CMD, PS2_CLK, PS2_SEL a los GPIO definidos en main.cpp.

    Ejes analógicos (VOLANTE, FRENO, ACELERADOR, EMBRAGUE) a las entradas ADC configuradas.
3. Selecciona la placa ESP32‑S3 en tu IDE y carga el firmware. (Recomendable usar PlatformIO + vscode)

---

## Uso

1. Conecta el ESP32 puerto USB-C al dock o puerto USB‑C de la Switch.

2. Enciende la Nintendo Switch y selecciona perfil de control.

3. Usa el mando PS2: la Switch lo reconocerá como un Pro Controller.
