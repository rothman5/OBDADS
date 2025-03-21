import subprocess
import sys
import time

import serial

IPC_ID = "OBDADS-IPC"
IPC_PORT = "/dev/ttyRPMSG0"
DELAY_MS = 500
BATCH_SIZE = 100
REMOTE_PROC_PATH = "/sys/class/remoteproc/remoteproc0/state"
CONTROL_COMMAND_START = [f"echo start > {REMOTE_PROC_PATH}"]
CONTROL_COMMAND_STOP = [f"echo stop > {REMOTE_PROC_PATH}"]
CONTROL_COMMAND_STATE = ["cat", REMOTE_PROC_PATH]


csv_buffer: list[list[str]] = []


def fw_state() -> str:
    return subprocess.check_output(CONTROL_COMMAND_STATE).decode("utf-8").strip()


def stop_firmware() -> bool:
    if fw_state() == "offline":
        return True

    subprocess.run(CONTROL_COMMAND_STOP, shell=True, check=True)
    time.sleep(DELAY_MS / 1000)

    return True if fw_state() == "offline" else False


def start_firmware() -> bool:
    subprocess.run(CONTROL_COMMAND_START, shell=True, check=True)
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
        csv_buffer.clear()


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
    try:
        if not stop_firmware():
            print("Failed to stop firmware!")
            sys.exit(1)
        if not start_firmware():
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
        stop_firmware()
        sys.exit(1)


if __name__ == "__main__":
    main()
