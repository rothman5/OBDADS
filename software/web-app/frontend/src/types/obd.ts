export interface OBDData {
  speed: number;
  rpm: number;
  temperature: number;
  fuel_level: number;
  timestamps?: string[];
  // Add more properties here
}

export interface ChartData {
  labels: string[];
  data: number[];
}
