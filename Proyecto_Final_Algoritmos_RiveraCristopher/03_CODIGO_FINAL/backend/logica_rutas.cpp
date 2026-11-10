#include "logica_rutas.h"
#include "validaciones.h"
#include "algoritmos.h"
#include "interfaz.h"
#include "conexion_mysql.h"
#include "conexion_db.h"
#include <iostream>
#include <iomanip>

using namespace std;

// CREATE directo: valida RV1-RV10, inserta en el arreglo y sincroniza con MySQL y archivos
bool crearRutaDirecta(RutaBus rutas[], int &cantidad, const RutaBus &nueva, string &mensajeError) {
    if (cantidad >= MAX_RUTAS) {
        mensajeError = "Limite de 100 rutas alcanzado.";
        return false;
    }
    if (nueva.id < 1 || nueva.id > ID_MAX) {
        mensajeError = "Valor de ID fuera de rango (1 a 9999).";
        return false;
    }
    if (buscarPorID(rutas, cantidad, nueva.id) != -1) {
        mensajeError = "Ya existe una ruta con ese ID.";
        return false;
    }
    string ori = recortar(nueva.origen);
    string des = recortar(nueva.destino);
    if (ori.empty() || des.empty()) {
        mensajeError = "El origen y el destino no pueden estar vacios.";
        return false;
    }
    if ((int)ori.length() > LONGITUD_TEXTO_MAX || (int)des.length() > LONGITUD_TEXTO_MAX) {
        mensajeError = "Maximo 30 caracteres en origen y destino.";
        return false;
    }
    for (int i = 0; i < (int)ori.length(); i++) {
        if (ori[i] == ';') {
            mensajeError = "No se permite el caracter ';'.";
            return false;
        }
    }
    for (int i = 0; i < (int)des.length(); i++) {
        if (des[i] == ';') {
            mensajeError = "No se permite el caracter ';'.";
            return false;
        }
    }
    if (mismoTexto(ori, des)) {
        mensajeError = "El origen y el destino no pueden ser iguales.";
        return false;
    }
    if (nueva.capacidad < 1 || nueva.capacidad > CAPACIDAD_MAX) {
        mensajeError = "Capacidad fuera de rango (1 a 100).";
        return false;
    }
    if (nueva.precio < 0.01 || nueva.precio > PRECIO_MAX) {
        mensajeError = "Precio fuera de rango (0.01 a 1000.00).";
        return false;
    }
    int pas = nueva.pasajeros;
    if (pas < 0 || pas > nueva.capacidad) {
        pas = 0;
    }

    RutaBus r = nueva;
    r.origen = ori;
    r.destino = des;
    r.pasajeros = pas;
    r.ingresos = r.pasajeros * r.precio;

    rutas[cantidad] = r;
    cantidad++;

    insertarRutaEnMySQL(r);
    guardarDatos(rutas, cantidad);
    return true;
}

// CREATE desde consola
bool registrarRuta(RutaBus rutas[], int &cantidad) {
    if (cantidad >= MAX_RUTAS) {
        cout << "\\n[!] Limite de " << MAX_RUTAS << " rutas alcanzado.\\n";
        return false;
    }

    RutaBus nueva;
    cout << "\\n--- REGISTRAR RUTA (CREATE) ---\\n";
    nueva.id = leerEntero("ID de la ruta (1-9999): ", 1, ID_MAX);

    if (buscarPorID(rutas, cantidad, nueva.id) != -1) {
        cout << "[!] Ya existe una ruta con ese ID.\\n";
        return false;
    }

    nueva.origen = leerTexto("Origen: ");
    nueva.destino = leerTexto("Destino: ");

    if (mismoTexto(nueva.origen, nueva.destino)) {
        cout << "[!] El origen y el destino no pueden ser iguales.\\n";
        return false;
    }

    nueva.capacidad = leerEntero("Capacidad de asientos (1-100): ", 1, CAPACIDAD_MAX);
    nueva.precio = leerDecimal("Precio del pasaje en Q (mayor a 0): ", 0.01, PRECIO_MAX);
    nueva.pasajeros = 0;
    nueva.ingresos = 0.0;
    nueva.activa = true;

    string err;
    if (!crearRutaDirecta(rutas, cantidad, nueva, err)) {
        cout << "[!] " << err << "\\n";
        return false;
    }
    cout << "\\n[*] Ruta registrada con exito y guardada en la Base de Datos.\\n";
    return true;
}

