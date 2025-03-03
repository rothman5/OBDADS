<template>
  <div id="app">
    <div>
      <header>
        <h1>System Information</h1>
      </header>

      <nav class="nav-header">
        <RouterLink to="/dashboard" class="nav-link">Dashboard</RouterLink>
        <RouterLink to="/fault-codes" class="nav-link">Fault-Codes</RouterLink>
      </nav>
    </div>

    <main class="content">
      <RouterView />
    </main>
  </div>
</template>

<style scoped>
html,
body,
#app {
  width: 100vw;
  height: 100vh;
  margin: 0;
  padding: 0;
}

#app {
  text-align: center;
  height: 100%;
  overflow: auto;
  text-align: center;
  display: flex;
  flex-direction: column;
}

header {
  padding: 20px;
  border-bottom: 2px solid #ddd;
}

.logo {
  display: block;
  margin: 0 auto 10px;
}

.nav-header {
  display: flex;
  justify-content: center;
  padding: 10px 0;
}

.nav-link {
  color: white;
  text-decoration: none;
  font-size: 1.2rem;
  font-weight: bold;
  padding: 10px 20px;
}

.nav-link:hover {
  background-color: rgba(255, 255, 255, 0.2);
}

.content {
  padding: 20px;
  width: 100%;
}
</style>
