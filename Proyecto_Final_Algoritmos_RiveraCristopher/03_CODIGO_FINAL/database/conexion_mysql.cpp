#include "conexion_mysql.h"
#include "cifrado_tontos.h"
#include "conexion_db.h"
#include "validaciones.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

using namespace std;

static string g_ultimaSQL = "SELECT * FROM rutas_buses ORDER BY id ASC;";
static bool g_mysqlConectado = false;

string obtenerUltimaSentenciaSQL() {
    return g_ultimaSQL;
}

bool estadoServidorMySQLActivo() {
    return g_mysqlConectado;
}

CredencialesMySQL obtenerCredencialesMySQL() {
    CredencialesMySQL cred;
    ifstream in(ARCHIVO_CONFIG_MYSQL);
    if (in.is_open()) {
        string linea;
        if (getline(in, linea)) cred.host = recortar(linea);
        if (getline(in, linea)) {
            int p = atoi(linea.c_str());
            if (p > 0) cred.puerto = (unsigned int)p;
        }
        if (getline(in, linea)) cred.usuario = recortar(linea);
        if (getline(in, linea)) cred.password = recortar(linea);
        if (getline(in, linea)) cred.baseDatos = recortar(linea);
        in.close();
    }
    if (cred.host.empty()) cred.host = "localhost";
    if (cred.usuario.empty()) cred.usuario = "root";
    if (cred.baseDatos.empty()) cred.baseDatos = "db_transporte_rutas";
    return cred;
}

void guardarCredencialesMySQL(const CredencialesMySQL &cred) {
    ofstream out(ARCHIVO_CONFIG_MYSQL);
    if (out.is_open()) {
        out << cred.host << "\n"
            << cred.puerto << "\n"
            << cred.usuario << "\n"
            << cred.password << "\n"
            << cred.baseDatos << "\n";
        out.close();
    }
}

static string formatearLineaPlana(const RutaBus &r) {
    ostringstream ss;
    ss << fixed << setprecision(2)
       << r.id << ";" << r.origen << ";" << r.destino << ";"
       << r.capacidad << ";" << r.pasajeros << ";"
       << r.precio << ";" << r.ingresos << ";" << (r.activa ? 1 : 0);
    return ss.str();
}

static string localizarBinarioMySQL() {
#ifdef _WIN32
    const char *rutasWin[] = {
        "C:\\Program Files\\MySQL\\MySQL Server 8.0\\bin\\mysql.exe",
        "C:\\Program Files\\MySQL\\MySQL Server 8.4\\bin\\mysql.exe",
        "C:\\xampp\\mysql\\bin\\mysql.exe",
        "C:\\wamp64\\bin\\mysql\\mysql8.0.31\\bin\\mysql.exe"
    };
    for (int i = 0; i < 4; i++) {
        ifstream f(rutasWin[i]);
        if (f.good()) {
            return string("\"") + rutasWin[i] + "\"";
        }
    }
    return "mysql";
#else
    ifstream f("/usr/bin/mysql");
    if (f.good()) return "/usr/bin/mysql";
    return "";
#endif
}

bool ejecutarSQLenMySQL(const string &sentenciaSQL, bool usarBaseDatos) {
    g_ultimaSQL = sentenciaSQL;
    CredencialesMySQL cred = obtenerCredencialesMySQL();

#ifdef USAR_LIBMYSQL
    MYSQL *con = mysql_init(NULL);
    if (!con) return false;
    const char *dbPtr = usarBaseDatos ? cred.baseDatos.c_str() : NULL;
    if (!mysql_real_connect(con, cred.host.c_str(), cred.usuario.c_str(),
                            cred.password.c_str(), dbPtr, cred.puerto, NULL, 0)) {
        mysql_close(con);
        g_mysqlConectado = false;
        return false;
    }
    int rc = mysql_query(con, sentenciaSQL.c_str());
    mysql_close(con);
    g_mysqlConectado = (rc == 0);
    return g_mysqlConectado;
#else
    string bin = localizarBinarioMySQL();
    if (bin.empty()) {
        g_mysqlConectado = false;
        return false;
    }

    string tmpSql = "tmp_query_mysql.sql";
    ofstream out(tmpSql);
    if (!out.is_open()) return false;
    if (usarBaseDatos) {
        out << "CREATE DATABASE IF NOT EXISTS " << cred.baseDatos << ";\n";
        out << "USE " << cred.baseDatos << ";\n";
    }
    out << sentenciaSQL << "\n";
    out.close();

    ostringstream cmd;
    cmd << bin << " -h " << cred.host << " -P " << cred.puerto << " -u " << cred.usuario;
    if (!cred.password.empty()) {
        cmd << " -p\"" << cred.password << "\"";
    }
#ifdef _WIN32
    cmd << " < " << tmpSql << " >nul 2>nul";
#else
    cmd << " < " << tmpSql << " >/dev/null 2>&1";
#endif
    int rc = system(cmd.str().c_str());
    remove(tmpSql.c_str());
    g_mysqlConectado = (rc == 0);
    return g_mysqlConectado;
#endif
}

bool inicializarBaseDeDatosMySQL() {
    string ddl =
        "CREATE TABLE IF NOT EXISTS rutas_buses ("
        "id INT PRIMARY KEY, "
        "origen VARCHAR(30) NOT NULL, "
        "destino VARCHAR(30) NOT NULL, "
        "capacidad INT NOT NULL, "
        "pasajeros INT NOT NULL DEFAULT 0, "
        "precio DECIMAL(10,2) NOT NULL, "
        "ingresos DECIMAL(12,2) NOT NULL DEFAULT 0.00, "
        "activa TINYINT(1) NOT NULL DEFAULT 1, "
        "linea_cifrado_tontos VARCHAR(150) NOT NULL, "
        "fecha_actualizacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
        ");";
    return ejecutarSQLenMySQL(ddl, true);
}

