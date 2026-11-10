import subprocess, tempfile, os, json, shutil

import sys
EXE = sys.argv[1] if len(sys.argv) > 1 else "./RutasBus"

def run(lines, setup=None):
    d = tempfile.mkdtemp()
    if setup is not None:
        open(os.path.join(d, "rutas.txt"), "w").write(setup)
    inp = "\n".join(lines) + "\n"
    p = subprocess.run([EXE], input=inp, capture_output=True, text=True, cwd=d, timeout=20)
    f = os.path.join(d, "rutas.txt")
    data = open(f).read() if os.path.exists(f) else None
    shutil.rmtree(d)
    return p.stdout, data, p.returncode

# helper para registrar: 1(menu rutas) 1(registrar) id orig dest cap precio ... 0(volver)
def reg(id, o, d, cap, pr):
    return ["1", "1", str(id), o, d, str(cap), str(pr), "0"]

T = []
def t(codigo, categoria, descripcion, entrada, esperado, lines, checks, setup=None, filecheck=None, nochecks=None, rc=0):
    out, data, code = run(lines, setup)
    ok = all(c in out for c in checks)
    if nochecks: ok = ok and all(c not in out for c in nochecks)
    if filecheck is not None: ok = ok and data is not None and all(c in data for c in filecheck)
    ok = ok and code == rc
    T.append(dict(codigo=codigo, categoria=categoria, descripcion=descripcion, entrada=entrada,
                  esperado=esperado, ok=ok, salida=out, archivo=data, checks=checks, filecheck=filecheck or [], extra=''))
    return out

base = reg(101,"Guatemala","Xela",40,50)

t("P01","Normal","Registrar una ruta valida","ID 101, Guatemala, Xela, cap 40, Q50.00",
  "Mensaje de exito y ruta guardada en rutas.txt",
  base+["0"], ["Ruta registrada con exito"], filecheck=["101;Guatemala;Xela;40;0;50.00;0.00;1"])

t("P02","Error","ID duplicado","Registrar 101 dos veces",
  "Rechazo: ya existe una ruta con ese ID",
  reg(101,"Guatemala","Xela",40,50)+["1","1","101","Peten","Flores","20","80","0","0","0"],
  ["Ya existe una ruta con ese ID"])

t("P03","Borde","ID 0, negativo y fuera de rango","IDs 0, -5, 10000, luego 7",
  "Cada valor es rechazado y se vuelve a pedir; 7 se acepta",
  ["1","1","0","-5","10000","7","A","B","10","20","0","0","0"],
  ["Valor fuera de rango (1 a 9999)","Ruta registrada con exito"])

t("P04","Error","ID con letras","ID 'abc'",
  "Mensaje de entrada invalida sin bloquear el programa",
  ["1","1","abc","12abc","3","A","B","10","20","0","0","0"],
  ["Entrada invalida. Escriba un numero entero","Ruta registrada con exito"])

t("P05","Borde","Capacidad 0, negativa y mayor al maximo","Capacidades 0, -3, 101, luego 1",
  "Rechazadas 0, -3 y 101; se acepta 1 (limite inferior)",
  ["1","1","1","A","B","0","-3","101","1","10","0","0","0"],
  ["Valor fuera de rango (1 a 100)","Ruta registrada con exito"])

t("P06","Error","Precio negativo, cero y con letras","Precios -10, 0, 'gratis', luego 25.50",
  "Rechazados los tres primeros; se acepta 25.50",
  ["1","1","1","A","B","10","-10","0","gratis","25.50","0","0","0"],
  ["Valor fuera de rango (0.01 a 1000)","Entrada invalida. Escriba un numero","Ruta registrada con exito"],
  filecheck=["25.50"])

t("P07","Error","Origen igual al destino (sin distinguir mayusculas)","Origen 'Xela', destino 'XELA'",
  "Rechazo: origen y destino no pueden ser iguales",
  ["1","1","1","Xela","XELA","0","0"],
  ["El origen y el destino no pueden ser iguales"])

