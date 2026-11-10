#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include "modelos.h"

// Busquedas (secuencial por ID, por origen y por destino)
int buscarPorID(const RutaBus rutas[], int cantidad, int idBuscado);
void buscarPorOrigen(const RutaBus rutas[], int cantidad);
void buscarPorDestino(const RutaBus rutas[], int cantidad);

// Ordenamiento manual por metodo de la Burbuja (sin std::sort)
void ordenarPorIngresos(const RutaBus rutas[], int cantidad, int orden[]);

// Reportes y estadisticas del sistema
void reporteIngresos(const RutaBus rutas[], int cantidad);
void reporteOcupacion(const RutaBus rutas[], int cantidad);
void reporteResumen(const RutaBus rutas[], int cantidad);

#endif
