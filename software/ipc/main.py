import subprocess
import sys
import time
from pathlib import Path

import serial

IPC_ID = "OBDADS-IPC"
IPC_PORT = "/dev/ttyRPMSG0"
DELAY_MS = 500
BATCH_SIZE = 100
REMOTEPROC_DIR = "/sys/class/remoteproc/remoteproc0"
CONTROL_COMMAND_START = [f"echo start > {REMOTEPROC_DIR}/state"]
CONTROL_COMMAND_STOP = [f"echo stop > {REMOTEPROC_DIR}/state"]
CONTROL_COMMAND_STATE = ["cat", f"{REMOTEPROC_DIR}/state"]
COLUMNS = [
    "Ax (g)",
    "Ay (g)",
    "Az (g)",
    "Gx (dps)",
    "Gy (dps)",
    "Gz (dps)",
    "Temp (degC)",
    "engine_speed (rpm)",
    "vehicle_speed (km/h)",
    "engine_load (%)",
    "throttle_position (%)",
    "distance_traveled_mil (km)",
    "o2_volt1 (V)",
    "o2_volt2 (V)",
    "o2_curr1 (A)",
    "o2_curr2 (A)",
]


csv_buffer: list[list[str]] = []


def fw_state() -> str:
    return subprocess.check_output(CONTROL_COMMAND_STATE).decode("utf-8").strip()


def load_firmware(fw: Path) -> None:
    subprocess.run([f"echo {fw} > {REMOTEPROC_DIR}/firmware"], shell=True, check=True)
    time.sleep(DELAY_MS / 1000)


def stop_firmware(script: Path) -> bool:
    if fw_state() == "offline":
        return True

    subprocess.run([f"{script} stop"], shell=True, check=True)
    time.sleep(DELAY_MS / 1000)

    return True if fw_state() == "offline" else False


def start_firmware(script: Path) -> bool:
    subprocess.run([f"{script} start"], shell=True, check=True)
    time.sleep(DELAY_MS / 1000)

    return True if fw_state() == "running" else False


def initiate_ipc() -> bool:
    try:
        ipc = serial.Serial(port=IPC_PORT, baudrate=115200, timeout=1)
    except serial.SerialException as e:
        print(f"Error: Failed to send initialization message: {e}")
        return False
    else:
        wr = ipc.write(f"{IPC_ID}\r\n".encode("utf-8"))
        rd = ipc.readline().decode("utf-8").strip()
        print(f"tx ({wr}): {IPC_ID}, rx ({len(rd)}): {rd}")
        if rd != IPC_ID:
            print("Error: Cortex M4 firmware failed to initialize IPC!")
            ipc.close()
            return False
        ipc.close()
    return True


def process_line(line: str) -> None:
    components = line.split(",")
    if len(components) < 1:
        print("Error: Invalid IPC message received!")
        return

    csv_buffer.append(components)
    if len(csv_buffer) >= BATCH_SIZE:
        print(f"Batch size: {len(csv_buffer)}, each batch {len(csv_buffer[0])} columns")
        # TODO: Add to database here
        exists = Path("tmp.csv").exists()
        with open(file="tmp.csv", mode="a", encoding="utf-8") as csv:
            if not exists:
                csv.write(", ".join(COLUMNS) + "\n")
            else:
                for row in csv_buffer:
                    csv.write(", ".join(row) + "\n")
        csv_buffer.clear()
    else:
        print(f"Batched [{len(csv_buffer)} / {BATCH_SIZE}]")


def listen_ipc() -> None:
    while True:
        try:
            ipc = serial.Serial(port=IPC_PORT, baudrate=115200, timeout=1)
        except serial.SerialException as e:
            print(f"Error: Failed to begin listening for IPC messages: {e}")
            time.sleep(DELAY_MS / 1000)
        else:
            print(f"Now listening for IPC messages from {ipc.port}...")
            while True:
                try:
                    message = ipc.readline().decode("utf-8").strip()
                    if not message:
                        continue
                except serial.SerialException as e:
                    print(f"Error: Failed to read IPC message: {e}")
                    ipc.close()
                    break
                else:
                    process_line(message)


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python main.py <firmware>")
        sys.exit(1)

    firmware = Path(sys.argv[1]).resolve()
    if not firmware.exists():
        print(f"Error: Firmware file {firmware} does not exist!")
        sys.exit(1)

    script = firmware.parent.parent.parent / "fw_cortex_m4.sh"

    try:
        load_firmware(firmware)
        if not stop_firmware(script):
            print("Failed to stop firmware!")
            sys.exit(1)
        if not start_firmware(script):
            print("Failed to start firmware!")
            sys.exit(1)
        if not initiate_ipc():
            print("Failed to initialize IPC!")
            sys.exit(1)
        listen_ipc()
    except KeyboardInterrupt:
        print("Keyboard interrupt detected!")
    finally:
        print("Stopping firmware...")
        stop_firmware(script)
        sys.exit(1)


if __name__ == "__main__":
    main()
