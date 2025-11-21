const express = require('express');
const { body } = require('express-validator');
const userController = require('../controllers/userController');
const { authenticate, isAdmin } = require('../middleware/auth');
const router = express.Router();

// All routes in this file are protected
router.use(authenticate);

// @route   GET /api/users
// @desc    Get all users (admin only)
router.get('/', isAdmin, userController.getAllUsers);

// @route   GET /api/users/:id
// @desc    Get user by id
router.get('/:id', userController.getUserById);

// @route   PUT /api/users/:id
// @desc    Update user
router.put(
  '/:id',
  [
    body('username')
      .optional()
      .isLength({ min: 3, max: 30 }).withMessage('Username must be between 3 and 30 characters'),
    body('email')
      .optional()
      .isEmail().withMessage('Please include a valid email'),
    body('password')
      .optional()
      .isLength({ min: 6 }).withMessage('Password must be at least 6 characters')
  ],
  userController.updateUser
);

// @route   DELETE /api/users/:id
// @desc    Delete user
router.delete('/:id', userController.deleteUser);

module.exports = router;