// READ: Muestra todas las rutas cargadas
void listarRutas(const RutaBus rutas[], int cantidad) {
    cout << "\\n--- LISTADO DE RUTAS (" << cantidad << ") ---\\n";
    if (cantidad == 0) {
        cout << "[!] No hay rutas registradas.\\n";
        return;
    }
    imprimirEncabezadoRutas();
    for (int i = 0; i < cantidad; i++) {
        imprimirFilaRuta(rutas[i]);
    }
}

// UPDATE directo: modifica origen, destino, capacidad, precio, pasajeros y estado de una ruta existente
bool modificarRutaDirecta(RutaBus rutas[], int cantidad, const RutaBus &editada, string &mensajeError) {
    int pos = buscarPorID(rutas, cantidad, editada.id);
    if (pos == -1) {
        mensajeError = "Ruta no encontrada.";
        return false;
    }
    string ori = recortar(editada.origen);
    string des = recortar(editada.destino);
    if (ori.empty() || des.empty()) {
        mensajeError = "El origen y el destino no pueden estar vacios.";
        return false;
    }
    if (mismoTexto(ori, des)) {
        mensajeError = "El origen y el destino no pueden ser iguales.";
        return false;
    }
    if (editada.capacidad < 1 || editada.capacidad > CAPACIDAD_MAX) {
        mensajeError = "Capacidad fuera de rango (1 a 100).";
        return false;
    }
    if (editada.pasajeros < 0 || editada.pasajeros > editada.capacidad) {
        mensajeError = "La capacidad no puede ser menor a los pasajeros actuales.";
        return false;
    }
    if (editada.precio < 0.01 || editada.precio > PRECIO_MAX) {
        mensajeError = "Precio fuera de rango (0.01 a 1000.00).";
        return false;
    }

    rutas[pos].origen = ori;
    rutas[pos].destino = des;
    rutas[pos].capacidad = editada.capacidad;
    rutas[pos].pasajeros = editada.pasajeros;
    rutas[pos].precio = editada.precio;
    rutas[pos].ingresos = rutas[pos].pasajeros * rutas[pos].precio;
    rutas[pos].activa = editada.activa;

    actualizarRutaEnMySQL(rutas[pos]);
    guardarDatos(rutas, cantidad);
    return true;
}

// UPDATE desde consola
bool actualizarRuta(RutaBus rutas[], int cantidad) {
    cout << "\\n--- ACTUALIZAR / MODIFICAR RUTA (UPDATE) ---\\n";
    int id = leerEntero("ID de la ruta a modificar: ", 1, ID_MAX);
    int pos = buscarPorID(rutas, cantidad, id);

    if (pos == -1) {
        cout << "[!] Ruta no encontrada.\\n";
        return false;
    }

    RutaBus editada = rutas[pos];
    cout << "Datos actuales -> " << editada.origen << " a " << editada.destino
         << " | Cap: " << editada.capacidad << " | Pasajeros: " << editada.pasajeros
         << " | Precio: Q" << fixed << setprecision(2) << editada.precio << "\\n";

    editada.origen = leerTexto("Nuevo origen: ");
    editada.destino = leerTexto("Nuevo destino: ");
    if (mismoTexto(editada.origen, editada.destino)) {
        cout << "[!] El origen y el destino no pueden ser iguales.\\n";
        return false;
    }

    int minCap = (editada.pasajeros > 1) ? editada.pasajeros : 1;
    editada.capacidad = leerEntero("Nueva capacidad (" + to_string(minCap) + "-100): ", minCap, CAPACIDAD_MAX);
    editada.precio = leerDecimal("Nuevo precio en Q (mayor a 0): ", 0.01, PRECIO_MAX);
    int est = leerEntero("Estado (1 = Activa, 0 = Inactiva): ", 0, 1);
    editada.activa = (est == 1);

    string err;
    if (!modificarRutaDirecta(rutas, cantidad, editada, err)) {
        cout << "[!] " << err << "\\n";
        return false;
    }
    cout << "[*] Ruta " << id << " actualizada correctamente en memoria y Base de Datos.\\n";
    return true;
}

