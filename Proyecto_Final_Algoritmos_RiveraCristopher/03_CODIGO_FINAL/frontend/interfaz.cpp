#include "interfaz.h"
#include "validaciones.h"
#include "logica_rutas.h"
#include "algoritmos.h"
#include "conexion_db.h"
#include "conexion_mysql.h"
#include "cifrado_tontos.h"
#include <iostream>
#include <iomanip>

using namespace std;

void imprimirEncabezadoRutas() {
    cout << left << setw(6) << "ID" << setw(16) << "Origen" << setw(16) << "Destino"
         << right << setw(6) << "Cap." << setw(7) << "Pasaj." << setw(7) << "Disp."
         << setw(10) << "Precio" << setw(12) << "Ingresos" << "  " << left << "Estado\\n";
    cout << string(88, '-') << "\\n";
}

void imprimirFilaRuta(const RutaBus &r) {
    string textoEstado = "Inactiva";
    if (r.activa) {
        textoEstado = "Activa";
    }
    cout << left << setw(6) << r.id << setw(16) << r.origen << setw(16) << r.destino
         << right << setw(6) << r.capacidad << setw(7) << r.pasajeros
         << setw(7) << (r.capacidad - r.pasajeros)
         << fixed << setprecision(2) << setw(10) << r.precio << setw(12) << r.ingresos
         << "  " << left << textoEstado << "\\n";
}

// Submenu CRUD Completo de Rutas (Create, Read, Update, Delete)
void menuRutas(RutaBus rutas[], int &cantidad, bool &cambios) {
    int opcion;
    do {
        cout << "\\n--- GESTION CRUD DE RUTAS ---\\n";
        cout << "1. Registrar nueva ruta (CREATE)\\n";
        cout << "2. Listar todas las rutas (READ)\\n";
        cout << "3. Activar / desactivar ruta (UPDATE estado)\\n";
        cout << "4. Modificar datos de una ruta (UPDATE completo)\\n";
        cout << "5. Eliminar una ruta (DELETE)\\n";
        cout << "0. Volver\\n";
        opcion = leerEntero("Opcion: ", 0, 5);

        switch (opcion) {
            case 1:
                if (registrarRuta(rutas, cantidad)) cambios = true;
                break;
            case 2:
                listarRutas(rutas, cantidad);
                break;
            case 3:
                if (cambiarEstadoRuta(rutas, cantidad)) cambios = true;
                break;
            case 4:
                if (actualizarRuta(rutas, cantidad)) cambios = true;
                break;
            case 5:
                if (eliminarRuta(rutas, cantidad)) cambios = true;
                break;
        }
    } while (opcion != 0);
}

void menuVentas(RutaBus rutas[], int cantidad, bool &cambios) {
    int opcion;
    do {
        cout << "\\n--- VENTAS Y ASIGNACION ---\\n";
        cout << "1. Vender boletos por ID de ruta\\n";
        cout << "2. Recomendar ruta (origen/destino) y vender\\n";
        cout << "3. Cancelar boletos\\n";
        cout << "0. Volver\\n";
        opcion = leerEntero("Opcion: ", 0, 3);

        switch (opcion) {
            case 1:
                if (venderBoletos(rutas, cantidad)) cambios = true;
                break;
            case 2:
                if (recomendarYVender(rutas, cantidad)) cambios = true;
                break;
            case 3:
                if (cancelarBoletos(rutas, cantidad)) cambios = true;
                break;
        }
    } while (opcion != 0);
}

void menuBusquedas(const RutaBus rutas[], int cantidad) {
    int opcion;
    do {
        cout << "\\n--- BUSQUEDAS ---\\n";
        cout << "1. Buscar por ID\\n";
        cout << "2. Buscar por origen\\n";
        cout << "3. Buscar por destino\\n";
        cout << "0. Volver\\n";
        opcion = leerEntero("Opcion: ", 0, 3);

        if (opcion == 1) {
            int id = leerEntero("ID de la ruta: ", 1, ID_MAX);
            int pos = buscarPorID(rutas, cantidad, id);
            if (pos == -1) {
                cout << "[!] Ruta no encontrada.\\n";
            } else {
                cout << "\\n";
                imprimirEncabezadoRutas();
                imprimirFilaRuta(rutas[pos]);
            }
        } else if (opcion == 2) {
            buscarPorOrigen(rutas, cantidad);
        } else if (opcion == 3) {
            buscarPorDestino(rutas, cantidad);
        }
    } while (opcion != 0);
}

void menuReportes(const RutaBus rutas[], int cantidad) {
    int opcion;
    do {
        cout << "\\n--- REPORTES ---\\n";
        cout << "1. Rutas ordenadas por ingresos\\n";
        cout << "2. Ocupacion y disponibilidad\\n";
        cout << "3. Resumen general\\n";
        cout << "0. Volver\\n";
        opcion = leerEntero("Opcion: ", 0, 3);

        switch (opcion) {
            case 1: reporteIngresos(rutas, cantidad); break;
            case 2: reporteOcupacion(rutas, cantidad); break;
            case 3: reporteResumen(rutas, cantidad); break;
        }
    } while (opcion != 0);
}

void menuBaseDatosYCifrado(RutaBus rutas[], int &cantidad) {
    int opcion;
    do {
        cout << "\\n--- CONEXION A BASE DE DATOS (MYSQL) Y CIFRADO PARA TONTOS ---\\n";
        cout << "1. Ver registros en Base de Datos y su Cifrado para Tontos\\n";
        cout << "2. Probar Cifrado para Tontos con un texto en vivo\\n";
        cout << "3. Configurar usuario/contrasena de MySQL y sincronizar ahora\\n";
        cout << "0. Volver\\n";
        opcion = leerEntero("Opcion: ", 0, 3);

        if (opcion == 1) {
            mostrarArchivoCifradoDB(rutas, cantidad);
        } else if (opcion == 2) {
            mostrarDemostracionCifrado();
        } else if (opcion == 3) {
            mostrarGuiaConexionMySQLenConsola();
            CredencialesMySQL cred = obtenerCredencialesMySQL();
            cout << "Ingrese la contrasena de su MySQL para root@localhost (o presione Enter si no tiene): ";
            string pass;
            getline(cin, pass);
            cred.password = recortar(pass);
            guardarCredencialesMySQL(cred);
            if (guardarRutasEnMySQL(rutas, cantidad)) {
                cout << "[*] Conexion exitosa: tabla rutas_buses actualizada en MySQL (db_transporte_rutas).\\n";
            } else {
                cout << "[*] Se guardo sincronizacion_mysql.sql (verifica que el servicio MySQL este iniciado).\\n";
            }
        }
    } while (opcion != 0);
}
