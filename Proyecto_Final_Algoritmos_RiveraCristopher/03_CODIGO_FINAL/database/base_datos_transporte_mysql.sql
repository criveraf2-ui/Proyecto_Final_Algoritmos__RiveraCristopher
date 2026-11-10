-- ============================================================================
-- UNIVERSIDAD MARIANO GALVEZ DE GUATEMALA
-- Ingenieria en Sistemas de Informacion y Ciencias de la Computacion
-- Curso: Algoritmos (2026) - Proyecto 12: Transporte y Rutas (Variante A)
-- Script Completo de Base de Datos en MySQL (Tablas, Restricciones, Vistas,
-- Procedimientos Almacenados, Cifrado para Tontos y Datos Iniciales)
-- ============================================================================

DROP DATABASE IF EXISTS db_transporte_rutas;
CREATE DATABASE db_transporte_rutas
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE db_transporte_rutas;

-- ----------------------------------------------------------------------------
-- 1. TABLA PRINCIPAL: rutas_buses
-- Incluye las mismas reglas de validacion (RV1 - RV10) del sistema en C++17
-- y una columna para almacenar el respaldo con "Cifrado para Tontos" (+3 ASCII)
-- ----------------------------------------------------------------------------
CREATE TABLE rutas_buses (
    id INT PRIMARY KEY,
    origen VARCHAR(30) NOT NULL,
    destino VARCHAR(30) NOT NULL,
    capacidad INT NOT NULL,
    pasajeros INT NOT NULL DEFAULT 0,
    precio DECIMAL(10, 2) NOT NULL,
    ingresos DECIMAL(12, 2) NOT NULL DEFAULT 0.00,
    activa TINYINT(1) NOT NULL DEFAULT 1,
    linea_cifrado_tontos VARCHAR(150) NOT NULL,
    fecha_actualizacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

    -- Reglas de validacion en el motor MySQL
    CONSTRAINT chk_id_rango CHECK (id >= 1 AND id <= 9999),
    CONSTRAINT chk_origen_destino CHECK (LOWER(TRIM(origen)) <> LOWER(TRIM(destino))),
    CONSTRAINT chk_capacidad_rango CHECK (capacidad >= 1 AND capacidad <= 100),
    CONSTRAINT chk_pasajeros_cupo CHECK (pasajeros >= 0 AND pasajeros <= capacidad),
    CONSTRAINT chk_precio_rango CHECK (precio >= 0.01 AND precio <= 1000.00),
    CONSTRAINT chk_estado_activa CHECK (activa IN (0, 1))
);