// Cambia rapidamente el estado Activa/Inactiva
bool cambiarEstadoRuta(RutaBus rutas[], int cantidad) {
    cout << "\\n--- ACTIVAR / DESACTIVAR RUTA ---\\n";
    int id = leerEntero("ID de la ruta: ", 1, ID_MAX);
    int pos = buscarPorID(rutas, cantidad, id);

    if (pos == -1) {
        cout << "[!] Ruta no encontrada.\\n";
        return false;
    }

    rutas[pos].activa = !rutas[pos].activa;
    actualizarRutaEnMySQL(rutas[pos]);
    guardarDatos(rutas, cantidad);
    cout << "[*] La ruta " << id << " ahora esta "
         << (rutas[pos].activa ? "ACTIVA" : "INACTIVA") << ".\\n";
    return true;
}

// DELETE directo: elimina del arreglo desplazando elementos a la izquierda y borra en MySQL
bool borrarRutaPorID(RutaBus rutas[], int &cantidad, int idBorrar, string &mensajeError) {
    int pos = buscarPorID(rutas, cantidad, idBorrar);
    if (pos == -1) {
        mensajeError = "Ruta no encontrada.";
        return false;
    }

    for (int i = pos; i < cantidad - 1; i++) {
        rutas[i] = rutas[i + 1];
    }
    cantidad--;

    eliminarRutaEnMySQL(idBorrar);
    guardarDatos(rutas, cantidad);
    return true;
}

// DELETE desde consola
bool eliminarRuta(RutaBus rutas[], int &cantidad) {
    cout << "\\n--- ELIMINAR RUTA (DELETE) ---\\n";
    int id = leerEntero("ID de la ruta a eliminar: ", 1, ID_MAX);
    int pos = buscarPorID(rutas, cantidad, id);

    if (pos == -1) {
        cout << "[!] Ruta no encontrada.\\n";
        return false;
    }

    cout << "Ruta seleccionada: " << rutas[pos].id << " (" << rutas[pos].origen
         << " -> " << rutas[pos].destino << ")\\n";
    int confirmar = leerEntero("Confirmar eliminacion? (1 = si, 0 = cancelar): ", 0, 1);
    if (confirmar == 0) {
        cout << "[*] Eliminacion cancelada.\\n";
        return false;
    }

    string err;
    if (!borrarRutaPorID(rutas, cantidad, id, err)) {
        cout << "[!] " << err << "\\n";
        return false;
    }
    cout << "[*] Ruta " << id << " eliminada de memoria y de la Base de Datos.\\n";
    return true;
}

void aplicarVenta(RutaBus &ruta, int boletos) {
    if (boletos <= 0 || ruta.pasajeros + boletos > ruta.capacidad) {
        return;
    }
    ruta.pasajeros += boletos;
    ruta.ingresos = ruta.pasajeros * ruta.precio;
}

bool venderBoletos(RutaBus rutas[], int cantidad) {
    cout << "\\n--- VENTA DE BOLETOS ---\\n";
    int id = leerEntero("ID de la ruta: ", 1, ID_MAX);
    int pos = buscarPorID(rutas, cantidad, id);

    if (pos == -1) {
        cout << "[!] Ruta no encontrada.\\n";
        return false;
    }

    RutaBus &ruta = rutas[pos];
    if (!ruta.activa) {
        cout << "[!] La ruta esta inactiva; no se pueden vender boletos.\\n";
        return false;
    }

    int disponibles = ruta.capacidad - ruta.pasajeros;
    cout << "Ruta: " << ruta.origen << " -> " << ruta.destino << "\\n";
    cout << "Asientos disponibles: " << disponibles << " | Precio: Q"
         << fixed << setprecision(2) << ruta.precio << "\\n";

    if (disponibles == 0) {
        cout << "[!] La ruta alcanzo su capacidad maxima. Venta rechazada.\\n";
        return false;
    }

    int boletos = leerEntero("Boletos a vender (0 = cancelar): ", 0, disponibles);
    if (boletos == 0) {
        cout << "[*] Venta cancelada.\\n";
        return false;
    }

    aplicarVenta(ruta, boletos);
    actualizarRutaEnMySQL(ruta);
    guardarDatos(rutas, cantidad);
    cout << "\\n[*] Venta exitosa. Total a cobrar: Q" << fixed << setprecision(2)
         << (boletos * ruta.precio) << "\\n";
    return true;
}

