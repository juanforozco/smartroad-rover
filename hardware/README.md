# Hardware — SmartRoad Rover

Documentación del hardware del proyecto. Este archivo refleja las conexiones reales del prototipo montado sobre protoboard.

---

## Sistema de alimentación

| Componente | Descripción |
|---|---|
| Batería | 3 celdas 18650 en serie — ~11.1V nominales |
| Regulador | Step-down DM02-28050016DS — salida ajustada a 5V |

### Distribución de voltajes

| Dominio | Voltaje | Alimenta |
|---|---|---|
| Potencia motores | ~11.1V directo batería | Pin VMOT del TB6612FNG |
| Lógica | 5V regulados (Vo+ step-down) | Bus (+) protoboard → VSYS Pico W, VCC HC-SR04 ×3, VCC GPS |
| Lógica driver | 3.3V | VCC TB6612FNG desde pin 3V3OUT del Pico W |
| GND común | 0V | Bus (−) protoboard, GND driver, GND Pico W |

### Conexiones step-down

| Pin step-down | Conexión |
|---|---|
| Vi+ | Batería (+) y VMOT TB6612FNG |
| Vi− | Batería (−) y GND TB6612FNG |
| Vo+ | Bus (+) protoboard — 5V regulados |
| Vo− | Bus (−) protoboard — GND común |

---

## Asignación de pines GPIO del Pico W

> Nota: la asignación de pines difiere de la planificada en el Entregable 3 por decisiones de ergonomía del montaje físico.

| Señal | GPIO | Pin físico | Dirección |
|---|---|---|---|
| TRIG ×3 sensores (compartido) | GP15 | 20 | Salida digital |
| ECHO sensor derecho | GP18 | 24 | Entrada (divisor de tensión) |
| ECHO sensor centro | GP20 | 26 | Entrada (divisor de tensión) |
| ECHO sensor izquierdo | GP19 | 25 | Entrada (divisor de tensión) |
| AIN1 Motor A | GP7 | 10 | Salida digital — dirección |
| AIN2 Motor A | GP8 | 11 | Salida digital — dirección |
| PWMA Motor A | GP9 | 12 | Salida PWM — velocidad |
| BIN1 Motor B | GP11 | 15 | Salida digital — dirección |
| BIN2 Motor B | GP12 | 16 | Salida digital — dirección |
| PWMB Motor B | GP13 | 17 | Salida PWM — velocidad |
| GPS TX → Pico RX | GP1 | 2 | UART0 RX |
| VSYS | Pin 39 | — | Entrada alimentación 5V |
| GND | Pin 38 | — | Tierra común |
| 3V3OUT | Pin 36 | — | 3.3V → VCC TB6612FNG |

---

## Driver de motores TB6612FNG

| Pin TB6612FNG | Conexión |
|---|---|
| VMOT | Vi+ step-down (~11.1V) |
| VCC | 3V3OUT Pico W (Pin 36) |
| GND | Bus (−) protoboard |
| STBY | VCC del driver — siempre habilitado |
| AIN1 | GP7 Pico W |
| AIN2 | GP8 Pico W |
| PWMA | GP9 Pico W |
| BIN1 | GP11 Pico W |
| BIN2 | GP12 Pico W |
| PWMB | GP13 Pico W |
| AO1 | Motor derecho (−) |
| AO2 | Motor derecho (+) |
| BO1 | Motor izquierdo (+) |
| BO2 | Motor izquierdo (−) |

> Motor derecho = motor lado derecho viendo el rover de frente.
> Motor izquierdo = motor lado izquierdo viendo el rover de frente.

---

## Sensores HC-SR04

Tres sensores ultrasónicos. Todos comparten el mismo pin TRIG. Cada ECHO pasa por su divisor de tensión antes de llegar al GPIO.

| Sensor | TRIG | ECHO (GPIO) |
|---|---|---|
| Centro (frontal) | GP15 | GP20 |
| Derecho | GP15 | GP18 |
| Izquierdo | GP15 | GP19 |

### Circuito divisor de tensión por cada sensor ECHO (×3)

Adapta la señal ECHO de 5V a 3.33V compatible con los GPIOs del Pico W (máx. 3.3V).

```
ECHO sensor (5V)
      │
     R1 = 1kΩ
      │
      ├──────────── GPIO Pico W (3.33V)
      │
     R2 = 2kΩ
      │
     GND

Vout = 5V × 2000/(1000+2000) = 3.33V
```

Resistencias de precisión 1% utilizadas.

---

## Módulo GPS GY-GPS6MV2

| Pin GPS | Conexión |
|---|---|
| VCC | Bus (+) 5V protoboard |
| GND | Bus (−) protoboard |
| TX | GP1 Pico W (UART0 RX) |
| RX | Sin conectar |

---

## Indicador de encendido

LED conectado entre el pin 3V3OUT del Pico W y GND con resistencia limitadora. Indica que el sistema está energizado.

---

## Notas importantes

- El STBY del TB6612FNG está conectado permanentemente a VCC (siempre habilitado). No se controla por software.
- Los tres sensores HC-SR04 comparten el mismo pin TRIG (GP15). El disparo es simultáneo y la identificación de cada sensor se realiza por el pin ECHO correspondiente.
- El pin RX del GPS está suelto ya que el sistema solo lee datos del GPS, no escribe.
- GND común es crítico: bus (−) de protoboard, GND del driver y GND del Pico W están todos conectados al mismo punto.