bool insertarRutaEnMySQL(const RutaBus &r) {
    inicializarBaseDeDatosMySQL();
    string cifrada = cifrarParaTontos(formatearLineaPlana(r));
    ostringstream sql;
    sql << fixed << setprecision(2);
    sql << "REPLACE INTO rutas_buses (id, origen, destino, capacidad, pasajeros, precio, ingresos, activa, linea_cifrado_tontos) VALUES ("
        << r.id << ", '" << r.origen << "', '" << r.destino << "', "
        << r.capacidad << ", " << r.pasajeros << ", "
        << r.precio << ", " << r.ingresos << ", "
        << (r.activa ? 1 : 0) << ", '" << cifrada << "');";
    return ejecutarSQLenMySQL(sql.str(), true);
}

bool actualizarRutaEnMySQL(const RutaBus &r) {
    string cifrada = cifrarParaTontos(formatearLineaPlana(r));
    ostringstream sql;
    sql << fixed << setprecision(2);
    sql << "UPDATE rutas_buses SET "
        << "origen='" << r.origen << "', "
        << "destino='" << r.destino << "', "
        << "capacidad=" << r.capacidad << ", "
        << "pasajeros=" << r.pasajeros << ", "
        << "precio=" << r.precio << ", "
        << "ingresos=" << r.ingresos << ", "
        << "activa=" << (r.activa ? 1 : 0) << ", "
        << "linea_cifrado_tontos='" << cifrada << "' "
        << "WHERE id=" << r.id << ";";
    return ejecutarSQLenMySQL(sql.str(), true);
}

bool eliminarRutaEnMySQL(int idRuta) {
    ostringstream sql;
    sql << "DELETE FROM rutas_buses WHERE id=" << idRuta << ";";
    return ejecutarSQLenMySQL(sql.str(), true);
}

bool cargarRutasDesdeMySQL(RutaBus rutas[], int &cantidad) {
#ifdef USAR_LIBMYSQL
    CredencialesMySQL cred = obtenerCredencialesMySQL();
    MYSQL *con = mysql_init(NULL);
    if (!con) return false;
    if (!mysql_real_connect(con, cred.host.c_str(), cred.usuario.c_str(),
                            cred.password.c_str(), cred.baseDatos.c_str(),
                            cred.puerto, NULL, 0)) {
        mysql_close(con);
        return false;
    }
    if (mysql_query(con, "SELECT id, origen, destino, capacidad, pasajeros, precio, ingresos, activa FROM rutas_buses ORDER BY id ASC") != 0) {
        mysql_close(con);
        return false;
    }
    MYSQL_RES *res = mysql_store_result(con);
    if (!res) {
        mysql_close(con);
        return false;
    }
    cantidad = 0;
    MYSQL_ROW fila;
    while ((fila = mysql_fetch_row(res)) != NULL && cantidad < MAX_RUTAS) {
        RutaBus r;
        r.id = atoi(fila[0]);
        r.origen = fila[1] ? fila[1] : "";
        r.destino = fila[2] ? fila[2] : "";
        r.capacidad = atoi(fila[3]);
        r.pasajeros = atoi(fila[4]);
        r.precio = atof(fila[5]);
        r.ingresos = r.pasajeros * r.precio;
        r.activa = (atoi(fila[7]) == 1);
        rutas[cantidad++] = r;
    }
    mysql_free_result(res);
    mysql_close(con);
    g_mysqlConectado = true;
    return true;
#else
    (void)rutas;
    (void)cantidad;
    return false;
#endif
}

bool guardarRutasEnMySQL(const RutaBus rutas[], int cantidad) {
    exportarSentenciasMySQL(rutas, cantidad);
    inicializarBaseDeDatosMySQL();
    ostringstream batch;
    batch << "DELETE FROM rutas_buses;\n";
    batch << fixed << setprecision(2);
    for (int i = 0; i < cantidad; i++) {
        string cifrada = cifrarParaTontos(formatearLineaPlana(rutas[i]));
        batch << "INSERT INTO rutas_buses (id, origen, destino, capacidad, pasajeros, precio, ingresos, activa, linea_cifrado_tontos) VALUES ("
              << rutas[i].id << ", '" << rutas[i].origen << "', '" << rutas[i].destino << "', "
              << rutas[i].capacidad << ", " << rutas[i].pasajeros << ", "
              << rutas[i].precio << ", " << rutas[i].ingresos << ", "
              << (rutas[i].activa ? 1 : 0) << ", '" << cifrada << "');\n";
    }
    return ejecutarSQLenMySQL(batch.str(), true);
}

void mostrarGuiaConexionMySQLenConsola() {
    CredencialesMySQL cred = obtenerCredencialesMySQL();
    cout << "\n--- CONFIGURACION Y CONEXION A MYSQL ---\n";
    cout << "Servidor MySQL : " << cred.host << ":" << cred.puerto << "\n";
    cout << "Base de Datos  : " << cred.baseDatos << "\n";
    cout << "Usuario        : " << cred.usuario << " | Password: " << (cred.password.empty() ? "(vacia)" : "********") << "\n";
    cout << "Ultima SQL     : " << g_ultimaSQL << "\n";
}
