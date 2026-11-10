#include "servidor_http.h"
#include "logica_rutas.h"
#include "algoritmos.h"
#include "conexion_db.h"
#include "conexion_mysql.h"
#include "cifrado_tontos.h"
#include "validaciones.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#define cerrarSocket closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define cerrarSocket close
#endif

using namespace std;

static string escaparJSON(const string &s) {
    string out;
    for (int i = 0; i < (int)s.length(); i++) {
        char c = s[i];
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') continue;
        else out += c;
    }
    return out;
}

static string extraerCampoForm(const string &datos, const string &clave) {
    string buscado = "&" + datos;
    string patron = "&" + clave + "=";
    size_t pos = buscado.find(patron);
    if (pos == string::npos) {
        patron = "?" + clave + "=";
        pos = buscado.find(patron);
    }
    if (pos == string::npos) return "";
    size_t ini = pos + patron.length();
    size_t fin = buscado.find('&', ini);
    string val = (fin == string::npos) ? buscado.substr(ini) : buscado.substr(ini, fin - ini);

    string limpio;
    for (int i = 0; i < (int)val.length(); i++) {
        if (val[i] == '+') {
            limpio += ' ';
        } else if (val[i] == '%' && i + 2 < (int)val.length()) {
            string hex = val.substr(i + 1, 2);
            char ch = (char)strtol(hex.c_str(), NULL, 16);
            limpio += ch;
            i += 2;
        } else {
            limpio += val[i];
        }
    }
    return recortar(limpio);
}

static string construirJSONEstado(const RutaBus rutas[], int cantidad, bool ok, const string &mensaje) {
    CredencialesMySQL cred = obtenerCredencialesMySQL();
    ostringstream js;
    js << fixed << setprecision(2);
    js << "{";
    js << "\"ok\":" << (ok ? "true" : "false") << ",";
    js << "\"mensaje\":\"" << escaparJSON(mensaje) << "\",";
    js << "\"mysql_conectado\":" << (estadoServidorMySQLActivo() ? "true" : "false") << ",";
    js << "\"mysql_host\":\"" << escaparJSON(cred.host) << "\",";
    js << "\"mysql_puerto\":" << cred.puerto << ",";
    js << "\"mysql_usuario\":\"" << escaparJSON(cred.usuario) << "\",";
    js << "\"mysql_db\":\"" << escaparJSON(cred.baseDatos) << "\",";
    js << "\"ultima_sql\":\"" << escaparJSON(obtenerUltimaSentenciaSQL()) << "\",";
    js << "\"rutas\":[";
    for (int i = 0; i < cantidad; i++) {
        const RutaBus &r = rutas[i];
        ostringstream plana;
        plana << fixed << setprecision(2)
              << r.id << ";" << r.origen << ";" << r.destino << ";"
              << r.capacidad << ";" << r.pasajeros << ";"
              << r.precio << ";" << r.ingresos << ";" << (r.activa ? 1 : 0);
        string cif = cifrarParaTontos(plana.str());

        if (i > 0) js << ",";
        js << "{"
           << "\"id\":" << r.id << ","
           << "\"origen\":\"" << escaparJSON(r.origen) << "\","
           << "\"destino\":\"" << escaparJSON(r.destino) << "\","
           << "\"capacidad\":" << r.capacidad << ","
           << "\"pasajeros\":" << r.pasajeros << ","
           << "\"precio\":" << r.precio << ","
           << "\"ingresos\":" << r.ingresos << ","
           << "\"activa\":" << (r.activa ? "true" : "false") << ","
           << "\"cifrado\":\"" << escaparJSON(cif) << "\""
           << "}";
    }
    js << "]}";
    return js.str();
}

void iniciarServidorWebCRUD(RutaBus rutas[], int &cantidad, int puerto) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "[!] No se pudo iniciar Winsock2.\n";
        return;
    }