t("P08","Error","Texto vacio o solo espacios en origen","Origen '' y '   '",
  "Rechazo: el texto no puede estar vacio; luego se acepta",
  ["1","1","1","","   ","Xela","Peten","10","20","0","0","0"],
  ["El texto no puede estar vacio","Ruta registrada con exito"])

t("P09","Normal","Venta valida actualiza pasajeros e ingresos","Ruta 101 (cap 40, Q50), vender 3",
  "Total Q150.00; en archivo pasajeros=3 e ingresos=150.00",
  base+["2","1","101","3","0","0"],
  ["Venta exitosa. Total a cobrar: Q150.00"], filecheck=["101;Guatemala;Xela;40;3;50.00;150.00;1"])

t("P10","Borde","Vender exactamente toda la capacidad","Ruta cap 5, vender 5",
  "Venta aceptada; asientos disponibles queda en 0",
  reg(1,"A","B",5,10)+["2","1","1","5","0","0"],
  ["Venta exitosa. Total a cobrar: Q50.00"], filecheck=["1;A;B;5;5;10.00;50.00;1"])

t("P11","Error","Vender mas boletos que los disponibles","Cap 5, intentar 6 y luego 0",
  "Rechazo por rango (1 a 5): nunca se sobrevende",
  reg(1,"A","B",5,10)+["2","1","1","6","0","0","0"],
  ["Valor fuera de rango (0 a 5)"], filecheck=["1;A;B;5;0;10.00;0.00;1"])

t("P12","Borde","Vender en una ruta ya llena","Llenar ruta de 5 y vender de nuevo",
  "Rechazo: ruta alcanzo su capacidad maxima",
  reg(1,"A","B",5,10)+["2","1","1","5","1","1","0","0"],
  ["alcanzo su capacidad maxima. Venta rechazada"])

t("P13","Error","Vender en ruta inexistente","ID 999",
  "Mensaje: ruta no encontrada",
  ["2","1","999","0","0"], ["Ruta no encontrada"])

t("P14","Error","Vender en ruta inactiva","Desactivar ruta 101 y vender",
  "Rechazo: ruta inactiva",
  base+["1","3","101","0","2","1","101","0","0"],
  ["ahora esta INACTIVA","La ruta esta inactiva"])

t("P15","Normal","Cancelar boletos recalcula ingresos","Vender 4 y cancelar 1 en ruta Q10",
  "Devolucion Q10.00; archivo con pasajeros=3 e ingresos=30.00",
  reg(1,"A","B",10,10)+["2","1","1","4","3","1","1","0","0"],
  ["Devolucion: Q10.00"], filecheck=["1;A;B;10;3;10.00;30.00;1"])

t("P16","Error","Cancelar en ruta sin boletos vendidos","Ruta nueva, cancelar",
  "Mensaje: no tiene boletos vendidos",
  reg(1,"A","B",10,10)+["2","3","1","0","0"],
  ["La ruta no tiene boletos vendidos"])

t("P17","Normal","Busqueda por origen sin distinguir mayusculas","Buscar 'guatemala' (registrado 'Guatemala')",
  "Muestra las rutas con ese origen",
  base+reg(102,"Guatemala","Peten",30,120)+["3","2","guatemala","0","0"],
  ["Xela","Peten"])

t("P18","Error","Busqueda por origen inexistente","Origen 'Mixco'",
  "Mensaje: no hay rutas con ese origen",
  base+["3","2","Mixco","0","0"], ["No hay rutas con ese origen"])

t("P19","Normal","Busqueda por destino","Destino 'xela'",
  "Lista la ruta 101",
  base+["3","3","xela","0","0"], ["101"])

t("P20","Normal","Busqueda secuencial por ID","Buscar ID 101 y ID 55",
  "101 se muestra; 55 informa no encontrada",
  base+["3","1","101","1","55","0","0"], ["Guatemala","Ruta no encontrada"])

