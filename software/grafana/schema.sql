DROP TABLE IF EXISTS IMU_DATA;
DROP TABLE IF EXISTS OBD_DATA;
DROP TABLE IF EXISTS ML_DATA;

PRAGMA foreign_keys = ON;

CREATE TABLE IMU_DATA (
    time TEXT NOT NULL,
    ax REAL,
    ay REAL,
    az REAL,
    gx REAL,
    gy REAL,
    gz REAL,
    temp REAL,
    PRIMARY KEY (time),
    FOREIGN KEY (time) REFERENCES OBD_DATA(time) ON DELETE CASCADE ON UPDATE CASCADE
);

CREATE TABLE OBD_DATA (
    time TEXT NOT NULL,
    engine_speed REAL,
    vehicle_speed REAL,
    engine_load REAL,
    throttle_position REAL,
    mil_distance REAL,
    oxy_sensor1_volt REAL,
    oxy_sensor1_curr REAL,
    oxy_sensor2_volt REAL,
    oxy_sensor2_curr REAL,
    PRIMARY KEY (time)
);

CREATE TABLE ML_DATA (
    aid INTEGER PRIMARY KEY AUTOINCREMENT,
    time TEXT NOT NULL,
    desc TEXT
);
