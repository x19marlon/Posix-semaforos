# Posix-semaforos

Este repositorio contiene la solución al laboratorio de Sistemas Operativos sobre concurrencia, sincronización de procesos e hilos, y comunicación entre procesos utilizando POSIX. 

El proyecto fue desarrollado en C y aborda el clásico problema de **Productor-Consumidor** mediante dos enfoques distintos de la API POSIX: sincronización por hilos (mutex y variables de condición) y memoria compartida con semáforos nombrados.

## Contenido del Repositorio

La estructura del proyecto está organizada de la siguiente manera:

- `src/`: Carpeta que contiene todo el código fuente en C.
  - `posixSincro.c`: Implementación del problema productor-consumidor utilizando múltiples hilos (pthreads), exclusión mutua (mutex) y variables de condición en un único proceso.
  - `producer.c`: Implementación de un proceso Productor independiente que crea y escribe en un segmento de memoria compartida POSIX.
  - `consumer.c`: Implementación de un proceso Consumidor independiente que se conecta al segmento de memoria compartida y lee los datos.
  - `consumer.h`: Archivo de cabecera con constantes y estructuras de memoria compartida para productor y consumidor.
- `Makefile`: Archivo de automatización para compilar todos los ejecutables del proyecto desde la raíz.
- `taller_3 .pdf`: Documento con las instrucciones originales del laboratorio.

## Requisitos

- Sistema operativo compatible con POSIX (Linux, macOS, etc.)
- Compilador de C (ej. `gcc`)
- Biblioteca `pthread` y `rt` (Real-Time) para hilos y memoria compartida.

## Compilación

Para compilar todo el proyecto, ejecuta el siguiente comando en la raíz del repositorio:

```bash
make
```

Esto generará los siguientes ejecutables:
- `posixSincro`
- `producer`
- `consumer`

Para limpiar los ejecutables generados, puedes ejecutar:

```bash
make clean
```

## Ejecución

### 1. Sincronización con Hilos
Para ejecutar la simulación de hilos (productores y spooler):

```bash
./posixSincro
```

### 2. Memoria Compartida y Semáforos (Procesos Independientes)
Este enfoque requiere ejecutar el productor y el consumidor en terminales separadas, o en segundo plano.

Primero inicia el productor (quien crea la memoria compartida y semáforos):
```bash
./producer &
```

Y luego ejecuta el consumidor:
```bash
./consumer
```

## Autor
- **Marlon Garcia** - *Pontificia Universidad Javeriana* - 19 de Mayo del 2026