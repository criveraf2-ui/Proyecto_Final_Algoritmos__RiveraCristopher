#include "conexion_db.h"
#include "cifrado_tontos.h"
#include "validaciones.h"
#include "algoritmos.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// Verifica si los archivos de la base de datos local estan accesibles
bool verificarConexionDB() {
    ifstream pruebaLectura(ARCHIVO_DATOS);
    if (pruebaLectura.is_open()) {
        pruebaLectura.close();
        return true;
    }
    return false;
}

// Guarda las rutas en el archivo principal y una copia protegida con Cifrado para Tontos
bool guardarDatos(const RutaBus rutas[], int cantidad) {
    ofstream archivo(ARCHIVO_DATOS);
    if (!archivo.is_open()) {
        cout << "\n[!] No se pudo abrir " << ARCHIVO_DATOS << " para guardar.\n";
        return false;
    }

    ofstream archivoCifrado(ARCHIVO_CIFRADO);

    for (int i = 0; i < cantidad; i++) {
        ostringstream lineaStream;
        lineaStream << fixed << setprecision(2);
        lineaStream << rutas[i].id << ';'
                    << rutas[i].origen << ';'
                    << rutas[i].destino << ';'
                    << rutas[i].capacidad << ';'
                    << rutas[i].pasajeros << ';'
                    << rutas[i].precio << ';'
                    << rutas[i].ingresos << ';'
                    << (rutas[i].activa ? 1 : 0);

        string lineaPlana = lineaStream.str();
        archivo << lineaPlana << "\n";

        if (archivoCifrado.is_open()) {
            archivoCifrado << cifrarParaTontos(lineaPlana) << "\n";
        }
    }

    archivo.close();
    if (archivoCifrado.is_open()) {
        archivoCifrado.close();
    }
    exportarSentenciasMySQL(rutas, cantidad);
    return true;
}

// Carga los registros desde la base de datos de texto, aceptando lineas normales o con Cifrado para Tontos
void cargarDatos(RutaBus rutas[], int &cantidad) {
    cantidad = 0;
    ifstream archivo(ARCHIVO_DATOS);
    if (!archivo.is_open()) {
        cout << "[*] No hay archivo previo. Se inicia con datos vacios.\n";
        return;
    }

    string linea;
    int numeroLinea = 0;
    int omitidas = 0;

    while (getline(archivo, linea)) {
        numeroLinea++;
        linea = recortar(linea);
        if (linea.empty()) {
            continue;
        }
        if (cantidad >= MAX_RUTAS) {
            cout << "[!] Se alcanzo el limite de " << MAX_RUTAS << " rutas; se ignora el resto.\n";
            break;
        }

        // Si la linea viene con Cifrado para Tontos, se descifra antes de separar los campos
        if (lineaEstaCifrada(linea)) {
            linea = descifrarParaTontos(linea);
        }

        istringstream lectura(linea);
        string campos[8];
        int n = 0;
        string campo;

        while (n < 8 && getline(lectura, campo, ';')) {
            campos[n] = campo;
            n++;
        }

        bool valida = (n == 8);
        RutaBus r;

        if (valida) {
            istringstream cId(campos[0]);
            istringstream cCap(campos[3]);
            istringstream cPas(campos[4]);
            istringstream cPre(campos[5]);
            istringstream cIng(campos[6]);
            istringstream cAct(campos[7]);

            int activaNum = -1;
            bool conversionOk = (bool)(cId >> r.id) &&
                                (bool)(cCap >> r.capacidad) &&
                                (bool)(cPas >> r.pasajeros) &&
                                (bool)(cPre >> r.precio) &&
                                (bool)(cIng >> r.ingresos) &&
                                (bool)(cAct >> activaNum);

            r.origen = recortar(campos[1]);
            r.destino = recortar(campos[2]);
            r.activa = (activaNum == 1);

            valida = conversionOk &&
                     (activaNum == 0 || activaNum == 1) &&
                     (r.id >= 1 && r.id <= ID_MAX) &&
                     (r.capacidad >= 1 && r.capacidad <= CAPACIDAD_MAX) &&
                     (r.pasajeros >= 0 && r.pasajeros <= r.capacidad) &&
                     (r.precio > 0.0 && r.precio <= PRECIO_MAX) &&
                     (!r.origen.empty() && !r.destino.empty()) &&
                     (!mismoTexto(r.origen, r.destino)) &&
                     (buscarPorID(rutas, cantidad, r.id) == -1);
        }

        if (valida) {
            r.ingresos = r.pasajeros * r.precio;
            rutas[cantidad] = r;
            cantidad++;
        } else {
            omitidas++;
            cout << "[!] Linea " << numeroLinea << " invalida u omitida en " << ARCHIVO_DATOS << ".\n";
        }
    }

    archivo.close();
    cout << "[*] Se cargaron " << cantidad << " rutas";
    if (omitidas > 0) {
        cout << " (" << omitidas << " omitidas)";
    }
    cout << ".\n";
}

