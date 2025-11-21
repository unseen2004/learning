const app = require('./app');
const { syncDatabase } = require('./models');
require('dotenv').config();

const PORT = process.env.PORT || 3000;

// Sync database models
syncDatabase().then(() => {
  // Start the server
  app.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
  });
});