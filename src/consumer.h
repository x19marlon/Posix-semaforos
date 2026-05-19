/**************************************************
* Pontificia Universidad Javeriana
* @Author: Marlon Garcia
* Fecha: 19 de Mayo del 2026
* Objetivos:
*    -Programar un sistema de productor-consumidor
*      utilizando memoria compartida y semáforos POSIX.
**************************************************/

#ifndef CONSUMER_H
#define CONSUMER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

// Tamaño máximo del búfer circular compartido
#define BUFFER 5

/*
 * Estructura de datos compartida entre el Productor y el Consumidor.
 * Se almacena en memoria compartida POSIX y es accedida por ambos procesos.
 */
typedef struct {
    int bus[BUFFER];   // Búfer circular para almacenar los elementos producidos
    int entrada;       // Índice donde el productor insertará el próximo elemento
    int salida;        // Índice donde el consumidor retirará el próximo elemento
    int terminado;     // Bandera (flag) que indica que el productor ha terminado de enviar datos
} compartir_datos;

#endif 