// Muestra como se ven los registros en la BD con el Cifrado para Tontos
void mostrarArchivoCifradoDB(const RutaBus rutas[], int cantidad) {
    cout << "\n--- VISTA DE BASE DE DATOS (CIFRADO PARA TONTOS) ---\n";
    cout << "Estado de conexion DB (" << ARCHIVO_DATOS << "): "
         << (verificarConexionDB() ? "CONECTADO" : "EN MEMORIA (pendiente guardar)") << "\n";
    ConfigConexionMySQL cfg;
    cout << "Motor MySQL configurado : " << cfg.usuario << "@" << cfg.host << ":" << cfg.puerto << "/" << cfg.baseDatos << "\n";
    cout << "Script SQL sincronizado : " << cfg.archivoSincronizacionSQL << "\n";
    cout << "Archivo respaldo cifrado: " << ARCHIVO_CIFRADO << " (Clave +" << CLAVE_TONTOS << ")\n\n";

    if (cantidad == 0) {
        cout << "[!] No hay rutas registradas en memoria para mostrar.\n";
        return;
    }

    for (int i = 0; i < cantidad; i++) {
        ostringstream lineaStream;
        lineaStream << fixed << setprecision(2);
        lineaStream << rutas[i].id << ';'
                    << rutas[i].origen << ';'
                    << rutas[i].destino << ';'
                    << rutas[i].capacidad << ';'
                    << rutas[i].pasajeros << ';'
                    << rutas[i].precio << ';'
                    << rutas[i].ingresos << ';'
                    << (rutas[i].activa ? 1 : 0);

        string plana = lineaStream.str();
        string cifrada = cifrarParaTontos(plana);
        cout << "Registro [" << (i + 1) << "] Normal  : " << plana << "\n";
        cout << "Registro [" << (i + 1) << "] Cifrado : " << cifrada << "\n\n";
    }
}


// Exporta en tiempo real las sentencias INSERT/REPLACE para sincronizar con MySQL
void exportarSentenciasMySQL(const RutaBus rutas[], int cantidad) {
    ConfigConexionMySQL config;
    ofstream sqlOut(config.archivoSincronizacionSQL);
    if (!sqlOut.is_open()) {
        return;
    }

    sqlOut << "-- Sincronizacion automatica generada desde C++17 para MySQL\n";
    sqlOut << "-- Servidor: " << config.host << ":" << config.puerto
           << " | Base de Datos: " << config.baseDatos << "\n";
    sqlOut << "USE " << config.baseDatos << ";\n\n";
    sqlOut << fixed << setprecision(2);

    for (int i = 0; i < cantidad; i++) {
        ostringstream planaStream;
        planaStream << fixed << setprecision(2)
                    << rutas[i].id << ";" << rutas[i].origen << ";" << rutas[i].destino << ";"
                    << rutas[i].capacidad << ";" << rutas[i].pasajeros << ";"
                    << rutas[i].precio << ";" << rutas[i].ingresos << ";"
                    << (rutas[i].activa ? 1 : 0);
        string cifrada = cifrarParaTontos(planaStream.str());

        sqlOut << "REPLACE INTO rutas_buses "
               << "(id, origen, destino, capacidad, pasajeros, precio, ingresos, activa, linea_cifrado_tontos) VALUES ("
               << rutas[i].id << ", '" << rutas[i].origen << "', '" << rutas[i].destino << "', "
               << rutas[i].capacidad << ", " << rutas[i].pasajeros << ", "
               << rutas[i].precio << ", " << rutas[i].ingresos << ", "
               << (rutas[i].activa ? 1 : 0) << ", '" << cifrada << "');\n";
    }
    sqlOut.close();
}
