<template>
  <div class="dashboard">
    <h1>OBD-II Dashboard</h1>

    <div class="content-container">
      <!-- Live Data (Prominent, Left) -->
      <div class="live-data-box">
        <h2>Live Data</h2>
        <!-- Display Error Code -->
        <p v-if="liveData.errors">
          <strong id="error">Error: {{ liveData.errors }}</strong>
        </p>
        <!-- Display Live Data -->
        <p><strong>Speed:</strong> {{ liveData.speed }} km/h</p>
        <p><strong>RPM:</strong> {{ liveData.rpm }}</p>
        <p><strong>Temperature:</strong> {{ liveData.temperature }} °C</p>
        <p><strong>Fuel Level:</strong> {{ liveData.fuel_level }} %</p>
      </div>

      <!-- Toggle Button -->
      <button @click="showHistory = !showHistory" class="toggle-btn">
        {{ showHistory ? 'Hide' : 'Show' }} Historical Data
      </button>
      <!-- Historical Data (Right) -->
      <div :class="['historical-data', { show: showHistory }]">
        <h2>Historical Data</h2>
        <div class="data-section" v-for="(chart, key) in chartData" :key="key">
          <DataChart :title="getTitle(key)" :chart-data="chart" :border-color="getColor(key)" />

          <table class="history-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>{{ getTitle(key) }}</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(value, index) in chart.data" :key="index">
                <td>{{ chart.labels[index] }}</td>
                <td>{{ value }}</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import type { OBDData, ChartData } from '../types/obd';
import DataChart from '../components/DataChart.vue';
// import axios from 'axios';

const liveData = ref<OBDData>({
  speed: 0,
  rpm: 0,
  temperature: 0,
  fuel_level: 0,
  errors: [],
});

const chartData = ref<Record<string, ChartData>>({
  speed: { labels: [], data: [] },
  rpm: { labels: [], data: [] },
  temperature: { labels: [], data: [] },
  fuel_level: { labels: [], data: [] },
});

const showHistory = ref(true);

const getTitle = (key: string): string => {
  const titles: Record<string, string> = {
    speed: 'Speed (km/h)',
    rpm: 'RPM',
    temperature: 'Temperature (°C)',
    fuel_level: 'Fuel Level (%)',
  };
  return titles[key] || key;
};

const getColor = (key: string): string => {
  const colors: Record<string, string> = {
    speed: 'blue',
    rpm: 'red',
    temperature: 'orange',
    fuel_level: 'green',
  };
  return colors[key] || 'black';
};

// Mock Data for Development
onMounted(() => {
  liveData.value = {
    speed: 60,
    rpm: 2000,
    temperature: 80,
    fuel_level: 50,
    errors: [{ code: 'P0301', severity: 'high', description: 'Cylinder 1 Misfire Detected' }],
  };

  chartData.value = {
    speed: { labels: ['12:00', '12:01', '12:02'], data: [60, 62, 64] },
    rpm: { labels: ['12:00', '12:01', '12:02'], data: [2000, 2100, 1000] },
    temperature: { labels: ['12:00', '12:01', '12:02'], data: [80, 81, 82] },
    fuel_level: { labels: ['12:00', '12:01', '12:02'], data: [50, 60, 48] },
  };

  // TODO: Set up API call to fetch live data
  // const fetchData = async (): Promise<void> => {
  //   try {
  //     const response = await axios.get<OBDData>('http://localhost:8000/api/obd/live-data/');
  //     const data = response.data;
  //     liveData.value = data;

  //     chartData.value = {
  //       speed: { labels: data.timestamps || [], data: [data.speed] },
  //       rpm: { labels: data.timestamps || [], data: [data.rpm] },
  //       temperature: { labels: data.timestamps || [], data: [data.temperature] },
  //       fuel_level: { labels: data.timestamps || [], data: [data.fuel_level] },
  //     };
  //   } catch (error) {
  //     console.error('Error fetching data:', error);
  //   }
  // };

  // We would be fetching our data from the API here on a timeout
  // setInterval(() => {
  //   fetchData().then(() => {
  //     console.log("Data Updated");
  //   });
  // }, 5000);
});
</script>

<style scoped>
/* Base Styles */
.dashboard {
  padding: 20px;
  text-align: center;
  width: 100%;
}

/* Flexbox container for Live and Historical Data */
.content-container {
  width: 100%;
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-direction: column;
  gap: 20px;
}

/* Live Data Box */
.live-data-box {
  width: 30%;
  padding: 20px;
  border: 2px solid #ddd;
  border-radius: 10px;
  background-color: #222;
  font-size: 1.4rem;
  text-align: left;
  color: white;
}

/* Historical Data Section */
.historical-data {
  width: 65%;
}

/* Flex container for charts and tables */
.data-section {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  gap: 20px;
}

/* Table styling */
.history-table {
  border-collapse: collapse;
  width: 40%;
  min-width: 250px;
  text-align: left;
}

.history-table th,
.history-table td {
  padding: 8px;
  border: 1px solid #ddd;
}

/* Toggle Button */
.toggle-btn {
  margin-top: 20px;
  padding: 10px 15px;
  font-size: 1rem;
  cursor: pointer;
  background-color: #007bff;
  color: white;
  border: none;
  border-radius: 5px;
}

.toggle-btn:hover {
  background-color: #0056b3;
}

#error {
  color: red;
}

/* Responsive Design - Mobile Breakpoint for the LCD */
@media (max-width: 768px) {
  .content-container {
    flex-direction: column;
    align-items: center;
  }

  .live-data-box {
    width: 90%;
    text-align: center;
  }

  .historical-data {
    display: none;
    width: 100%;
  }

  .data-section {
    flex-direction: column;
    gap: 10px;
  }

  .history-table {
    display: none;
    width: 100%;
  }

  /* Show historical data when the button is clicked */
  .historical-data.show {
    display: block;
  }
}
</style>
