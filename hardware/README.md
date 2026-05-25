# Hardware — SmartRoad Rover

Documentación del hardware del proyecto.

---

## Contenido

| Archivo | Descripción |
|---|---|
| Esquemático | Diagrama de conexiones completo del sistema |
| Asignación de pines | Tabla de pines GPIO del Raspberry Pi Pico W |

## Asignación de pines GPIO

| Señal | GPIO | Pin físico | Dirección |
|---|---|---|---|
| TRIG sensor frontal | GP0 | 1 | Salida |
| ECHO sensor frontal | GP1 | 2 | Entrada (divisor de tensión) |
| TRIG sensor izquierdo | GP2 | 4 | Salida |
| ECHO sensor izquierdo | GP3 | 5 | Entrada (divisor de tensión) |
| TRIG sensor derecho | GP4 | 6 | Salida |
| ECHO sensor derecho | GP5 | 7 | Entrada (divisor de tensión) |
| GPS TX → Pico RX | GP9 | 12 | UART1 RX |
| GPS RX → Pico TX | GP8 | 11 | UART1 TX |
| IN1 Motor A | GP10 | 14 | Salida |
| IN2 Motor A | GP11 | 15 | Salida |
| PWM Motor A | GP12 | 16 | Salida PWM |
| IN1 Motor B | GP13 | 17 | Salida |
| IN2 Motor B | GP14 | 19 | Salida |
| PWM Motor B | GP15 | 20 | Salida PWM |
| STBY TB6612FNG | GP16 | 21 | Salida |

## Circuitos analógicos requeridos

1. Divisor de tensión ECHO frontal: R1=1kΩ, R2=2kΩ (5V → 3.33V)
2. Divisor de tensión ECHO izquierdo: R1=1kΩ, R2=2kΩ
3. Divisor de tensión ECHO derecho: R1=1kΩ, R2=2kΩ
4. Regulador step-down 7.4V → 5V (módulo MP1584 o similar)
