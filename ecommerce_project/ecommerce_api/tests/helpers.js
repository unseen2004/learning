const request = require('supertest');
const app = require('../app');
const { User, Product } = require('../models');
const jwt = require('jsonwebtoken');

// Create test users
async function createTestUsers() {
    const adminUser = await User.create({
        username: 'admin',
        email: 'admin@test.com',
        password: 'password123',
        role: 'admin'
    });

    const regularUser = await User.create({
        username: 'user',
        email: 'user@test.com',
        password: 'password123',
        role: 'user'
    });

    return { adminUser, regularUser };
}

// Generate JWT tokens for test users
function generateUserTokens(users) {
    const adminToken = jwt.sign(
        { id: users.adminUser.id, role: users.adminUser.role },
        process.env.JWT_SECRET,
        { expiresIn: process.env.JWT_EXPIRES_IN }
    );

    const userToken = jwt.sign(
        { id: users.regularUser.id, role: users.regularUser.role },
        process.env.JWT_SECRET,
        { expiresIn: process.env.JWT_EXPIRES_IN }
    );

    return { adminToken, userToken };
}

// Create test products
async function createTestProducts(userId) {
    const products = [];

    for (let i = 1; i <= 5; i++) {
        const product = await Product.create({
            name: `Test Product ${i}`,
            description: `Description for test product ${i}`,
            price: 10.99 * i,
            stock: 10 * i,
            category: i % 2 === 0 ? 'electronics' : 'clothing',
            userId
        });

        products.push(product);
    }

    return products;
}

module.exports = {
    createTestUsers,
    generateUserTokens,
    createTestProducts
};