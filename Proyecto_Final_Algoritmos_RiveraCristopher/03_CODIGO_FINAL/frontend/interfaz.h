#ifndef INTERFAZ_H
#define INTERFAZ_H

#include "modelos.h"

void imprimirEncabezadoRutas();
void imprimirFilaRuta(const RutaBus &r);

void menuRutas(RutaBus rutas[], int &cantidad, bool &cambios);
void menuVentas(RutaBus rutas[], int cantidad, bool &cambios);
void menuBusquedas(const RutaBus rutas[], int cantidad);
void menuReportes(const RutaBus rutas[], int cantidad);
void menuBaseDatosYCifrado(RutaBus rutas[], int &cantidad);

#endif
