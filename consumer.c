/**************************************************
* Pontificia Universidad Javeriana
* @Author: Marlon Garcia
* Fecha: 19 de Mayo del 2026
* Objetivos:
*    -Programar un sistema de productor-consumidor
**************************************************/

#include "consumer.h"
#include <signal.h>

/*
 * Manejador de señales para interrupciones (SIGINT, SIGTERM).
 * Su objetivo es liberar de forma limpia todos los recursos de IPC de POSIX
 * (semáforos y memoria compartida) creados en el sistema operativo antes de salir.
 */
void limpiar_recursos_consumidor(int sig) {
    (void)sig; // Evita la advertencia de parámetro no usado
    printf("\n[Consumidor] Interrumpido. Limpiando recursos...\n");
    // Desenlazar (eliminar del sistema) los semáforos y la memoria compartida
    sem_unlink("/lleno");
    sem_unlink("/vacio");
    shm_unlink("/memoria_compartida");
    exit(EXIT_SUCCESS);
}

int main() {
    // Registrar los manejadores de señales para garantizar una limpieza segura
    signal(SIGINT, limpiar_recursos_consumidor);
    signal(SIGTERM, limpiar_recursos_consumidor);

    sem_t *vacio = SEM_FAILED;
    sem_t *lleno = SEM_FAILED;
    int retries = 0;
    
    printf("$Consumidor Intentando conectar con los semáforos...\n");
    
    /*
     * Bucle paciente: Espera a que el proceso productor cree e inicialice 
     * los semáforos de sincronización ("/vacio" y "/lleno"). Intenta 15 veces.
     */
    while (retries < 15) {
        vacio = sem_open("/vacio", 0);
        lleno = sem_open("/lleno", 0);
        if (vacio != SEM_FAILED && lleno != SEM_FAILED) {
            break; // Conexión exitosa, salir del bucle de reintentos
        }
        // Si solo se abrió uno, lo cerramos para mantener el estado limpio
        if (vacio != SEM_FAILED) { sem_close(vacio); vacio = SEM_FAILED; }
        if (lleno != SEM_FAILED) { sem_close(lleno); lleno = SEM_FAILED; }
        
        printf("$Consumidor Esperando a que el productor cree los semáforos (intento %d/15)...\n", retries + 1);
        sleep(1);
        retries++;
    }

    // Si después de 15 intentos no se logró conectar, se aborta el programa
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        fprintf(stderr, "$Consumidor Error: No se pudo conectar con los semáforos (¿el productor está corriendo?)\n");
        exit(EXIT_FAILURE);
    }

    int fd_compartido = -1;
    retries = 0;
    
    /*
     * Bucle paciente: Espera a que el proceso productor cree el objeto 
     * de memoria compartida "/memoria_compartida".
     */
    while (retries < 15) {
        fd_compartido = shm_open("/memoria_compartida", O_RDWR, 0644);
        if (fd_compartido >= 0) {
            break; // Conexión exitosa a la memoria compartida
        }
        sleep(1);
        retries++;
    }

    // Si falló la apertura de la memoria compartida, se imprime el error y se aborta
    if (fd_compartido < 0) {
        perror("$Consumidor$ shm_open");
        exit(EXIT_FAILURE);
    }

    /*
     * Mapea el objeto de memoria compartida en el espacio de direcciones de este proceso.
     * Permitimos lectura y escritura (PROT_READ | PROT_WRITE).
     */
    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, fd_compartido, 0);
    if (compartir == MAP_FAILED) {
        perror("$Consumidor mmap");
        exit(EXIT_FAILURE);
    }

    printf("[Consumidor] Conexión establecida. Esperando datos...\n");

    /*
     * Bucle principal de consumo:
     * Retira elementos del búfer circular siguiendo el algoritmo de productor-consumidor.
     */
    while (1) {
        // Esperar a que haya elementos disponibles en el buffer (bloquea si lleno == 0)
        sem_wait(lleno);

        /*
         * Condición de parada: Si el productor ha marcado que terminó de enviar datos
         * y además ya hemos procesado y consumido todos los datos pendientes en el búfer 
         * (es decir, la posición de entrada es igual a la de salida).
         */
        if (compartir->terminado && compartir->entrada == compartir->salida) {
            printf("[Consumidor] Productor finalizado y buffer vacío. Saliendo...\n");
            break;
        }

        // Leer el dato del búfer en la posición indicada por el índice 'salida'
        int item = compartir->bus[compartir->salida];
        printf("Consumidor: Consume %d\n", item);
        
        // Actualizar el índice 'salida' de forma circular usando el tamaño del BUFFER
        compartir->salida = (compartir->salida + 1) % BUFFER;

        // Notificar que se ha liberado una ranura en el búfer circular (incrementar vacio)
        sem_post(vacio);
        
        // Simular tiempo de procesamiento del dato consumido
        sleep(2);  
    }

    /*
     * Sección de limpieza:
     * Una vez terminado el consumo, desmapeamos la memoria, cerramos descriptores y semáforos,
     * y eliminamos los objetos IPC del sistema operativo.
     */
    munmap(compartir, sizeof(compartir_datos));
    close(fd_compartido);
    sem_close(lleno);
    sem_close(vacio);
    sem_unlink("/lleno");
    sem_unlink("/vacio");
    shm_unlink("/memoria_compartida");
    
    return 0;
}

