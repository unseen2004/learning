const request = require('supertest');
const app = require('../app');
const { createTestUsers, generateUserTokens, createTestProducts } = require('./helpers');

describe('Product API', () => {
    let users;
    let tokens;
    let products;

    beforeAll(async () => {
        // Create test users and generate tokens
        users = await createTestUsers();
        tokens = generateUserTokens(users);

        // Create test products owned by the admin user
        products = await createTestProducts(users.adminUser.id);
    });

    describe('GET /api/products', () => {
        // Test case: Get all products without authentication
        test('should return all products for public access', async () => {
            const res = await request(app)
                .get('/api/products');

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('products');
            expect(res.body).toHaveProperty('pagination');
            expect(res.body.products.length).toBeGreaterThanOrEqual(5);
        });

        // Test case: Get products with pagination
        test('should return paginated products', async () => {
            const res = await request(app)
                .get('/api/products?page=1&limit=2');

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('products');
            expect(res.body).toHaveProperty('pagination');
            expect(res.body.products.length).toBe(2);
            expect(res.body.pagination).toHaveProperty('currentPage', 1);
            expect(res.body.pagination).toHaveProperty('itemsPerPage', 2);
        });

        // Test case: Filter products by category
        test('should filter products by category', async () => {
            const res = await request(app)
                .get('/api/products?category=electronics');

            expect(res.statusCode).toBe(200);
            expect(res.body.products.length).toBeGreaterThanOrEqual(1);
            expect(res.body.products.every(p => p.category === 'electronics')).toBe(true);
        });

        // Test case: Filter products by price range
        test('should filter products by price range', async () => {
            const minPrice = 20;
            const maxPrice = 40;

            const res = await request(app)
                .get(`/api/products?minPrice=${minPrice}&maxPrice=${maxPrice}`);

            expect(res.statusCode).toBe(200);
            expect(res.body.products.length).toBeGreaterThanOrEqual(1);
            expect(res.body.products.every(p =>
                parseFloat(p.price) >= minPrice && parseFloat(p.price) <= maxPrice
            )).toBe(true);
        });

        // Test case: Sort products by price ascending
        test('should sort products by price ascending', async () => {
            const res = await request(app)
                .get('/api/products?sort=price');

            expect(res.statusCode).toBe(200);

            // Check if products are sorted by price in ascending order
            const prices = res.body.products.map(p => parseFloat(p.price));
            const sortedPrices = [...prices].sort((a, b) => a - b);
            expect(prices).toEqual(sortedPrices);
        });

        // Test case: Sort products by price descending
        test('should sort products by price descending', async () => {
            const res = await request(app)
                .get('/api/products?sort=-price');

            expect(res.statusCode).toBe(200);

            // Check if products are sorted by price in descending order
            const prices = res.body.products.map(p => parseFloat(p.price));
            const sortedPrices = [...prices].sort((a, b) => b - a);
            expect(prices).toEqual(sortedPrices);
        });

        // Test case: Search products by name
        test('should search products by name', async () => {
            const res = await request(app)
                .get('/api/products?search=Test Product 1');

            expect(res.statusCode).toBe(200);
            expect(res.body.products.length).toBeGreaterThanOrEqual(1);
            expect(res.body.products[0].name).toContain('Test Product 1');
        });
    });

    describe('GET /api/products/:id', () => {
        // Test case: Get a single product by ID
        test('should return a single product by ID', async () => {
            const res = await request(app)
                .get(`/api/products/${products[0].id}`);

            expect(res.statusCode).toBe(200);
            expect(res.body).toHaveProperty('id', products[0].id);
            expect(res.body).toHaveProperty('name', products[0].name);
        });

        // Test case: Try to get a non-existent product
        test('should return 404 for non-existent product', async () => {
            const res = await request(app)
                .get('/api/products/9999');

            expect(res.statusCode).toBe(404);
            expect(res.body).toHaveProperty('message', 'Product not found');
        });
    });

    describe('POST /api/products', () => {
        // Test case: Create a new product
        test('should create a new product', async () => {
            const newProduct = {
                name: 'New Test Product',
                description: 'This is a new test product',
                price: 29.99,
                stock: 15,
                category: 'electronics'
            };

            const res = await request(app)
                .post('/api/products')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send(newProduct);

            expect(res.statusCode).toBe(201);
            expect(res.body).toHaveProperty('message', 'Product created successfully');
            expect(res.body).toHaveProperty('product');
            expect(res.body.product).toHaveProperty('name', newProduct.name);
            expect(res.body.product).toHaveProperty('price', newProduct.price.toString());
            expect(res.body.product).toHaveProperty('userId', users.regularUser.id);
        });

        // Test case: Try to create product without authentication
        test('should not create product without authentication', async () => {
            const newProduct = {
                name: 'Another Test Product',
                description: 'This is another test product',
                price: 39.99,
                stock: 20,
                category: 'clothing'
            };

            const res = await request(app)
                .post('/api/products')
                .send(newProduct);

            expect(res.statusCode).toBe(401);
            expect(res.body).toHaveProperty('message', 'No authentication token, access denied');
        });

        // Test case: Validation errors when creating product
        test('should validate product input', async () => {
            const invalidProduct = {
                name: '', // Empty name
                description: 'Invalid product',
                price: -10, // Negative price
                stock: -5,  // Negative stock
                // Missing category
            };

            const res = await request(app)
                .post('/api/products')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send(invalidProduct);

            expect(res.statusCode).toBe(400);
            expect(res.body).toHaveProperty('errors');
        });
    });

    describe('PUT /api/products/:id', () => {
        // Test case: Update product by owner
        test('should update product by owner', async () => {
            // First create a product owned by regularUser
            const createRes = await request(app)
                .post('/api/products')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    name: 'Product to Update',
                    description: 'This will be updated',
                    price: 49.99,
                    stock: 25,
                    category: 'electronics'
                });

            const productId = createRes.body.product.id;

            // Now update the product
            const updateRes = await request(app)
                .put(`/api/products/${productId}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    name: 'Updated Product',
                    price: 59.99
                });

            expect(updateRes.statusCode).toBe(200);
            expect(updateRes.body).toHaveProperty('message', 'Product updated successfully');
            expect(updateRes.body.product).toHaveProperty('name', 'Updated Product');
            expect(updateRes.body.product).toHaveProperty('price', 59.99);
            // Description should remain unchanged
            expect(updateRes.body.product).toHaveProperty('description', 'This will be updated');
        });

        // Test case: Admin can update any product
        test('should allow admin to update any product', async () => {
            const updateRes = await request(app)
                .put(`/api/products/${products[0].id}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`)
                .send({
                    name: 'Admin Updated Product',
                    stock: 100
                });

            expect(updateRes.statusCode).toBe(200);
            expect(updateRes.body).toHaveProperty('message', 'Product updated successfully');
            expect(updateRes.body.product).toHaveProperty('name', 'Admin Updated Product');
            expect(updateRes.body.product).toHaveProperty('stock', 100);
        });

        // Test case: User cannot update product they don't own
        test('should not allow user to update product they do not own', async () => {
            const updateRes = await request(app)
                .put(`/api/products/${products[0].id}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    name: 'Unauthorized Update'
                });

            expect(updateRes.statusCode).toBe(403);
            expect(updateRes.body).toHaveProperty('message', 'Not authorized to update this product');
        });

        // Test case: Validation errors when updating product
        test('should validate product update input', async () => {
            // First create a product owned by regularUser
            const createRes = await request(app)
                .post('/api/products')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    name: 'Product for Validation',
                    description: 'Will test validation',
                    price: 29.99,
                    stock: 10,
                    category: 'electronics'
                });

            const productId = createRes.body.product.id;

            // Now try invalid update
            const updateRes = await request(app)
                .put(`/api/products/${productId}`)
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    price: -10,  // Negative price
                    stock: -5    // Negative stock
                });

            expect(updateRes.statusCode).toBe(400);
            expect(updateRes.body).toHaveProperty('errors');
        });
    });

    describe('DELETE /api/products/:id', () => {
        // Test case: Delete product by owner
        test('should delete product by owner', async () => {
            // First create a product owned by regularUser
            const createRes = await request(app)
                .post('/api/products')
                .set('Authorization', `Bearer ${tokens.userToken}`)
                .send({
                    name: 'Product to Delete',
                    description: 'This will be deleted',
                    price: 19.99,
                    stock: 5,
                    category: 'clothing'
                });

            const productId = createRes.body.product.id;

            // Now delete the product
            const deleteRes = await request(app)
                .delete(`/api/products/${productId}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(deleteRes.statusCode).toBe(200);
            expect(deleteRes.body).toHaveProperty('message', 'Product deleted successfully');

            // Verify product is deleted
            const checkRes = await request(app)
                .get(`/api/products/${productId}`);

            expect(checkRes.statusCode).toBe(404);
        });

        // Test case: Admin can delete any product
        test('should allow admin to delete any product', async () => {
            const productId = products[1].id;

            const deleteRes = await request(app)
                .delete(`/api/products/${productId}`)
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(deleteRes.statusCode).toBe(200);
            expect(deleteRes.body).toHaveProperty('message', 'Product deleted successfully');

            // Verify product is deleted
            const checkRes = await request(app)
                .get(`/api/products/${productId}`);

            expect(checkRes.statusCode).toBe(404);
        });

        // Test case: User cannot delete product they don't own
        test('should not allow user to delete product they do not own', async () => {
            const productId = products[2].id;

            const deleteRes = await request(app)
                .delete(`/api/products/${productId}`)
                .set('Authorization', `Bearer ${tokens.userToken}`);

            expect(deleteRes.statusCode).toBe(403);
            expect(deleteRes.body).toHaveProperty('message', 'Not authorized to delete this product');
        });

        // Test case: Try to delete non-existent product
        test('should return 404 when deleting non-existent product', async () => {
            const deleteRes = await request(app)
                .delete('/api/products/9999')
                .set('Authorization', `Bearer ${tokens.adminToken}`);

            expect(deleteRes.statusCode).toBe(404);
            expect(deleteRes.body).toHaveProperty('message', 'Product not found');
        });
    });
});