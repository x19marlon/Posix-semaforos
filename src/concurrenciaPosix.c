/*********************************************************************************************
 * Pontificia Universidad Javeriana
 *
 * Materia Sistemas Operativos
 * @Author: Marlon Garcia
 * Fecha: 19 de Mayo del 2026
 * Tema: Posix para la creación de hilos concurrentes:
 *
 * Descripción:
 * Programa que busca el valor máximo en un arreglo de enteros utilizando
 * múltiples hilos (pthreads) para procesar porciones del arreglo en paralelo.
 * Recibe un archivo con los números y la cantidad de hilos a utilizar.
 *********************************************************************************************/

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Estructura para pasar múltiples argumentos a los hilos.
 * Define el rango del vector que un hilo particular debe analizar.
 */
struct argHilos {
  int inicio;     // Índice inicial en el arreglo
  int fin;        // Índice final (no inclusivo) en el arreglo
  int *vector;    // Puntero al arreglo completo
  int maxparcial; // Almacena el resultado máximo de este hilo
};

typedef struct argHilos param_H;

/*
 * Función ejecutada por cada hilo.
 * Busca el valor máximo dentro de la porción del vector asignada al hilo.
 */
void *buscarMax(void *parametro) {
  param_H *argumentos = (param_H *)parametro;

  // Inicializar el máximo parcial con el primer elemento de su porción
  argumentos->maxparcial = argumentos->vector[argumentos->inicio];

  // Recorrer sólo la porción asignada (desde 'inicio' hasta 'fin')
  for (int i = argumentos->inicio; i < argumentos->fin; i++) {
    if (argumentos->vector[i] > argumentos->maxparcial)
      argumentos->maxparcial = argumentos->vector[i];
  }

  pthread_exit(0);
  return NULL;
}

/*
 * Función para dividir el trabajo entre múltiples hilos, lanzar su ejecución,
 * esperar sus resultados y calcular el máximo global.
 */
int maximoValor(int *vec, int n, int nhilos) {
  pthread_t hilos[nhilos];
  param_H args[nhilos];

  // Calcular cuántos elementos le tocan a cada hilo y el resto
  int elementos_por_hilo = n / nhilos;
  int resto = n % nhilos;

  int inicio = 0;

  // 1. Crear y lanzar los hilos
  for (int i = 0; i < nhilos; i++) {
    args[i].inicio = inicio;
    // Si la división no es exacta, repartir el resto entre los primeros hilos
    args[i].fin = inicio + elementos_por_hilo + (i < resto ? 1 : 0);
    args[i].vector = vec;
    args[i].maxparcial = INT_MIN; // Iniciar con el mínimo posible

    // Si a un hilo no le tocan elementos, saltar
    if (args[i].inicio < args[i].fin) {
      pthread_create(&hilos[i], NULL, buscarMax, (void *)&args[i]);
    }
    inicio = args[i].fin;
  }

  int max_global = INT_MIN;

  // 2. Esperar (join) a que terminen los hilos y agregar sus resultados
  // parciales
  for (int i = 0; i < nhilos; i++) {
    // Sólo esperar y chequear si se le asignaron elementos
    if (args[i].inicio < args[i].fin) {
      pthread_join(hilos[i], NULL);
      if (args[i].maxparcial > max_global) {
        max_global = args[i].maxparcial;
      }
    }
  }

  return max_global;
}

int main(int argc, char *argv[]) {
  FILE *fichero;
  int n, nhilos, i;
  int *vec;
  int ret, maximo;

  // Verificar los argumentos de entrada: <ejecutable> <archivo_datos>
  // <num_hilos>
  if (argc != 3) {
    fprintf(stderr, "Error en número de argumentos \n");
    fprintf(stderr, "Uso: %s <archivo_datos> <num_hilos>\n", argv[0]);
    exit(-1);
  }

  // Abrir el archivo en modo lectura
  fichero = fopen(argv[1], "r");
  if (fichero == NULL) {
    perror("No se puede abrir fichero");
    exit(-2);
  }

  // Leer el tamaño del vector (primer número en el archivo)
  ret = fscanf(fichero, "%d", &n);
  if (ret != 1) {
    fprintf(stderr, "No se puede leer tamaño\n");
    exit(-3);
  }

  // Leer la cantidad de hilos de la línea de comandos
  nhilos = atoi(argv[2]);
  if (nhilos <= 0)
    nhilos = 1; // Evitar división por cero

  // Reservar memoria para el arreglo de números
  vec = malloc(sizeof(int) * n);

  // Leer los elementos del archivo
  for (i = 0; i != n; ++i) {
    ret = fscanf(fichero, "%d", &vec[i]);
    if (ret != 1) {
      fprintf(stderr, "No se puede leer elemento nro %d\n", i);
      fclose(fichero);
      free(vec);
      exit(-4);
    }
  }

  // Invocar a la función principal que distribuye el trabajo concurrente
  maximo = maximoValor(vec, n, nhilos);
  printf("Máximo: %d\n", maximo);

  // Limpieza de memoria y cierre de archivo
  fclose(fichero);
  free(vec);
  return 0;
}
