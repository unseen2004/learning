const request = require('supertest');
const app = require('../app');
const { createTestUsers, generateUserTokens, createTestProducts } = require('./helpers');
const { Review } = require('../models');

describe('Review API', () => {
    let users;
    let tokens;
    let products;
    let reviews;

    beforeAll(async () => {
        // Create test users and generate tokens
        users = await createTestUsers();
        tokens = generateUserTokens(users);

        // Create test products owned by the admin user
        products = await createTestProducts(users.adminUser.id);

        // Create test reviews
        reviews = [];

        // Admin user reviews a product
        const adminReview = await Review.create({
            content: 'Admin review',
            rating: 5,
            userId: users.adminUser.id,
            productId: products[0].id
        });
        reviews.push(adminReview);

        // Regular user reviews a product
        const userReview = await Review.create({
            content: 'User review',
            rating: 4,
            userId: users.regularUser.id,
            productId: products[1].id
        });
        reviews.push(userReview);
    });


    describe('GET /api/reviews/:id', () => {
        // Test case: Get review by ID
        test('should return a review by ID', async () => {
            const res = await request(app)
                .get(`/api/reviews/${reviews[0].id}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('id', reviews[0].id);
            expect(res.body).toHaveProperty('content', reviews[0].content);
            expect(res.body).toHaveProperty('rating', reviews[0].rating);
            expect(res.body).toHaveProperty('User');
            expect(res.body).toHaveProperty('Product');
        });

        // Test case: Try to get non-existent review
        test('should return 404 for non-existent review', async () => {
            const res = await request(app)
                .get('/api/reviews/9999');

            expect(res.statusCode).toBe(404);
            expect(res.body).toHaveProperty('message', 'Review not found');
        });
    });

    describe('POST /api/products/:productId/reviews', () => {
        // Test case: Create a new review
        test('should create a new review', async () => {
            const res = await request(app)
                .post(`/api/products/${products[2].id}/reviews`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: 'Great product!',
                    rating: 5
                });

            expect(res.statusCode).toBe(201);
            expect(res.body).toHaveProperty('message', 'Review created successfully');
            expect(res.body).toHaveProperty('review');
            expect(res.body.review).toHaveProperty('content', 'Great product!');
            expect(res.body.review).toHaveProperty('rating', 5);
            expect(res.body.review).toHaveProperty('userId', users.regularUser.id);
            expect(res.body.review).toHaveProperty('productId', products[2].id);
        });

        // Test case: Cannot review same product twice
        test('should not allow user to review same product twice', async () => {
            // Try to add another review for the same product
            const res = await request(app)
                .post(`/api/products/${products[2].id}/reviews`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: 'Second review should fail',
                    rating: 4
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('message', 'You already reviewed this product');
        });

        // Test case: Try to review non-existent product
        test('should return 404 when reviewing non-existent product', async () => {
            const res = await request(app)
                .post('/api/products/9999/reviews')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: 'This should fail',
                    rating: 3
                });

            expect(res.statusCode).toBe(404);
            expect(res.body).toHaveProperty('message', 'Product not found');
        });

        // Test case: Validation errors
        test('should validate review input', async () => {
            const res = await request(app)
                .post(`/api/products/${products[3].id}/reviews`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: '',  // Empty content
                    rating: 6     // Rating out of range
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });
    });

    describe('PUT /api/reviews/:id', () => {
        // Test case: Update own review
        test('should update own review', async () => {
            const res = await request(app)
                .put(`/api/reviews/${reviews[1].id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: 'Updated review content',
                    rating: 3
                });

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'Review updated successfully');
            expect(res.body.review).toHaveProperty('content', 'Updated review content');
            expect(res.body.review).toHaveProperty('rating', 3);
        });

        // Test case: Cannot update other user's review
        test('should not allow updating another user\'s review', async () => {
            const res = await request(app)
                .put(`/api/reviews/${reviews[0].id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    content: 'Trying to update admin review',
                    rating: 2
                });

            expect(res.statusCode).toBe(403);
            expect(res.body).toHaveProperty('message', 'Not authorized to update this review');
        });

        // Test case: Validation errors when updating
        test('should validate review update input', async () => {
            const res = await request(app)
                .put(`/api/reviews/${reviews[1].id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    rating: 6  // Rating out of range
                });

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });
    });

    describe('DELETE /api/reviews/:id', () => {
        let reviewToDelete;

        beforeAll(async () => {
            // Create a review that will be deleted
            reviewToDelete = await Review.create({
                content: 'Review to delete',
                rating: 3,
                userId: users.regularUser.id,
                productId: products[3].id
            });
        });

        // Test case: Delete own review
        test('should delete own review', async () => {
            const res = await request(app)
                .delete(`/api/reviews/${reviewToDelete.id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'Review deleted successfully');

            // Verify review is deleted
            const checkRes = await request(app)
                .get(`/api/reviews/${reviewToDelete.id}`);

            expect(checkRes.statusCode).toBe(404);
        });

        // Test case: Admin can delete any review
        test('should allow admin to delete any review', async () => {
            const res = await request(app)
                .delete(`/api/reviews/${reviews[1].id}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('message', 'Review deleted successfully');

            // Verify review is deleted
            const checkRes = await request(app)
                .get(`/api/reviews/${reviews[1].id}`);

            expect(checkRes.statusCode).toBe(404);
        });

        // Test case: Cannot delete other user's review
        test('should not allow deleting another user\'s review', async () => {
            const res = await request(app)
                .delete(`/api/reviews/${reviews[0].id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(res.statusCode).toBe(403);
            expect(res.body).toHaveProperty('message', 'Not authorized to delete this review');
        });
    });
});