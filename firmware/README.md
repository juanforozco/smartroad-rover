# Firmware

Aquí se almacenará el código del microcontrolador Raspberry Pi Pico W.

# Firmware — SmartRoad Rover

Código fuente del microcontrolador Raspberry Pi Pico W.

**Lenguaje:** C  
**SDK:** Raspberry Pi Pico SDK oficial  
**Arquitectura:** Polling + IRQ  

---

## Estructura planificada

```
firmware/
├── CMakeLists.txt          # Configuración del proyecto Pico SDK
├── src/
│   ├── main.c              # Loop principal y despacho de la FSM
│   ├── sensors.c           # Lectura HC-SR04: disparo TRIG (polling), ISR ECHO, filtrado
│   ├── motors.c            # Control PWM de motores DC vía TB6612FNG
│   ├── navigation.c        # Algoritmo de evasión autónoma de obstáculos
│   ├── web_server.c        # Servidor HTTP embebido (lwIP), watchdog WiFi
│   ├── gps.c               # UART1 + DMA para GPS NEO-6M, watchdog GPS
│   └── fsm.c               # FSM principal: 5 estados y transiciones
└── include/
    ├── sensors.h
    ├── motors.h
    ├── navigation.h
    ├── web_server.h
    ├── gps.h
    └── fsm.h
```

## Estado

El firmware está en fase de diseño. La arquitectura completa está documentada en `docs/entregable3/`. La implementación comienza en la Semana 4 del plan de trabajo.
