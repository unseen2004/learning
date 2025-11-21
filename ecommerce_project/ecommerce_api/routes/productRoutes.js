const express = require('express');
const { body } = require('express-validator');
const productController = require('../controllers/productController');
const reviewController = require('../controllers/reviewController');
const { authenticate, paginationFilter } = require('../middleware/auth');
const router = express.Router();

// @route   GET /api/products
// @desc    Get all products with pagination, filtering and sorting
router.get('/', paginationFilter, productController.getAllProducts);

// @route   GET /api/products/:id
// @desc    Get product by id
router.get('/:id', productController.getProductById);

// Protected routes - require authentication
router.use(authenticate);

// @route   POST /api/products
// @desc    Create a new product
router.post(
  '/',
  [
    body('name')
      .notEmpty().withMessage('Name is required')
      .isLength({ min: 2, max: 100 }).withMessage('Name must be between 2 and 100 characters'),
    body('description')
      .notEmpty().withMessage('Description is required'),
    body('price')
      .notEmpty().withMessage('Price is required')
      .isFloat({ min: 0 }).withMessage('Price must be a positive number'),
    body('stock')
      .optional()
      .isInt({ min: 0 }).withMessage('Stock must be a positive integer'),
    body('category')
      .notEmpty().withMessage('Category is required')
  ],
  productController.createProduct
);

// @route   PUT /api/products/:id
// @desc    Update product
router.put(
  '/:id',
  [
    body('name')
      .optional()
      .isLength({ min: 2, max: 100 }).withMessage('Name must be between 2 and 100 characters'),
    body('price')
      .optional()
      .isFloat({ min: 0 }).withMessage('Price must be a positive number'),
    body('stock')
      .optional()
      .isInt({ min: 0 }).withMessage('Stock must be a positive integer')
  ],
  productController.updateProduct
);

// @route   DELETE /api/products/:id
// @desc    Delete product
router.delete('/:id', productController.deleteProduct);

// Review routes related to products
// @route   GET /api/products/:productId/reviews
// @desc    Get all reviews for a product
router.get('/:productId/reviews', reviewController.getProductReviews);

// @route   POST /api/products/:productId/reviews
// @desc    Create a new review for a product
router.post(
  '/:productId/reviews',
  [
    body('content')
      .notEmpty().withMessage('Review content is required'),
    body('rating')
      .notEmpty().withMessage('Rating is required')
      .isInt({ min: 1, max: 5 }).withMessage('Rating must be between 1 and 5')
  ],
  reviewController.createReview
);

module.exports = router;