const sequelize = require('../config/db.config');
const { User, Product, Review } = require('../models');

beforeAll(async () => {
    // Use a test database or clear existing one
    await sequelize.sync({ force: true });
});

afterAll(async () => {
    await sequelize.close();
});