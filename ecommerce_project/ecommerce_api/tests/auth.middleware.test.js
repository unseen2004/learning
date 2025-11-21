const request = require('supertest');
const app = require('../app');
const jwt = require('jsonwebtoken');
const { createTestUsers } = require('./helpers');

describe('Auth Middleware', () => {
    let users;

    beforeAll(async () => {
        users = await createTestUsers();
    });

    describe('authenticate middleware', () => {
        // Test case: Missing token
        test('should reject requests with no token', async () => {
            const res = await request(app)
                .get('/api/users');

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'No authentication token, access denied');
        });

        // Test case: Invalid token
        test('should reject requests with invalid token', async () => {
            const res = await request(app)
                .get('/api/users')
                .set('Authorization', 'Bearer invalidtokenstring');

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'Invalid token');
        });

        // Test case: Expired token
        test('should reject requests with expired token', async () => {
            // Create a token that's already expired
            const expiredToken = jwt.sign(
                { id: users.adminUser.id, role: users.adminUser.role },
                process.env.JWT_SECRET,
                { expiresIn: '0s' }  // Expires immediately
            );

            // Wait a moment to ensure token expires
            await new Promise(resolve => setTimeout(resolve, 1000));

            const res = await request(app)
                .get('/api/users')
                .set('Authorization', `Bearer ${expiredToken}`);

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'Token expired');
        });

        // Test case: Non-existent user in token
        test('should reject token with non-existent user ID', async () => {
            const invalidUserToken = jwt.sign(
                { id: 9999, role: 'user' },  // Non-existent user ID
                process.env.JWT_SECRET,
                { expiresIn: '1h' }
            );

            const res = await request(app)
                .get('/api/users/1')
                .set('Authorization', `Bearer ${invalidUserToken}`);

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'Invalid token');
        });
    });

    describe('isAdmin middleware', () => {
        // Test case: Non-admin accessing admin route
        test('should reject non-admin users from admin routes', async () => {
            const userToken = jwt.sign(
                { id: users.regularUser.id, role: 'user' },
                process.env.JWT_SECRET,
                { expiresIn: '1h' }
            );

            const res = await request(app)
                .get('/api/users')  // Admin-only route
                .set('Authorization', `Bearer ${userToken}`);

            expect(res.statusCode).toBe(403);
            expect(res.body).toHaveProperty('message', 'Access denied: Admin role required');
        });

        // Test case: Admin accessing admin route
        test('should allow admin to access admin routes', async () => {
            const adminToken = jwt.sign(
                { id: users.adminUser.id, role: 'admin' },
                process.env.JWT_SECRET,
                { expiresIn: '1h' }
            );

            const res = await request(app)
                .get('/api/users')  // Admin-only route
                .set('Authorization', `Bearer ${adminToken}`);

            expect(res.statusCode).toBe(200);
            expect(Array.isArray(res.body)).toBe(true);
        });
    });

    describe('paginationFilter middleware', () => {
        // Test case: Default pagination
        test('should apply default pagination when not specified', async () => {
            const res = await request(app)
                .get('/api/products');

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('pagination');
            expect(res.body.pagination).toHaveProperty('currentPage', 1);
            expect(res.body.pagination).toHaveProperty('itemsPerPage', 10);
        });

        // Test case: Custom pagination
        test('should apply custom pagination when specified', async () => {
            const res = await request(app)
                .get('/api/products?page=2&limit=3');

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('pagination');
            expect(res.body.pagination).toHaveProperty('currentPage', 2);
            expect(res.body.pagination).toHaveProperty('itemsPerPage', 3);
        });

        // Test case: Default sorting
        test('should apply default sorting when not specified', async () => {
            const res = await request(app)
                .get('/api/products');

            expect(res.statusCode).toBe(200);

            // Default sorting should be by createdAt DESC (newest first)
            const dates = res.body.products.map(p => new Date(p.createdAt).getTime());
            const sortedDates = [...dates].sort((a, b) => b - a);  // Descending order
            expect(dates).toEqual(sortedDates);
        });

        // Test case: Custom sorting
        test('should apply custom sorting when specified', async () => {
            const res = await request(app)
                .get('/api/products?sort=name');

            expect(res.statusCode).toBe(200);

            // Products should be sorted alphabetically by name
            const names = res.body.products.map(p => p.name);
            const sortedNames = [...names].sort();
            expect(names).toEqual(sortedNames);
        });
    });
});