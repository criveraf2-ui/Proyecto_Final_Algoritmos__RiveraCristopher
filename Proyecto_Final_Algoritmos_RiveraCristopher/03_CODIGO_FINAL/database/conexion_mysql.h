#ifndef CONEXION_MYSQL_H
#define CONEXION_MYSQL_H

#include "modelos.h"
#include <string>
using namespace std;

#ifdef USAR_LIBMYSQL
#include <mysql.h>
#endif

struct CredencialesMySQL {
    string host = "localhost";
    unsigned int puerto = 3306;
    string usuario = "root";
    string password = "";
    string baseDatos = "db_transporte_rutas";
};

// Administracion de credenciales y conexion MySQL (compatible con <mysql.h> y cliente mysql.exe)
CredencialesMySQL obtenerCredencialesMySQL();
void guardarCredencialesMySQL(const CredencialesMySQL &cred);
bool ejecutarSQLenMySQL(const string &sentenciaSQL, bool crearDBSiNoExiste = true);
bool inicializarBaseDeDatosMySQL();

// Operaciones CRUD directas hacia la tabla rutas_buses en MySQL
bool insertarRutaEnMySQL(const RutaBus &r);
bool actualizarRutaEnMySQL(const RutaBus &r);
bool eliminarRutaEnMySQL(int idRuta);
bool cargarRutasDesdeMySQL(RutaBus rutas[], int &cantidad);
bool guardarRutasEnMySQL(const RutaBus rutas[], int cantidad);
string obtenerUltimaSentenciaSQL();
bool estadoServidorMySQLActivo();
void mostrarGuiaConexionMySQLenConsola();

#endif
