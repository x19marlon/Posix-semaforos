##################################################
# Pontificia Universidad Javeriana
# @Author: Marlon Garcia
# Fecha: 05 de Mayo del 2026
# Objetivos:
#	-Argumento de Entrada
#	-Fichero de Automatización de Compilación
#	-Programación Modular
##################################################

# Compilador y banderas/banderas de enlazado comunes
CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread -lrt

# Nombre de los ejecutables objetivo
TARGETS = posixSincro producer consumer concurrenciaPosix

# Regla por defecto para compilar todos los ejecutables
all: $(TARGETS)

# Regla de compilación para el ejercicio de sincronización POSIX con hilos
posixSincro: src/posixSincro.c
	$(CC) $(CFLAGS) $< -o $@

# Regla de compilación para el productor del búfer compartido
producer: src/producer.c src/consumer.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Regla de compilación para el consumidor del búfer compartido
consumer: src/consumer.c src/consumer.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Regla de compilación para el buscador de máximos con hilos
concurrenciaPosix: src/concurrenciaPosix.c
	$(CC) $(CFLAGS) $< -o $@

# Regla de limpieza para borrar los binarios generados
clean:
	rm -f $(TARGETS)

.PHONY: all clean

