import time

import serial
from database import db_write

_ID = "OBDADS-IPC"
_SAMPLE_RATE = 1 / 50  # 1 sample / 50 ms
_BATCH_SIZE = int(1000 * _SAMPLE_RATE)  # 1000 ms * 1 sample / 50 ms  = 20 samples


_ipc: serial.Serial = serial.Serial()
_csv: list[list[str]] = []


def ipc_init(port: str = "", baudrate: int = 115_200, timeout: int = 1) -> None:
    try:
        _ipc.port = port
        _ipc.baudrate = baudrate
        _ipc.timeout = timeout
        _ipc.open()
    except serial.SerialException as e:
        print(f"Error: Failed to initialize IPC: {e}")
        raise serial.SerialException from e


def ipc_listen() -> None:
    if not _ipc.is_open:
        print("Error: IPC has not been initialized!")
        return

    while True:
        if not _ipc.is_open:
            try:
                _ipc.open()
            except serial.SerialException as e:
                print(f"Error: Failed to open IPC port: {e}")
                time.sleep(1)
                continue
            else:
                print(f"Now listening for IPC messages from {_ipc.port}...")

        while True:
            try:
                msg = _ipc.readline()
                if not msg:
                    continue
                print(msg)
                msg = msg.decode("utf-8").strip()
                print(msg)
            except serial.SerialException as e:
                print(f"Error: Failed to read IPC message: {e}")
                _ipc.close()
                break
            except Exception as e:
                print(f"Error: Failed to read IPC message: {e}")
                continue
            else:
                _ipc_process_line(msg)


def _ipc_process_line(line: str, batch_size: int = _BATCH_SIZE) -> None:
    components = line.split(",")
    if len(components) < 1:
        print("Error: Invalid IPC message received!")
        return

    _csv.append(components)
    if len(_csv) >= batch_size:
        db_write(data=_csv)
        print(f"Wrote batch [size: {len(_csv)}, each batch {len(_csv[0])} columns]")
        _csv.clear()
    else:
        print(f"Batched [{len(_csv)} / {batch_size}]")
