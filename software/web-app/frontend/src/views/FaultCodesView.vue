<template>
  <div class="fault-codes">
    <h1>Fault Codes</h1>

    <table>
      <thead>
        <tr>
          <th>Status Code</th>
          <th>Severity</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="code in faultCodes" :key="code.code">
          <td>{{ code.code }}</td>
          <td>
            <span :class="getSeverityClass(code.severity)">
              {{ code.severity }}
            </span>
          </td>
          <td>
            <button @click="toggleDescription(code.code)">
              {{ expandedCode === code.code ? 'Hide' : 'Show' }} Details
            </button>
            <p v-if="expandedCode === code.code" class="description">
              {{ code.description }}
            </p>
          </td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';

// Define fault codes with status, severity, and descriptions
const faultCodes = ref([
  { code: 'P0301', severity: 'High', description: 'Cylinder 1 Misfire Detected' },
  { code: 'P0420', severity: 'Medium', description: 'Catalyst System Efficiency Below Threshold' },
  { code: 'P0171', severity: 'Low', description: 'System Too Lean (Bank 1)' },
  {
    code: 'P0507',
    severity: 'Medium',
    description: 'Idle Control System RPM Higher Than Expected',
  },
]);

const expandedCode = ref<string | null>(null);

const toggleDescription = (code: string) => {
  expandedCode.value = expandedCode.value === code ? null : code;
};

const getSeverityClass = (severity: string) => {
  return {
    'high-severity': severity === 'High',
    'medium-severity': severity === 'Medium',
    'low-severity': severity === 'Low',
  };
};

// Simulated API call to fetch fault codes
// const fetchFaultCodes = async () => {
//   try {
//     const response = await axios.get('http://localhost:PORT/api/obd/fault-codes/');
//     faultCodes.value = response.data;
//   } catch (error) {
//     console.error('Error fetching fault codes:', error);
//   }
// };

// fetchFaultCodes();
</script>

<style scoped>
.fault-codes {
  padding: 20px;
  text-align: center;
}

table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 20px;
}

th,
td {
  padding: 12px;
  border: 1px solid #ddd;
}

button {
  cursor: pointer;
  padding: 5px 10px;
  border: none;
  background-color: #007bff;
  color: white;
  border-radius: 5px;
}

button:hover {
  background-color: #0056b3;
}

.description {
  margin-top: 5px;
  font-style: italic;
  color: #666;
}

.high-severity {
  color: red;
  font-weight: bold;
}

.medium-severity {
  color: orange;
}

.low-severity {
  color: green;
}
</style>
