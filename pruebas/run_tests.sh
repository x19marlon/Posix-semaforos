#!/bin/bash

echo "=== Ejecutando pruebas para concurrenciaPosix ==="

echo -e "\n[Prueba 1] Arreglo mediano (10,000 elementos) con 2 hilos:"
./concurrenciaPosix pruebas/test1_pequeno.txt 2

echo -e "\n[Prueba 2] Arreglo grande (100,000 elementos) con 4 hilos (Máximo esperado: 999999):"
./concurrenciaPosix pruebas/test2_grande.txt 4

echo -e "\n[Prueba 3] Arreglo muy grande y negativos (1,000,000 elementos) con 8 hilos (Máximo esperado: -5):"
./concurrenciaPosix pruebas/test3_negativos.txt 8

echo -e "\n=== Pruebas finalizadas ==="
