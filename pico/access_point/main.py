import network
import time

# Configure AP
ap = network.WLAN(network.AP_IF)
ap.config(ssid='MyPicoAP', password='my_password')
ap.active(True)

# Wait for AP to become active
while not ap.active():
    time.sleep(1)

print("AP Mode Active. IP Address:", ap.ifconfig()[0])
