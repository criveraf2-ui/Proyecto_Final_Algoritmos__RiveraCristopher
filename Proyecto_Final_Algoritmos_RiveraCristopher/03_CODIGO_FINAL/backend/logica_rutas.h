#ifndef LOGICA_RUTAS_H
#define LOGICA_RUTAS_H

#include "modelos.h"
#include <string>
using namespace std;

// Operaciones CRUD en consola y directas (para el Frontend HTML y C++)
// CREATE (Crear)
bool crearRutaDirecta(RutaBus rutas[], int &cantidad, const RutaBus &nueva, string &mensajeError);
bool registrarRuta(RutaBus rutas[], int &cantidad);

// READ (Leer)
void listarRutas(const RutaBus rutas[], int cantidad);

// UPDATE (Actualizar / Modificar datos de una ruta o su estado)
bool modificarRutaDirecta(RutaBus rutas[], int cantidad, const RutaBus &editada, string &mensajeError);
bool actualizarRuta(RutaBus rutas[], int cantidad);
bool cambiarEstadoRuta(RutaBus rutas[], int cantidad);

// DELETE (Eliminar ruta del arreglo y de la BD)
bool borrarRutaPorID(RutaBus rutas[], int &cantidad, int idBorrar, string &mensajeError);
bool eliminarRuta(RutaBus rutas[], int &cantidad);

// Ventas, cancelaciones y reto algoritmico de recomendacion
void aplicarVenta(RutaBus &ruta, int boletos);
bool venderBoletos(RutaBus rutas[], int cantidad);
bool cancelarBoletos(RutaBus rutas[], int cantidad);
bool recomendarYVender(RutaBus rutas[], int cantidad);

#endif
