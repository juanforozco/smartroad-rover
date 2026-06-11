# Firmware — SmartRoad Rover

Código fuente del microcontrolador Raspberry Pi Pico W.

**Lenguaje:** C  
**SDK:** Raspberry Pi Pico SDK v2.2.0  
**Arquitectura:** Polling + IRQ  

---

## Estructura del proyecto

```
firmware/
└── smartroad-rover/
    ├── CMakeLists.txt           # Configuración del proyecto Pico SDK
    ├── smartroad-rover.c        # Main: loop principal y despacho de la FSM
    ├── lwipopts.h               # Configuración lwIP para WiFi
    ├── pico_sdk_import.cmake    # Importación del SDK
    ├── src/
    │   ├── sensors.c            # Lectura HC-SR04: disparo TRIG (polling), ISR ECHO, filtrado
    │   ├── motors.c             # Control PWM de motores DC vía TB6612FNG
    │   ├── navigation.c         # Algoritmo de evasión autónoma de obstáculos
    │   ├── fsm.c                # FSM principal: estados y transiciones
    │   ├── web_server.c         # Servidor HTTP embebido (lwIP) — pendiente
    │   └── gps.c                # UART0 + DMA para GPS GY-GPS6MV2 — pendiente
    └── include/
        ├── sensors.h
        ├── motors.h
        ├── navigation.h
        ├── fsm.h
        ├── web_server.h         # pendiente
        └── gps.h                # pendiente
```

## Asignación de pines — referencia rápida

| Señal | GPIO |
|---|---|
| TRIG ×3 sensores | GP15 |
| ECHO sensor derecho | GP18 |
| ECHO sensor centro | GP20 |
| ECHO sensor izquierdo | GP19 |
| AIN1 Motor A | GP7 |
| AIN2 Motor A | GP8 |
| PWMA Motor A | GP9 |
| BIN1 Motor B | GP11 |
| BIN2 Motor B | GP12 |
| PWMB Motor B | GP13 |
| GPS TX → Pico RX | GP1 (UART0) |

## Estado de implementación

| Módulo | Estado |
|---|---|
| `motors.c` | En desarrollo |
| `sensors.c` | Pendiente |
| `navigation.c` | Pendiente |
| `fsm.c` | Pendiente |
| `web_server.c` | Pendiente |
| `gps.c` | Pendiente |

## Cómo compilar

Desde VS Code con la extensión Raspberry Pi Pico:
1. Abrir la carpeta `firmware/smartroad-rover/`
2. Clic en **"Compile"** en la barra inferior
3. El archivo `.uf2` se genera en `build/`
4. Conectar el Pico W en modo BOOTSEL y copiar el `.uf2`