"""
Run the machine learning model for the OBDADS project.
"""

import sqlite3
import time

import numpy as np
import pandas as pd
from keras._tf_keras import keras
from sklearn.preprocessing import MinMaxScaler

from common import SEQ_LENGTH, THRESHOLD, create_sequences, features, samples_in_10_minutes


def run_model():
    try:
        # Load the model
        model = keras.models.load_model('model.keras')
        print('Model loaded from model.keras')

        # Make a copy of the model
        model_copy = keras.models.clone_model(model)
        model_copy.set_weights(model.get_weights())
        print('Model copy created')

        # Connect to DB
        conn = sqlite3.connect('tmp.db')
        cursor = conn.cursor()

        # Read the last minute of data from the data table
        cursor.execute('SELECT engine_speed,vehicle_speed,throttle_pos,engine_load FROM OBD_DATA ORDER BY time DESC LIMIT ?', (samples_in_10_minutes,))

        # Select the relevant columns
        rows = cursor.fetchall()
        data = pd.DataFrame(rows, columns=features)
        data = data[features].values

        # Close the cursor and connection
        cursor.close()
        conn.close()

        # Normalize the data
        scaler = MinMaxScaler()
        data = scaler.fit_transform(data)

        # Create sequences
        data_last_minute = create_sequences(data, SEQ_LENGTH)

        # Detect anomalies
        reconstructed_data = model_copy.predict(data_last_minute)
        reconstruction_error = np.mean(np.abs(reconstructed_data - data_last_minute), axis=-1)

        # Get the reconstruction error for each feature
        avg_reconstruction_error = np.mean(reconstruction_error, axis=0)

        try:
            conn = sqlite3.connect('test.db')
            cursor = conn.cursor()
            print("Connected to SQLite")
        except sqlite3.Error as e:
            print(f"Error connecting to SQLite: {e}")

        # Create a table for the data
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS ML_DATA (
                aid INTEGER PRIMARY KEY AUTOINCREMENT,
                time TEXT NOT NULL,
                desc TEXT NOT NULL
            )
        ''')

        # Print whether there is an anomaly for each stat
        for i, feature in enumerate(features):
            if avg_reconstruction_error[i] > THRESHOLD:
                print(f'Anomaly detected in {feature} with error {avg_reconstruction_error[i]}')

                # Construct the message
                description = f'Anomaly detected in {feature} with error {avg_reconstruction_error[i]}'

                # Insert the data into the database
                cursor.execute('''
                    INSERT INTO ML_DATA (time, desc)
                    VALUES (datetime("now"), ?)
                ''', (description,))

                conn.commit()
                print(f'Inserted anomaly into database: {description}')
            else:
                print(f'No anomaly detected in {feature} with error {avg_reconstruction_error[i]}')
    except sqlite3.Error as e:
        print(f"Error connecting to SQLite: {e}")
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        if conn:
            conn.close()
        print("SQLite connection closed")
        print("Model run completed")


if __name__ == "__main__":
    # Run the model every 10 seconds
    while True:
        run_model()
        time.sleep(10)
        # Sleep for 10 seconds