-- ----------------------------------------------------------------------------
-- 2. TABLA DE MOVIMIENTOS: historial_boletos
-- Registra cada venta o cancelacion realizada sobre una ruta
-- ----------------------------------------------------------------------------
CREATE TABLE historial_boletos (
    id_movimiento INT AUTO_INCREMENT PRIMARY KEY,
    id_ruta INT NOT NULL,
    tipo_operacion ENUM('VENTA', 'CANCELACION', 'RECOMENDACION') NOT NULL,
    cantidad_boletos INT NOT NULL,
    monto_operacion DECIMAL(10, 2) NOT NULL,
    fecha_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_historial_ruta FOREIGN KEY (id_ruta)
        REFERENCES rutas_buses(id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- ----------------------------------------------------------------------------
-- 3. DATOS INICIALES (Coinciden con rutas.txt y rutas_cifradas.txt en C++)
-- ----------------------------------------------------------------------------
INSERT INTO rutas_buses
(id, origen, destino, capacidad, pasajeros, precio, ingresos, activa, linea_cifrado_tontos)
VALUES
(101, 'Guatemala', 'Quetzaltenango', 40, 18, 60.00, 1080.00, 1, '434>Jxdwhpdod>Txhw}dowhqdqjr>73>4;>93133>43;3133>4'),
(102, 'Guatemala', 'Peten',          45, 45, 150.00, 6750.00, 1, '435>Jxdwhpdod>Shwhq>78>78>483133>9:83133>4'),
(103, 'Guatemala', 'Escuintla',      30, 12, 25.00, 300.00, 1, '436>Jxdwhpdod>Hvfxlnwod>63>45>58133>633133>4'),
(104, 'Xela',      'Huehuetenango',  35,  0, 40.00,   0.00, 1, '437>[hod>Kxhkxhwhqdqjr>68>3>73133>3133>4'),
(105, 'Guatemala', 'Quetzaltenango', 50, 20, 55.00, 1100.00, 1, '438>Jxdwhpdod>Txhw}dowhqdqjr>83>53>88133>4433133>4'),
(106, 'Coban',     'Guatemala',      38, 10, 70.00, 700.00, 0, '439>Fredq>Jxdwhpdod>6;>43>:3133>:33133>3');

INSERT INTO historial_boletos (id_ruta, tipo_operacion, cantidad_boletos, monto_operacion)
VALUES
(101, 'VENTA', 18, 1080.00),
(102, 'VENTA', 45, 6750.00),
(103, 'VENTA', 12, 300.00),
(105, 'RECOMENDACION', 20, 1100.00),
(106, 'VENTA', 10, 700.00);

-- ----------------------------------------------------------------------------
-- 4. VISTAS PARA LOS 3 REPORTES DEL SISTEMA
-- ----------------------------------------------------------------------------

-- REPORTE 1: Ranking de rutas por ingresos (mismo triple criterio de desempate que Burbuja en C++)
CREATE OR REPLACE VIEW vw_reporte_ingresos AS
SELECT
    id,
    CONCAT(origen, ' - ', destino) AS recorrido,
    pasajeros,
    precio,
    ingresos,
    CASE WHEN activa = 1 THEN 'Activa' ELSE 'Inactiva' END AS estado
FROM rutas_buses
ORDER BY ingresos DESC, pasajeros DESC, id ASC;

-- REPORTE 2: Ocupacion y disponibilidad por recorrido
CREATE OR REPLACE VIEW vw_reporte_ocupacion AS
SELECT
    id,
    CONCAT(origen, ' - ', destino) AS recorrido,
    capacidad,
    pasajeros AS ocupados,
    (capacidad - pasajeros) AS disponibles,
    ROUND((pasajeros * 100.0) / capacidad, 1) AS porcentaje_ocupacion,
    CASE
        WHEN activa = 0 THEN 'Inactiva'
        WHEN pasajeros = capacidad THEN 'LLENA'
        ELSE 'Disponible'
    END AS estado
FROM rutas_buses;

-- REPORTE 3: Resumen general con contadores y acumuladores
CREATE OR REPLACE VIEW vw_resumen_general AS
SELECT
    COUNT(*) AS total_rutas,
    SUM(CASE WHEN activa = 1 THEN 1 ELSE 0 END) AS rutas_activas,
    SUM(CASE WHEN activa = 0 THEN 1 ELSE 0 END) AS rutas_inactivas,
    SUM(CASE WHEN pasajeros = capacidad THEN 1 ELSE 0 END) AS rutas_llenas,
    SUM(pasajeros) AS total_pasajeros_transportados,
    SUM(capacidad) AS capacidad_total_asientos,
    ROUND((SUM(pasajeros) * 100.0) / NULLIF(SUM(capacidad), 0), 2) AS ocupacion_global_pct,
    SUM(ingresos) AS ingresos_totales_q,
    ROUND(AVG(ingresos), 2) AS ingreso_promedio_por_ruta_q
FROM rutas_buses;

-- ----------------------------------------------------------------------------
-- 5. PROCEDIMIENTOS ALMACENADOS (Logica del Reto Algoritmico y Ventas en SQL)
-- ----------------------------------------------------------------------------

DELIMITER //

-- Procedimiento para vender boletos validando capacidad y estado activo
CREATE PROCEDURE sp_vender_boletos(
    IN p_id_ruta INT,
    IN p_boletos INT
)
BEGIN
    DECLARE v_cap INT;
    DECLARE v_pas INT;
    DECLARE v_act TINYINT;
    DECLARE v_pre DECIMAL(10,2);

    SELECT capacidad, pasajeros, activa, precio
      INTO v_cap, v_pas, v_act, v_pre
      FROM rutas_buses
     WHERE id = p_id_ruta;

    IF v_cap IS NULL THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Error: La ruta indicada no existe.';
    ELSEIF v_act = 0 THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Error: La ruta esta inactiva; no se pueden vender boletos.';
    ELSEIF p_boletos <= 0 OR (v_pas + p_boletos) > v_cap THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Error: Cantidad de boletos excede los asientos disponibles.';
    ELSE
        UPDATE rutas_buses
           SET pasajeros = pasajeros + p_boletos,
               ingresos  = (pasajeros + p_boletos) * precio
         WHERE id = p_id_ruta;

        INSERT INTO historial_boletos (id_ruta, tipo_operacion, cantidad_boletos, monto_operacion)
        VALUES (p_id_ruta, 'VENTA', p_boletos, p_boletos * v_pre);
    END IF;
END //

-- Procedimiento del Reto Algoritmico: recomienda la ruta activa mas barata donde quepa todo el grupo
CREATE PROCEDURE sp_recomendar_ruta(
    IN p_origen VARCHAR(30),
    IN p_destino VARCHAR(30),
    IN p_pasajeros_grupo INT
)
BEGIN
    SELECT
        id,
        origen,
        destino,
        capacidad,
        pasajeros,
        (capacidad - pasajeros) AS asientos_libres,
        precio AS precio_boleto,
        (p_pasajeros_grupo * precio) AS total_grupo_q
    FROM rutas_buses
    WHERE LOWER(origen) = LOWER(p_origen)
      AND LOWER(destino) = LOWER(p_destino)
      AND activa = 1
      AND (capacidad - pasajeros) >= p_pasajeros_grupo
    ORDER BY precio ASC, (capacidad - pasajeros) DESC
    LIMIT 1;
END //

DELIMITER ;

-- ----------------------------------------------------------------------------
-- 6. CONSULTAS DE PRUEBA LISTAS PARA EJECUTAR EN MYSQL WORKBENCH
-- ----------------------------------------------------------------------------
-- SELECT * FROM rutas_buses;
-- SELECT * FROM vw_reporte_ingresos;
-- SELECT * FROM vw_reporte_ocupacion;
-- SELECT * FROM vw_resumen_general;
-- CALL sp_recomendar_ruta('Guatemala', 'Quetzaltenango', 5);