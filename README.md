# SmartRoad Rover

Robot móvil autónomo basado en Raspberry Pi Pico W desarrollado como proyecto final para el curso **Electrónica Digital III**.

---

# Descripción del proyecto

SmartRoad Rover es una plataforma robótica móvil capaz de desplazarse en un entorno controlado evitando obstáculos y ejecutando comandos de movimiento enviados por el usuario.

El sistema está basado en el microcontrolador **Raspberry Pi Pico W**, el cual permite integrar sensores, control de motores y comunicación inalámbrica mediante WiFi dentro de un sistema embebido.

El robot contará con dos modos principales de operación:

**Modo manual**

El usuario podrá controlar el movimiento del robot desde un dispositivo móvil o computador mediante una interfaz web accesible a través de la conexión WiFi generada por el propio sistema. Desde esta interfaz se podrán enviar comandos como avanzar, retroceder, girar o detenerse.

**Modo autónomo**

En este modo el robot se desplazará de forma independiente utilizando sensores de distancia para detectar obstáculos y modificar su trayectoria evitando colisiones.

---

# Motivación

La motivación para este proyecto surge del interés personal en el área de robótica móvil y sistemas de navegación autónoma. Los vehículos autónomos modernos integran sensores, procesamiento embebido y sistemas de control para interactuar con su entorno y desplazarse de forma autónoma.

Este proyecto busca emular, a pequeña escala, algunos de los principios fundamentales utilizados en los sistemas de conducción autónoma. Mediante el uso de sensores, control de motores y procesamiento en un microcontrolador, se pretende desarrollar un prototipo que permita explorar conceptos de robótica móvil y sistemas embebidos.

Además, este proyecto representa una oportunidad para aplicar de manera práctica conocimientos relacionados con microcontroladores, adquisición de datos de sensores, control de actuadores y comunicación inalámbrica dentro de una plataforma robótica funcional.

---

# Arquitectura del sistema

El sistema está compuesto por los siguientes módulos principales:

- Microcontrolador **Raspberry Pi Pico W** encargado del procesamiento del sistema.
- Sensores ultrasónicos **HC-SR04** utilizados para medir la distancia a obstáculos.
- Driver de motores para controlar la velocidad y dirección de los motores DC.
- Plataforma robótica tipo **2WD** para el desplazamiento del robot.
- Interfaz de comunicación inalámbrica mediante **WiFi** para el control del sistema desde un dispositivo móvil.

El microcontrolador procesa continuamente la información proveniente de los sensores y genera las señales de control necesarias para los motores del robot.

---

# Componentes principales

## Microcontrolador

Raspberry Pi Pico W

## Sensores

Sensores ultrasónicos **HC-SR04** (3 unidades)

- Sensor frontal
- Sensor izquierdo
- Sensor derecho

Estos sensores permiten medir la distancia a obstáculos utilizando el principio de tiempo de vuelo de ondas ultrasónicas.

## Plataforma robótica

Chasis robótico tipo **2WD** con dos motores DC controlados mediante un driver de motores.

---

# Resultados esperados

El resultado esperado del proyecto es un robot móvil capaz de:

- Desplazarse de forma autónoma
- Detectar y evitar obstáculos
- Recibir comandos de movimiento desde una interfaz web
- Operar en modo manual y modo autónomo

Este proyecto permitirá aplicar conceptos fundamentales de sistemas embebidos como adquisición de datos de sensores, control de actuadores mediante PWM y comunicación inalámbrica.

---

# Información del curso

Curso: Electrónica Digital III  
Estudiante: Tu nombre  
Universidad: Tu universidad  
Año: 2026
