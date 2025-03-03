export interface OBDData {
  speed: number;
  rpm: number;
  temperature: number;
  fuel_level: number;
  timestamps?: string[];
  errors: ErrorCode[];
  // Add more properties here
}

interface ErrorCode {
  code: string;
  severity: 'low' | 'medium' | 'high';
  description: string;
}

export interface ChartData {
  labels: string[];
  data: number[];
}