orden_in = (reg(1,"A","B",10,10)+reg(2,"C","D",10,30)+reg(3,"E","F",10,20)
            +["2","1","1","5","1","2","5","1","3","5","0"]+["4","1","0","0"])
out = t("P21","Normal","Ranking por ingresos (burbuja) mayor a menor","Ingresos: R1=Q50, R2=Q150, R3=Q100",
  "Orden: ruta 2, ruta 3, ruta 1",
  orden_in, ["Q"], nochecks=[])
# verificar orden por posicion en la salida
rep = out.split("REPORTE 1")[1]
T[-1]["ok"] = T[-1]["ok"] and rep.index("C - D") < rep.index("E - F") < rep.index("A - B")
T[-1]["extra"]="Orden obtenido en el reporte: C-D (Q150), E-F (Q100), A-B (Q50)"

emp = (reg(5,"A","B",10,10)+reg(2,"C","D",10,10)+["2","1","5","2","1","2","2","0","4","1","0","0"])
out = t("P22","Borde","Empate de ingresos: desempata por pasajeros y luego ID","R5 y R2 con mismos ingresos y pasajeros",
  "Primero la ruta de menor ID (2)", emp, ["Q"])
rep = out.split("REPORTE 1")[1]
T[-1]["ok"] = T[-1]["ok"] and rep.index("C - D") < rep.index("A - B")
T[-1]["extra"]="Orden obtenido: ruta 2 (C-D) y despues ruta 5 (A-B)"

t("P23","Borde","Reportes sin rutas registradas","Abrir reportes 1, 2 y 3 sin datos",
  "Mensaje claro sin errores ni division entre cero",
  ["4","1","2","3","0","0"], ["No hay rutas registradas"])

t("P24","Normal","Reporte de ocupacion","Cap 4, vender 4 en R1 (llena) y R2 sin ventas",
  "R1 aparece como LLENA con 100.0%",
  reg(1,"A","B",4,10)+reg(2,"C","D",4,10)+["2","1","1","4","0","4","2","0","0"],
  ["LLENA","100.0%","Disponible"])

t("P25","Normal","Resumen general","2 rutas, ventas conocidas",
  "Pasajeros, ingresos totales (Q150.00) y rutas extremas correctas",
  reg(1,"A","B",10,10)+reg(2,"C","D",10,20)+["2","1","1","5","1","2","5","0","4","3","0","0"],
  ["Ingresos totales:         Q150.00","Pasajeros transportados:  10 de 20","Ocupacion global:         50.00%"])

rec = (reg(1,"Guate","Xela",10,60)+reg(2,"Guate","Xela",10,45)+reg(3,"Guate","Xela",2,30))
t("P26","Normal","RETO: recomienda la ruta mas barata con cupo para el grupo","Grupo de 5; rutas Q60 (10), Q45 (10), Q30 (cap 2)",
  "Recomienda la ruta 2 (Q45): la de Q30 se descarta por no caber el grupo",
  rec+["2","2","guate","xela","5","1","0","0"],
  ["Venta registrada en la ruta 2"], filecheck=["2;Guate;Xela;10;5;45.00;225.00;1"])

t("P27","Borde","RETO: empate de precio elige la de mas asientos libres","Dos rutas Q50; una ya con 8 vendidos",
  "Recomienda la ruta con mas libres (ruta 2)",
  reg(1,"G","X",10,50)+reg(2,"G","X",10,50)+["2","1","1","8","0","2","2","g","x","1","1","0","0"],
  ["Venta registrada en la ruta 2"])

t("P28","Error","RETO: ningun bus tiene cupo suficiente","Grupo de 8 y la mayor disponibilidad es 5",
  "Mensaje con maximo disponible 5; no se vende",
  reg(1,"G","X",5,50)+["2","2","G","X","8","0","0"],
  ["Ninguna ruta tiene 8 asientos libres (maximo disponible: 5)"])

