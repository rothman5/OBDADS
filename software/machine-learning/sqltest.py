"""
Testing SQLite
"""

import sqlite3
from sqlite3 import Error
import os


# Open a connection to the SQLite database
conn = sqlite3.connect('test.db')

# Create a cursor object
cursor = conn.cursor()


# Read all data in the ml table
cursor.execute('SELECT * FROM ml')
rows = cursor.fetchall()
# Print the data
for row in rows:
    print(row)
# Close the cursor and connection
cursor.close()
conn.close()
