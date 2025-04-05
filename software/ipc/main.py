import os
import subprocess
import sys
import time
from typing import Optional

import serial
from dotenv import load_dotenv

from database import db_init
from ipc import ipc_init, ipc_listen

_ENV_FILE = ".env"


def get_env_variable(key: str) -> Optional[str]:
    return os.getenv(key)


def set_env_variable(key: str, value) -> None:
    lines = []
    found = False

    if os.path.exists(_ENV_FILE):
        with open(_ENV_FILE, "r") as file:
            lines = file.readlines()

    with open(_ENV_FILE, "w") as file:
        for line in lines:
            if line.startswith(f"{key}="):
                file.write(f"{key}={value}\n")
                found = True
            else:
                file.write(line)
        if not found:
            file.write(f"{key}={value}\n")

    print(f"Set {key}={value} in {_ENV_FILE}")


def run_shell_command(command) -> None:
    try:
        subprocess.run(command, shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
    else:
        time.sleep(1)


def check_env_variables() -> tuple[str, str, str]:
    firmware_path = get_env_variable("FIRMWARE_PATH")
    script_path = get_env_variable("SCRIPT_PATH")
    database_path = get_env_variable("DATABASE_PATH")

    if not firmware_path or not script_path or not database_path:
        print("Error: Firmware, script and DB paths must be set before using commands.")
        print("Run the following commands to set them:")
        print("> python main.py firmware <path/to/firmware.elf>")
        print("> python main.py script <path/to/script.sh>")
        print("> python main.py database <path/to/database.db>")
        sys.exit(1)

    return firmware_path, script_path, database_path


def main() -> None:
    if len(sys.argv) < 2:
        use_msg = "Usage: python main.py [state | start <port> | stop | firmware <path> | script <path> | database <path>]"
        print(use_msg)
        sys.exit(1)

    load_dotenv()

    command = sys.argv[1]
    if command == "firmware":
        if len(sys.argv) < 3:
            print("Usage: python main.py firmware <path>")
            sys.exit(1)
        firmware_path = sys.argv[2]
        set_env_variable("FIRMWARE_PATH", firmware_path)

    elif command == "script":
        if len(sys.argv) < 3:
            print("Usage: python main.py script <path>")
            sys.exit(1)
        script_path = sys.argv[2]
        set_env_variable("SCRIPT_PATH", script_path)

    elif command == "database":
        if len(sys.argv) < 3:
            print("Usage: python main.py database <path>")
            sys.exit(1)
        database_path = sys.argv[2]
        set_env_variable("DATABASE_PATH", database_path)

    elif command in ["state", "start", "stop"]:
        firmware_path, script_path, database_path = check_env_variables()

        if command == "state":
            run_shell_command("cat /sys/class/remoteproc/remoteproc0/state")

        elif command == "start":
            if len(sys.argv) < 3:
                print("Usage: python main.py start <port>")
                sys.exit(1)
            serial_port = sys.argv[2]

            run_shell_command(f"sh {firmware_path} start")
            try:
                db_init(path=database_path)
                ipc_init(port=serial_port)
            except serial.SerialException as e:
                print(f"Error: Failed to initialize IPC: {e}")
                sys.exit(1)
            else:
                try:
                    ipc_listen()
                except KeyboardInterrupt:
                    print("Stopping IPC listener...")
                    run_shell_command(f"sh {firmware_path} stop")

        elif command == "stop":
            run_shell_command(f"sh {firmware_path} stop")

        else:
            print("Invalid command.")
            sys.exit(1)
    else:
        print("Invalid command.")
        sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        run_shell_command("echo stop > /sys/class/remoteproc/remoteproc0/state")
    finally:
        sys.exit(0)
