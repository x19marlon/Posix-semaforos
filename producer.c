/**************************************************
* Pontificia Universidad Javeriana
* @Author: Marlon Garcia
* Fecha: 19 de Mayo del 2026
* Objetivos:
*	-Programar un sistema de productor-consumidor 
utilizando memoria compartida y semáforos POSIX.
**************************************************/

#include "consumer.h"
#include <signal.h>
#include <string.h>

/*
 * Manejador de señales para interrupciones (SIGINT, SIGTERM).
 * Su objetivo es liberar de forma limpia todos los recursos de IPC de POSIX
 * (semáforos y memoria compartida) creados en el sistema operativo antes de salir.
 */
void limpiar_recursos(int sig) {
    (void)sig; // Evita la advertencia de parámetro no usado
    printf("\n[Productor] Interrumpido. Limpiando recursos...\n");
    // Desenlazar (eliminar del sistema) los semáforos y la memoria compartida
    sem_unlink("/vacio");
    sem_unlink("/lleno");
    shm_unlink("/memoria_compartida");
    exit(EXIT_SUCCESS);
}

int main() {
    // Registrar los manejadores de señales para garantizar una limpieza segura en caso de abortar
    signal(SIGINT, limpiar_recursos);
    signal(SIGTERM, limpiar_recursos);

    // 1. Limpiar recursos de ejecuciones anteriores para evitar corrupción de datos
    shm_unlink("/memoria_compartida");
    sem_unlink("/vacio");
    sem_unlink("/lleno");

    // 2. Crear y mapear memoria compartida primero
    // Abrir o crear el objeto de memoria compartida "/memoria_compartida" con permisos de lectura y escritura
    int shm_fd = shm_open("/memoria_compartida", O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("Productor: shm_open");
        exit(EXIT_FAILURE);
    }
    // Ajustar el tamaño del objeto de memoria compartida al tamaño de nuestra estructura compartir_datos
    ftruncate(shm_fd, sizeof(compartir_datos));

    // Mapear el objeto de memoria compartida en el espacio de direcciones de este proceso
    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (compartir == MAP_FAILED) {
        perror("Productor: mmap");
        exit(EXIT_FAILURE);
    }
    
    // 3. Inicializar a cero toda la estructura compartida (resetea terminado, entrada, salida, bus)
    // Esto asegura que la comunicación comience con un estado perfectamente predecible
    memset(compartir, 0, sizeof(compartir_datos));

    // 4. Crear semáforos SOLO después de que la memoria compartida esté lista y limpia.
    // Esto garantiza que el consumidor no empiece a leer datos viejos (estado persistente).
    // Semáforo 'vacio' se inicializa con el tamaño total del BUFFER (ranuras disponibles)
    // Semáforo 'lleno' se inicializa con 0 (ranuras con elementos listos)
    sem_t *vacio = sem_open("/vacio", O_CREAT, 0644, BUFFER);
    sem_t *lleno = sem_open("/lleno", O_CREAT, 0644, 0);
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("Productor: sem_open");
        exit(EXIT_FAILURE);
    }

    printf("[Productor] Iniciando producción...\n");

    // Producir un flujo de 10 elementos numerados del 1 al 10
    for (int i = 1; i <= 10; i++) {
        // Esperar a que haya espacio disponible en el buffer (vacio > 0)
        sem_wait(vacio);
        
        // Escribir el elemento producido en la posición 'entrada' del búfer circular
        compartir->bus[compartir->entrada] = i;
        printf("Productor: Produce %d\n", i);
        
        // Actualizar el índice 'entrada' circularmente
        compartir->entrada = (compartir->entrada + 1) % BUFFER;
        
        // Notificar que hay un nuevo elemento disponible (incrementar lleno)
        sem_post(lleno);
        
        // Simular tiempo necesario para producir el siguiente elemento
        sleep(1);  
    }

    // Indicar al consumidor que ya no se enviarán más datos (fin de producción)
    compartir->terminado = 1;
    
    // Despertar al consumidor en caso de que esté bloqueado esperando datos en sem_wait(lleno)
    sem_post(lleno);

    printf("[Productor] Producción finalizada. Liberando memoria local...\n");
    
    // Liberar el mapeo de memoria y cerrar descriptores de archivos/semáforos del proceso local
    munmap(compartir, sizeof(compartir_datos));
    close(shm_fd);
    sem_close(vacio);
    sem_close(lleno);
    
    return 0;
}