#endif

    int servidorFd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (servidorFd < 0) {
        cout << "[!] Error al crear el socket del servidor HTTP.\n";
        return;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(servidorFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(servidorFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_addr.s_addr = INADDR_ANY;
    dir.sin_port = htons((unsigned short)puerto);

    if (bind(servidorFd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        cout << "[!] El puerto " << puerto << " ya esta en uso.\n";
        cerrarSocket(servidorFd);
        return;
    }

    listen(servidorFd, 10);
    guardarRutasEnMySQL(rutas, cantidad);

    cout << "\n====================================================================\n";
    cout << "  SERVIDOR WEB CRUD + MYSQL ACTIVO EN: http://localhost:" << puerto << "\n";
    cout << "====================================================================\n";
    cout << "1. Abre http://localhost:" << puerto << " en tu navegador (o abre frontend_transporte.html)\n";
    cout << "2. Todo lo que crees, edites o elimines en el Frontend HTML se guardara\n";
    cout << "   en tiempo real en MySQL (db_transporte_rutas), rutas.txt y rutas_cifradas.txt\n";
    cout << "3. Presiona Ctrl + C cuando desees detener el servidor.\n\n";

    while (true) {
        sockaddr_in clienteDir;
        socklen_t tam = sizeof(clienteDir);
        int clienteFd = (int)accept(servidorFd, (struct sockaddr *)&clienteDir, &tam);
        if (clienteFd < 0) continue;

        char buffer[16384];
        memset(buffer, 0, sizeof(buffer));
        int leidos = (int)recv(clienteFd, buffer, sizeof(buffer) - 1, 0);
        if (leidos <= 0) {
            cerrarSocket(clienteFd);
            continue;
        }

        string req(buffer, leidos);
        istringstream reqStream(req);
        string metodo, rutaURL;
        reqStream >> metodo >> rutaURL;

        size_t posBody = req.find("\r\n\r\n");
        string body = (posBody != string::npos) ? req.substr(posBody + 4) : "";
        string datos = rutaURL + "&" + body;

        string contenidoRespuesta;
        string tipoContenido = "application/json; charset=utf-8";

        if (metodo == "OPTIONS") {
            contenidoRespuesta = "{}";
        } else if (metodo == "GET" && (rutaURL == "/" || rutaURL == "/index.html" || rutaURL == "/frontend_transporte.html")) {
            ifstream fHtml("frontend/frontend_transporte.html");
            if (!fHtml.is_open()) fHtml.open("frontend_transporte.html");
            if (fHtml.is_open()) {
                ostringstream ss;
                ss << fHtml.rdbuf();
                contenidoRespuesta = ss.str();
                tipoContenido = "text/html; charset=utf-8";
            } else {
                contenidoRespuesta = "<h1>Abre el archivo frontend/frontend_transporte.html en tu navegador</h1>";
                tipoContenido = "text/html; charset=utf-8";
            }
        } else if (metodo == "GET" && rutaURL.find("/api/rutas") == 0) {
            contenidoRespuesta = construirJSONEstado(rutas, cantidad, true, "Datos sincronizados con C++ y MySQL.");
        } else if (metodo == "POST" && rutaURL.find("/api/rutas") == 0) {
            RutaBus nueva;
            nueva.id = atoi(extraerCampoForm(datos, "id").c_str());
            nueva.origen = extraerCampoForm(datos, "origen");
            nueva.destino = extraerCampoForm(datos, "destino");
            nueva.capacidad = atoi(extraerCampoForm(datos, "capacidad").c_str());
            nueva.pasajeros = atoi(extraerCampoForm(datos, "pasajeros").c_str());
            nueva.precio = atof(extraerCampoForm(datos, "precio").c_str());
            nueva.activa = (extraerCampoForm(datos, "activa") != "0");

            string err;
            if (crearRutaDirecta(rutas, cantidad, nueva, err)) {
                cout << "[CRUD CREATE] Ruta " << nueva.id << " guardada en MySQL y rutas.txt\n";
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, true, "Ruta creada y guardada en MySQL.");
            } else {
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, false, err);
            }
        } else if (metodo == "PUT" && rutaURL.find("/api/rutas") == 0) {
            RutaBus editada;
            editada.id = atoi(extraerCampoForm(datos, "id").c_str());
            editada.origen = extraerCampoForm(datos, "origen");
            editada.destino = extraerCampoForm(datos, "destino");
            editada.capacidad = atoi(extraerCampoForm(datos, "capacidad").c_str());
            editada.pasajeros = atoi(extraerCampoForm(datos, "pasajeros").c_str());
            editada.precio = atof(extraerCampoForm(datos, "precio").c_str());
            editada.activa = (extraerCampoForm(datos, "activa") != "0");

            string err;
            if (modificarRutaDirecta(rutas, cantidad, editada, err)) {
                cout << "[CRUD UPDATE] Ruta " << editada.id << " actualizada en MySQL y rutas.txt\n";
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, true, "Ruta actualizada en MySQL.");
            } else {
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, false, err);
            }
        } else if (metodo == "DELETE" && rutaURL.find("/api/rutas") == 0) {
            int idBorrar = atoi(extraerCampoForm(datos, "id").c_str());
            string err;
            if (borrarRutaPorID(rutas, cantidad, idBorrar, err)) {
                cout << "[CRUD DELETE] Ruta " << idBorrar << " eliminada de MySQL y rutas.txt\n";
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, true, "Ruta eliminada de MySQL.");
            } else {
                contenidoRespuesta = construirJSONEstado(rutas, cantidad, false, err);
            }
        } else if (metodo == "POST" && rutaURL.find("/api/mysql-config") == 0) {
            CredencialesMySQL cred = obtenerCredencialesMySQL();
            string h = extraerCampoForm(datos, "host");
            string p = extraerCampoForm(datos, "puerto");
            string u = extraerCampoForm(datos, "usuario");
            string pw = extraerCampoForm(datos, "password");
            string db = extraerCampoForm(datos, "baseDatos");
            if (!h.empty()) cred.host = h;
            if (!p.empty() && atoi(p.c_str()) > 0) cred.puerto = (unsigned int)atoi(p.c_str());
            if (!u.empty()) cred.usuario = u;
            cred.password = pw;
            if (!db.empty()) cred.baseDatos = db;
            guardarCredencialesMySQL(cred);
            bool ok = guardarRutasEnMySQL(rutas, cantidad);
            contenidoRespuesta = construirJSONEstado(rutas, cantidad, ok,
                ok ? "Conectado y sincronizado con MySQL." : "Credenciales guardadas (verifica que el servicio MySQL este activo).");
        } else {
            contenidoRespuesta = construirJSONEstado(rutas, cantidad, true, "Servidor C++ activo.");
        }

        ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << tipoContenido << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Length: " << contenidoRespuesta.length() << "\r\n"
             << "Connection: close\r\n\r\n"
             << contenidoRespuesta;

        string rawResp = resp.str();
        send(clienteFd, rawResp.c_str(), (int)rawResp.length(), 0);
        cerrarSocket(clienteFd);
    }
}
