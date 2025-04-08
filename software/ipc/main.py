import sys

import serial
from database import db_init
from ipc import ipc_init, ipc_listen


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: python main.py <port> <database>")
        sys.exit(1)

    port = sys.argv[1]
    db_path = sys.argv[2]

    try:
        db_init(path=db_path)
        ipc_init(port=port)
    except serial.SerialException as e:
        print(f"Error: Failed to initialize IPC: {e}")
        sys.exit(1)
    else:
        try:
            ipc_listen()
        except KeyboardInterrupt:
            print("Stopping IPC listener...")
        except Exception as e:
            raise Exception from e


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        sys.exit(0)