bool cancelarBoletos(RutaBus rutas[], int cantidad) {
    cout << "\\n--- CANCELACION DE BOLETOS ---\\n";
    int id = leerEntero("ID de la ruta: ", 1, ID_MAX);
    int pos = buscarPorID(rutas, cantidad, id);

    if (pos == -1) {
        cout << "[!] Ruta no encontrada.\\n";
        return false;
    }

    RutaBus &ruta = rutas[pos];
    if (ruta.pasajeros == 0) {
        cout << "[!] La ruta no tiene boletos vendidos.\\n";
        return false;
    }

    cout << "Boletos vendidos: " << ruta.pasajeros << "\\n";
    int boletos = leerEntero("Boletos a cancelar (0 = salir): ", 0, ruta.pasajeros);
    if (boletos == 0) {
        cout << "[*] Operacion cancelada.\\n";
        return false;
    }

    ruta.pasajeros -= boletos;
    ruta.ingresos = ruta.pasajeros * ruta.precio;
    actualizarRutaEnMySQL(ruta);
    guardarDatos(rutas, cantidad);
    cout << "[*] Devolucion: Q" << fixed << setprecision(2) << (boletos * ruta.precio) << "\\n";
    return true;
}

bool recomendarYVender(RutaBus rutas[], int cantidad) {
    cout << "\\n--- RECOMENDAR RUTA Y VENDER ---\\n";
    string origen = leerTexto("Origen: ");
    string destino = leerTexto("Destino: ");
    int necesarios = leerEntero("Cantidad de pasajeros (1-100): ", 1, CAPACIDAD_MAX);

    int mejor = -1;
    int coincidencias = 0;
    int maxDisponibles = 0;

    for (int i = 0; i < cantidad; i++) {
        if (!mismoTexto(rutas[i].origen, origen) || !mismoTexto(rutas[i].destino, destino)) {
            continue;
        }
        if (!rutas[i].activa) {
            continue;
        }

        coincidencias++;
        int disp = rutas[i].capacidad - rutas[i].pasajeros;
        if (disp > maxDisponibles) {
            maxDisponibles = disp;
        }

        if (disp < necesarios) {
            continue;
        }

        if (mejor == -1) {
            mejor = i;
        } else if (rutas[i].precio < rutas[mejor].precio) {
            mejor = i;
        } else if (rutas[i].precio == rutas[mejor].precio &&
                   disp > (rutas[mejor].capacidad - rutas[mejor].pasajeros)) {
            mejor = i;
        }
    }

    if (coincidencias == 0) {
        cout << "[!] No hay rutas activas para ese recorrido.\\n";
        return false;
    }
    if (mejor == -1) {
        cout << "[!] Ninguna ruta tiene " << necesarios
             << " asientos libres (maximo disponible: " << maxDisponibles << ").\\n";
        return false;
    }

    cout << "\\nRuta recomendada:\\n";
    imprimirEncabezadoRutas();
    imprimirFilaRuta(rutas[mejor]);
    cout << "Total del grupo: Q" << fixed << setprecision(2)
         << (necesarios * rutas[mejor].precio) << "\\n";

    int confirmar = leerEntero("Vender ahora? (1 = si, 0 = no): ", 0, 1);
    if (confirmar == 0) {
        cout << "[*] No se realizo la venta.\\n";
        return false;
    }

    aplicarVenta(rutas[mejor], necesarios);
    actualizarRutaEnMySQL(rutas[mejor]);
    guardarDatos(rutas, cantidad);
    cout << "[*] Venta registrada en la ruta " << rutas[mejor].id << ".\\n";
    return true;
}
