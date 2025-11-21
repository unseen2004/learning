const { Review, User, Product } = require('../models');
const { validationResult } = require('express-validator');

// @route   GET /api/products/:productId/reviews
// @desc    Get all reviews for a product
// @access  Public
exports.getProductReviews = async (req, res) => {
  try {
    const reviews = await Review.findAll({
      where: { productId: req.params.productId },
      include: [{
        model: User,
        attributes: ['id', 'username']
      }],
      order: [['createdAt', 'DESC']]
    });
    
    res.json(reviews);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   GET /api/reviews/:id
// @desc    Get review by ID
// @access  Public
exports.getReviewById = async (req, res) => {
  try {
    const review = await Review.findByPk(req.params.id, {
      include: [
        {
          model: User,
          attributes: ['id', 'username']
        },
        {
          model: Product,
          attributes: ['id', 'name']
        }
      ]
    });

    if (!review) {
      return res.status(404).json({ message: 'Review not found' });
    }

    res.json(review);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   POST /api/products/:productId/reviews
// @desc    Create a new review for a product
// @access  Private
exports.createReview = async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }

  try {
    const { content, rating } = req.body;
    const productId = req.params.productId;

    // Check if product exists
    const product = await Product.findByPk(productId);
    
    if (!product) {
      return res.status(404).json({ message: 'Product not found' });
    }

    // Check if user already reviewed this product
    const existingReview = await Review.findOne({
      where: { 
        userId: req.user.id,
        productId
      }
    });

    if (existingReview) {
      return res.status(400).json({ message: 'You already reviewed this product' });
    }

    const review = await Review.create({
      content,
      rating,
      userId: req.user.id,
      productId
    });

    const reviewWithUser = await Review.findByPk(review.id, {
      include: [{
        model: User,
        attributes: ['id', 'username']
      }]
    });

    res.status(201).json({
      message: 'Review created successfully',
      review: reviewWithUser
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   PUT /api/reviews/:id
// @desc    Update review
// @access  Private
exports.updateReview = async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }

  try {
    const review = await Review.findByPk(req.params.id);

    if (!review) {
      return res.status(404).json({ message: 'Review not found' });
    }

    // Only allow review author to update
    if (req.user.id !== review.userId) {
      return res.status(403).json({ message: 'Not authorized to update this review' });
    }

    const { content, rating } = req.body;

    // Update review fields
    if (content) review.content = content;
    if (rating) review.rating = rating;

    await review.save();

    const updatedReview = await Review.findByPk(review.id, {
      include: [{
        model: User,
        attributes: ['id', 'username']
      }]
    });

    res.json({
      message: 'Review updated successfully',
      review: updatedReview
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   DELETE /api/reviews/:id
// @desc    Delete review
// @access  Private
exports.deleteReview = async (req, res) => {
  try {
    const review = await Review.findByPk(req.params.id);

    if (!review) {
      return res.status(404).json({ message: 'Review not found' });
    }

    // Only allow review author or admin to delete
    if (req.user.role !== 'admin' && req.user.id !== review.userId) {
      return res.status(403).json({ message: 'Not authorized to delete this review' });
    }

    await review.destroy();
    res.json({ message: 'Review deleted successfully' });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};