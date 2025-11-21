const request = require('supertest');
const app = require('../app');
const { User } = require('../models');
const { Op } = require('sequelize');

describe('Authentication API', () => {
    beforeEach(async () => {
        await User.destroy({ where: { [Op.or]: [{ email: 'test@example.com' }, { username: 'testuser' }] } });
    });

    describe('POST /api/auth/register', () => {
        // Test case: Successfully register a new user
        test('should register a new user', async () => {
            const res = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'test@example.com',
                    password: 'password123'
                });

            expect(res.statusCode).toBe(201);
            expect(res.body).toHaveProperty('token');
            expect(res.body).toHaveProperty('user');
            expect(res.body.user).toHaveProperty('username', 'testuser');
            expect(res.body.user).toHaveProperty('email', 'test@example.com');
            expect(res.body.user).toHaveProperty('role', 'user');
        });

        // Test case: Try to register with missing fields
        test('should return validation error for missing fields', async () => {
            const res = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser'
                    // Missing email and password
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });

        // Test case: Try to register with an invalid email
        test('should return validation error for invalid email', async () => {
            const res = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'invalid-email',
                    password: 'password123'
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });

        // Test case: Try to register with a short password
        test('should return validation error for short password', async () => {
            const res = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'test@example.com',
                    password: '123'  // Too short
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });

        // Test case: Try to register with duplicate username/email
        test('should not allow duplicate username/email', async () => {
            // First registration
            await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'test@example.com',
                    password: 'password123'
                });

            // Second registration with same data
            const res = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'test@example.com',
                    password: 'password123'
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('message', 'User already exists');
        });
    });

    describe('POST /api/auth/login', () => {
        beforeEach(async () => {
            // Create a test user for login tests
            await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'testuser',
                    email: 'test@example.com',
                    password: 'password123'
                });
        });

        // Test case: Successfully login
        test('should login successfully with valid credentials', async () => {
            const res = await request(app)
                .post('/api/auth/login')
                .send({
                    email: 'test@example.com',
                    password: 'password123'
                });

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('token');
            expect(res.body).toHaveProperty('user');
            expect(res.body.message).toBe('Login successful');
        });

        // Test case: Failed login - wrong password
        test('should fail with wrong password', async () => {
            const res = await request(app)
                .post('/api/auth/login')
                .send({
                    email: 'test@example.com',
                    password: 'wrongpassword'
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('message', 'Invalid credentials');
        });

        // Test case: Failed login - non-existent email
        test('should fail with non-existent email', async () => {
            const res = await request(app)
                .post('/api/auth/login')
                .send({
                    email: 'nonexistent@example.com',
                    password: 'password123'
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('message', 'Invalid credentials');
        });

        // Test case: Missing required fields
        test('should return validation error for missing fields', async () => {
            const res = await request(app)
                .post('/api/auth/login')
                .send({
                    // Missing email and password
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });
    });
});