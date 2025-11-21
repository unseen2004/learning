const request = require('supertest');
const app = require('../app');
const { createTestUsers, generateUserTokens } = require('./helpers');

describe('User API', () => {
    let users;
    let tokens;

    beforeAll(async () => {
        // Create test users and generate tokens
        users = await createTestUsers();
        tokens = generateUserTokens(users);
    });

    describe('GET /api/users', () => {
        // Test case: Admin can fetch all users
        test('should allow admin to get all users', async () => {
            const res = await request(app)
                .get('/api/users')
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(res.statusCode).toBe(200);
            expect(Array.isArray(res.body)).toBe(true);
            expect(res.body.length).toBeGreaterThanOrEqual(2);
        });

        // Test case: Regular user cannot fetch all users
        test('should not allow regular user to get all users', async () => {
            const res = await request(app)
                .get('/api/users')
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(res.statusCode).toBe(403);
            expect(res.body).toHaveProperty('message', 'Access denied: Admin role required');
        });

        // Test case: Unauthenticated access is rejected
        test('should reject unauthenticated access', async () => {
            const res = await request(app)
                .get('/api/users');

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'No authentication token, access denied');
        });
    });

    describe('GET /api/users/:id', () => {
        // Test case: User can access their own details
        test('should allow user to access their own details', async () => {
            const res = await request(app)
                .get(`/api/users/${users.regularUser.id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('id', users.regularUser.id);
            expect(res.body).toHaveProperty('username', users.regularUser.username);
        });

        // Test case: Admin can access any user's details
        test('should allow admin to access any user details', async () => {
            const res = await request(app)
                .get(`/api/users/${users.regularUser.id}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('id', users.regularUser.id);
        });

        // Test case: User cannot access another user's details
        test('should not allow user to access another user details', async () => {
            const res = await request(app)
                .get(`/api/users/${users.adminUser.id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(res.statusCode).toBe(403);
        });

        // Test case: Non-existent user ID returns 404
        test('should return 404 for non-existent user', async () => {
            const res = await request(app)
                .get('/api/users/9999')
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(res.statusCode).toBe(404);
            expect(res.body).toHaveProperty('message', 'User not found');
        });
    });

    describe('PUT /api/users/:id', () => {
        // Test case: Admin can update any user's details
        test('should allow admin to update any user details', async () => {
            const res = await request(app)
                .put(`/api/users/${users.regularUser.id}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`)
                .send({
                    username: 'adminupdated',
                    role: 'admin'  // Only admin can change roles
                });

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'User updated successfully');
            expect(res.body.user).toHaveProperty('username', 'adminupdated');
            expect(res.body.user).toHaveProperty('role', 'admin');
        });

        // Test case: Validation errors
        test('should validate input data', async () => {
            const res = await request(app)
                .put(`/api/users/${users.regularUser.id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    username: 'a',  // Too short
                    email: 'invalid-email'
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });
    });

    describe('DELETE /api/users/:id', () => {
        // Test case: User can delete their own account
        test('should allow user to delete their own account', async () => {
            // Create a temporary user for deletion test
            const registerRes = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'deleteuser',
                    email: 'delete@test.com',
                    password: 'password123'
                });

            const tempToken = registerRes.body.token;
            const tempUserId = registerRes.body.user.id;

            const res = await request(app)
                .delete(`/api/users/${tempUserId}`)
                .set('Authorization', `Bearer ${tempToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'User deleted successfully');

            // Verify user is deleted
            const checkRes = await request(app)
                .get(`/api/users/${tempUserId}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(checkRes.statusCode).toBe(404);
        });

        // Test case: Admin can delete any user
        test('should allow admin to delete any user', async () => {
            // Create a temporary user for admin to delete
            const registerRes = await request(app)
                .post('/api/auth/register')
                .send({
                    username: 'admindelete',
                    email: 'admindelete@test.com',
                    password: 'password123'
                });

            const tempUserId = registerRes.body.user.id;

            const res = await request(app)
                .delete(`/api/users/${tempUserId}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'User deleted successfully');
        });

    });
});