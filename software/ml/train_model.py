"""
Code to train a model for the anomaly detection task using a recurrent neural network (RNN) with LSTM layers.
"""

import sqlite3
import time

import pandas as pd
from keras._tf_keras import keras
from keras._tf_keras.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from keras._tf_keras.keras.models import Sequential
from sklearn.preprocessing import MinMaxScaler

from common import SEQ_LENGTH, create_sequences, features, samples_in_10_minutes


def train_model():
    try:
        # Define the model
        model = Sequential([
            LSTM(64, activation='relu', input_shape=(SEQ_LENGTH, len(features)), return_sequences=True),
            LSTM(32, activation='relu', return_sequences=False),
            RepeatVector(SEQ_LENGTH),
            LSTM(32, activation='relu', return_sequences=True),
            LSTM(64, activation='relu', return_sequences=True),
            TimeDistributed(Dense(len(features)))
        ])

        # Compile the model
        model.compile(optimizer='adam', loss='mse', metrics=['accuracy'])

        # Read the last 10 minutes of data from sqlite
        conn = sqlite3.connect('tmp.db')
        cursor = conn.cursor()

        # Read the last 10 minutes of data from the data table
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
        X_train = create_sequences(data, SEQ_LENGTH)

        # Train the model
        early_stopping = keras.callbacks.EarlyStopping(patience=2, restore_best_weights=True)
        model.fit(X_train, X_train, epochs=100, batch_size=64, validation_split=0.1, callbacks=[early_stopping])

        # Save the model
        model.save('model.keras')
        print('Model saved to model.keras')
    except sqlite3.Error as e:
        print(f"Error connecting to SQLite: {e}")
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        if conn:
            conn.close()
        print("SQLite connection closed")
        print("Model training completed")


if __name__ == "__main__":
    # Run a training session every 10 minutes
    while True:
        train_model()
        time.sleep(600)  # Sleep for 10 minutes