t("P29","Error","Recomendar para recorrido inexistente","Origen/Destino no registrados",
  "Mensaje: no hay rutas activas para ese recorrido",
  base+["2","2","Mixco","Antigua","2","0","0"],
  ["No hay rutas activas para ese recorrido"])

setup_ok = "10;Guatemala;Xela;40;3;50.00;150.00;1\n20;Peten;Flores;20;0;80.00;0.00;0\n"
t("P30","Normal","Persistencia: cargar archivo de una ejecucion previa","rutas.txt con 2 rutas existentes",
  "Mensaje 'Se cargaron 2 rutas' y datos disponibles",
  ["1","2","0","0"], ["Se cargaron 2 rutas","Flores"], setup=setup_ok)

t("P31","Normal","Persistencia: guardar, y datos intactos al reabrir","Cargar, vender 2 boletos, salir",
  "Archivo actualizado: pasajeros=5 e ingresos=250.00",
  ["2","1","10","2","0","0"], ["Venta exitosa"], setup=setup_ok,
  filecheck=["10;Guatemala;Xela;40;5;50.00;250.00;1","20;Peten;Flores;20;0;80.00;0.00;0"])

setup_bad = ("10;Guatemala;Xela;40;3;50.00;150.00;1\n"
             "basura sin formato\n"
             "11;A;B;10;99;5.00;0;1\n"
             "12;A;B;-5;0;5.00;0;1\n"
             "10;Dup;Dup2;10;0;5.00;0;1\n"
             "13;C;D;10;0;7.00;0.00;1\n")
t("P32","Error","Archivo corrupto: lineas invalidas se omiten","Archivo con basura, pasajeros>capacidad, capacidad negativa y ID repetido",
  "Se cargan solo las 2 lineas validas y se avisa de 4 omitidas",
  ["0"], ["Se cargaron 2 rutas (4 omitidas)"], setup=setup_bad)

t("P33","Borde","Sin archivo previo","No existe rutas.txt",
  "Mensaje de inicio limpio y el programa funciona",
  ["1","2","0","0"], ["No hay archivo previo","No hay rutas registradas"])

t("P34","Error","Opcion de menu invalida (letras, simbolos, fuera de rango)","'x', '9', '', '2.5'",
  "Cada una rechazada; el menu se vuelve a mostrar",
  ["x","9","","2.5","0"], ["Entrada invalida","fuera de rango"])

t("P35","Error","Fin de la entrada sin cerrar el programa","Se corta la entrada en medio de un registro",
  "El programa termina de forma controlada sin ciclo infinito",
  ["1","1","5"], ["Entrada cerrada"], rc=1)

# limite de 100 rutas
big = ["1"]
for i in range(1,101):
    big += ["1", str(i), "O"+str(i), "D"+str(i), "10", "10"]
big += ["1","101","X","Y","10","10","0","0"]
t("P36","Borde","Limite de 100 rutas en el arreglo","Registrar 100 rutas e intentar la 101",
  "Mensaje de limite alcanzado; sin desbordar el arreglo",
  big, ["Limite de 100 rutas alcanzado"])

t("P37","Normal","Activar y reactivar ruta","Desactivar y activar de nuevo la ruta 101",
  "Estado alterna INACTIVA -> ACTIVA",
  base+["1","3","101","3","101","0","0"], ["ahora esta INACTIVA","ahora esta ACTIVA"])

t("P38","Error","Decimal en campo entero y espacios alrededor del numero","Boletos '2.5' y ' 3 '",
  "2.5 rechazado; ' 3 ' aceptado (se ignoran espacios)",
  base+["2","1","101","2.5"," 3 ","0","0"], ["Entrada invalida","Venta exitosa. Total a cobrar: Q150.00"])

json.dump(T, open("resultados.json","w"), ensure_ascii=False, indent=1)
print(sum(x["ok"] for x in T), "/", len(T), "pasan")
for x in T:
    if not x["ok"]: print("FALLA", x["codigo"], x["descripcion"])
