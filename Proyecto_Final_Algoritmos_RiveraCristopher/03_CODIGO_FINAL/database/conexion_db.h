#ifndef CONEXION_DB_H
#define CONEXION_DB_H

#include "modelos.h"
#include <string>
using namespace std;

// Parametros de conexion para el servidor MySQL (db_transporte_rutas)
struct ConfigConexionMySQL {
    string host = "localhost";
    int puerto = 3306;
    string usuario = "root";
    string password = "";
    string baseDatos = "db_transporte_rutas";
    string archivoSincronizacionSQL = "sincronizacion_mysql.sql";
};

// Capa de conexion a Base de Datos (fstream + sincronizacion de sentencias MySQL)
bool verificarConexionDB();
bool guardarDatos(const RutaBus rutas[], int cantidad);
void cargarDatos(RutaBus rutas[], int &cantidad);
void exportarSentenciasMySQL(const RutaBus rutas[], int cantidad);
void mostrarArchivoCifradoDB(const RutaBus rutas[], int cantidad);

#endif
