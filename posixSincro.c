/**************************************************
* Pontificia Universidad Javeriana
* @Author: Marlon Garcia
* Fecha: 05 de Mayo del 2026
* Objetivos:
*	-Programar un sistema de productor-consumidor 
utilizando memoria compartida y semáforos POSIX.
**************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Número máximo de búferes/ranuras temporales para almacenar líneas de salida
#define MAX_BUFFERS 5

// Búfer compartido de cadenas de texto (almacena el contenido a imprimir)
char buf[MAX_BUFFERS][100];
int buffer_index;          // Índice donde los productores escribirán el siguiente elemento
int buffer_print_index;    // Índice donde el spooler leerá el siguiente elemento a imprimir

// Mutex para controlar el acceso exclusivo a los búferes y variables compartidas
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;

// Variable de condición para indicar a los productores que hay búferes libres disponibles
pthread_cond_t buf_cond = PTHREAD_COND_INITIALIZER;

// Variable de condición para indicar al spooler que hay líneas listas para imprimir
pthread_cond_t spool_cond = PTHREAD_COND_INITIALIZER;

int buffers_available = MAX_BUFFERS; // Contador de ranuras libres en el búfer
int lines_to_print = 0;              // Contador de líneas pendientes de ser impresas por el spooler
int active_producers = 0;            // Lleva la cuenta de hilos productores actualmente activos

// Declaración de funciones de los hilos
void *producer(void *arg);
void *spooler(void *arg);

int main(int argc, char **argv) {
  (void)argc; // Evitar advertencias de compilación
  (void)argv;
  
  pthread_t tid_producer[10], tid_spooler;
  int i, r;

  buffer_index = buffer_print_index = 0;
  active_producers = 10; // Se configuran 10 productores inicialmente

  // 1. Crear el hilo del spooler (encargado de la impresión)
  if ((r = pthread_create(&tid_spooler, NULL, spooler, NULL)) != 0) {
    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
    exit(1);
  }

  // 2. Crear los 10 hilos productores
  int thread_no[10];
  for (i = 0; i < 10; i++) {
    thread_no[i] = i;
    if ((r = pthread_create(&tid_producer[i], NULL, producer,
                            (void *)&thread_no[i])) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }
  }

  // 3. Esperar (join) a que todos los hilos productores terminen su ejecución
  for (i = 0; i < 10; i++) {
    if ((r = pthread_join(tid_producer[i], NULL)) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }
  }

  // 4. Esperar a que el spooler termine de imprimir todo y finalice elegantemente
  if ((r = pthread_join(tid_spooler, NULL)) != 0) {
    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
    exit(1);
  }

  return 0;
}

/*
 * Función ejecutada por cada uno de los 10 hilos productores.
 * Cada hilo genera un total de 10 líneas de texto, esperando 1 segundo entre cada una.
 */
void *producer(void *arg) {
  int i, r;
  int my_id = *((int *)arg); // Obtener el ID asignado a este hilo productor
  int count = 0;

  for (i = 0; i < 10; i++) {

    // Adquirir el mutex para proteger el acceso a las variables compartidas
    if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }
    
    // Si no hay búferes disponibles, esperar en la variable de condición buf_cond
    while (!buffers_available)
      pthread_cond_wait(&buf_cond, &buf_mutex);

    // Escribir en la ranura libre indicada por buffer_index de manera circular
    int j = buffer_index;
    buffer_index++;
    if (buffer_index == MAX_BUFFERS)
      buffer_index = 0;
    buffers_available--; // Decrementar el espacio disponible

    // Generar la cadena formateada que se guardará en el búfer
    sprintf(buf[j], "Thread %d: %d\n", my_id, ++count);
    lines_to_print++; // Incrementar líneas pendientes de impresión

    // Despertar al spooler que podría estar bloqueado esperando líneas para imprimir
    pthread_cond_signal(&spool_cond);

    // Liberar el mutex
    if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }

    // Simular trabajo (1 segundo) antes de producir la siguiente línea
    sleep(1);
  }

  // Decrementar la cuenta de productores activos al finalizar el ciclo de producción
  if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
    exit(1);
  }
  
  active_producers--;
  
  // Enviar señal final al spooler para que verifique si ya debe salir
  pthread_cond_signal(&spool_cond);
  
  if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
    exit(1);
  }

  return NULL;
}

/*
 * Función ejecutada por el hilo del spooler.
 * Se encarga de leer secuencialmente del búfer circular e imprimir las líneas en pantalla.
 */
void *spooler(void *arg) {
  (void)arg;
  int r;

  while (1) {
    // Adquirir el mutex
    if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }
    
    // Esperar si no hay líneas para imprimir y todavía hay productores activos trabajando
    while (!lines_to_print && active_producers > 0)
      pthread_cond_wait(&spool_cond, &buf_mutex);

    /*
     * Condición de salida limpia: Si no hay más líneas que imprimir y todos 
     * los productores activos terminaron su ejecución, el spooler finaliza.
     */
    if (!lines_to_print && active_producers == 0) {
      if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
        fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
        exit(1);
      }
      break;
    }

    // Imprimir el contenido de la ranura actual en la salida estándar
    printf("%s", buf[buffer_print_index]);
    lines_to_print--; // Decrementar el contador de líneas pendientes

    // Avanzar el índice de lectura de forma circular
    buffer_print_index++;
    if (buffer_print_index == MAX_BUFFERS)
      buffer_print_index = 0;

    // Liberar la ranura del búfer y despertar a los productores que esperan espacio
    buffers_available++;
    pthread_cond_signal(&buf_cond);

    // Liberar el mutex
    if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
      fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
      exit(1);
    }
  }
  return NULL;
}

