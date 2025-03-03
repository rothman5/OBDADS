<template>
  <div class="chart-container">
    <h2>{{ title }}</h2>
    <LineChart :chart-data="chartDataFormatted" />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import { LineChart } from 'vue-chart-3';
import { Chart, registerables } from 'chart.js';

Chart.register(...registerables);

interface Props {
  title: string;
  chartData: { labels: string[]; data: number[] };
  borderColor: string;
}

const props = defineProps<Props>();

const chartDataFormatted = computed(() => ({
  labels: props.chartData.labels,
  datasets: [{ label: props.title, data: props.chartData.data, borderColor: props.borderColor }],
}));
</script>

<style scoped>
.chart-container {
  margin: 20px 0;
}
</style>
