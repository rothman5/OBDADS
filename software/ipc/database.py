import random
import sqlite3
from datetime import datetime, timedelta
from pathlib import Path
from typing import Optional

_database_path: str = ""

_IMU_COLUMNS = [
    "Timestamp",
    "ax",
    "ay",
    "az",
    "gx",
    "gy",
    "gz",
    "temp",
]

_OBD_COLUMNS = [
    "Timestamp",
    "engine_speed",
    "vehicle_speed",
    "engine_load",
    "throttle_pos",
    "mil_distance",
    "oxy_sensor1_volt",
    "oxy_sensor2_volt",
    "oxy_sensor1_curr",
    "oxy_sensor2_curr",
]


def db_init(path: str) -> None:
    if not path:
        print("No database path provided.")
        return

    global _database_path
    _database_path = path


def db_write(data: list[list[str]]) -> None:
    if not data:
        print("No data to write to the database.")
        return

    if not _database_path:
        print("No database path provided.")
        return

    for r in range(len(data)):
        timestamp = datetime.now()
        timestamp = timestamp - timedelta(milliseconds=int(data[r][0]))
        timestamp = timestamp.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        data[r][0] = timestamp

    imu_data = [row[: len(_IMU_COLUMNS)] for row in data]
    imu_headers = ", ".join(_IMU_COLUMNS)
    imu_placeholders = ", ".join(["?" for _ in range(len(_IMU_COLUMNS))])

    obd_data = [[data[r][0]] + row[len(_IMU_COLUMNS) :] for r, row in enumerate(data)]
    obd_headers = ", ".join(_OBD_COLUMNS)
    obd_placeholders = ", ".join(["?" for _ in range(len(_OBD_COLUMNS))])

    print(f"Writing {len(data)} rows and {len(data[0])} columns to the database...")
    try:
        with sqlite3.connect(_database_path) as db:
            cursor = db.cursor()

            query = f"CREATE TABLE IF NOT EXISTS IMU_DATA ({imu_headers})"
            cursor.execute(query)
            query = f"INSERT INTO IMU_DATA VALUES ({imu_placeholders})"
            cursor.executemany(query, imu_data)

            query = f"CREATE TABLE IF NOT EXISTS OBD_DATA ({obd_headers})"
            cursor.execute(query)
            query = f"INSERT INTO OBD_DATA VALUES ({obd_placeholders})"
            cursor.executemany(query, obd_data)

            db.commit()
    except sqlite3.Error as e:
        print(f"Database error: {e}")


def load_csv_data(file: Path) -> Optional[list[list[str]]]:
    if not file.exists():
        print("CSV file doesn't exist.")
        return None

    with open(file=file, mode="r", encoding="utf-8") as csv:
        data = [line.strip().split(",") for line in csv]
        return data

    return None


def generate_random_csv_data(rows: int, cols: int) -> list[list[str]]:
    return [
        [f"{random.uniform(-20, 9999):.4f}" for _ in range(cols)] for _ in range(rows)
    ]


if __name__ == "__main__":
    db_init("tmp.db")
    csv_data = load_csv_data(Path("tmp.csv"))
    if csv_data:
        db_write(csv_data)
