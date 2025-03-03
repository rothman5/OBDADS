import { createRouter, createWebHistory } from 'vue-router';
import DashboardView from '@/views/DashboardView.vue';
import FaultCodesView from '@/views/FaultCodesView.vue';

const routes = [
  {
    path: '/',
    redirect: '/dashboard', // Redirect root to /dashboard
  },
  {
    path: '/dashboard',
    component: DashboardView,
  },
  {
    path: '/fault-codes',
    component: FaultCodesView,
  },
];

const router = createRouter({
  history: createWebHistory(),
  routes,
});

export default router;
