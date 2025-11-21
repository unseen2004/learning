const express = require('express');
const { body } = require('express-validator');
const reviewController = require('../controllers/reviewController');
const { authenticate } = require('../middleware/auth');
const router = express.Router();

// @route   GET /api/reviews/:id
// @desc    Get review by id
router.get('/:id', reviewController.getReviewById);

// Protected routes - require authentication
router.use(authenticate);

// @route   PUT /api/reviews/:id
// @desc    Update review
router.put(
  '/:id',
  [
    body('content')
      .optional()
      .notEmpty().withMessage('Review content cannot be empty'),
    body('rating')
      .optional()
      .isInt({ min: 1, max: 5 }).withMessage('Rating must be between 1 and 5')
  ],
  reviewController.updateReview
);

// @route   DELETE /api/reviews/:id
// @desc    Delete review
router.delete('/:id', reviewController.deleteReview);

module.exports = router;