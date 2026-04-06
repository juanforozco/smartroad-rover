# SmartRoad Rover

Robot móvil autónomo basado en Raspberry Pi Pico W desarrollado como proyecto final para el curso **Electrónica Digital III**.

---

# Descripción del proyecto

SmartRoad Rover es una plataforma robótica móvil capaz de desplazarse en un entorno, evitando obstáculos de forma autónoma y permitiendo el control por parte del usuario mediante una interfaz web.

El sistema está basado en el microcontrolador **Raspberry Pi Pico W**, el cual integra la adquisición de datos de sensores, el procesamiento de información en tiempo real y el control de actuadores, junto con comunicación inalámbrica mediante WiFi.

El robot cuenta con tres modos de operación:

**Modo manual**

El usuario controla el movimiento del robot desde un dispositivo móvil o computador mediante una interfaz web accesible a través de WiFi. Se pueden ejecutar comandos de movimiento como avanzar, retroceder, girar a la izquierda, girar a la derecha y detenerse.

**Modo autónomo reactivo (principal)**

El robot se desplaza de forma independiente utilizando sensores ultrasónicos para detectar obstáculos y ejecutar maniobras de evasión en tiempo real. La lógica de navegación se basa en la comparación de distancias medidas por sensores frontal y laterales.

**Modo autónomo asistido por GPS (extendido)**

El robot puede desplazarse hacia una coordenada objetivo en entornos exteriores, utilizando un módulo GPS. Durante la navegación, el sistema continúa realizando detección y evasión de obstáculos.

---

# Motivación

La motivación del proyecto surge del interés en robótica móvil y sistemas embebidos aplicados a navegación autónoma. Los vehículos autónomos modernos integran sensores, procesamiento y control para interactuar con su entorno.

Este proyecto busca emular estos principios a pequeña escala, implementando un sistema capaz de percibir su entorno, tomar decisiones y actuar sobre él. Además, permite aplicar conocimientos en adquisición de datos, control de motores, comunicación inalámbrica y diseño de sistemas embebidos.

---

# Arquitectura del sistema

El sistema se compone de los siguientes módulos principales:

- Microcontrolador **Raspberry Pi Pico W** encargado del procesamiento del sistema.
- Sensores ultrasónicos **HC-SR04** (frontal, izquierdo y derecho) para detección de obstáculos.
- Driver de motores para el control de velocidad y dirección.
- Plataforma robótica tipo **2WD** para el desplazamiento.
- Comunicación inalámbrica mediante **WiFi** para control remoto.
- Módulo GPS (opcional) para navegación en exteriores.
- Sistema de alimentación basado en batería recargable con regulación de voltaje.
- Implementación final sobre **placa de circuito impreso (PCB)**.

El microcontrolador ejecuta un ciclo continuo de lectura de sensores, toma de decisiones y generación de señales de control hacia los actuadores.

---

# Componentes principales

## Microcontrolador

Raspberry Pi Pico W

## Sensores

Sensores ultrasónicos **HC-SR04** (3 unidades)

- Sensor frontal
- Sensor izquierdo
- Sensor derecho

Estos sensores permiten medir distancias mediante el principio de tiempo de vuelo de ondas ultrasónicas.

## Plataforma robótica

Chasis tipo **2WD** con dos motores DC controlados mediante un driver de motores.

## Sistema de alimentación

Batería recargable (7.4V) con módulo de regulación para alimentar el sistema de control y los actuadores.

---

# Escenario de pruebas

El sistema será evaluado en dos entornos:

**Entorno interior**

- Pruebas de navegación autónoma reactiva
- Validación de detección de obstáculos (< 20 cm)
- Verificación de tiempo de respuesta (< 300 ms)
- Operación continua durante al menos 2 minutos
- Pruebas de control manual mediante interfaz web

**Entorno exterior**

- Pruebas de navegación con GPS
- Movimiento hacia coordenada objetivo
- Aproximación dentro de un radio de 3 metros
- Integración con evasión de obstáculos

---

# Presupuesto

El costo total estimado del proyecto es de aproximadamente:

292,000 COP

Este valor incluye componentes electrónicos, diseño y fabricación de PCB, plataforma mecánica y sistema de alimentación.

---

# Resultados esperados

El sistema desarrollado deberá ser capaz de:

- Desplazarse de forma autónoma evitando obstáculos
- Operar de manera continua sin fallos durante el tiempo de prueba
- Responder en tiempo real a eventos del entorno
- Permitir control manual mediante interfaz web
- Navegar hacia una coordenada objetivo en exteriores (modo GPS)

---

# Información del curso

Curso: Electrónica Digital III  
Estudiante: Juan Felipe Orozco Londoño  
Universidad: Universidad de Antioquia  
Año: 2026
