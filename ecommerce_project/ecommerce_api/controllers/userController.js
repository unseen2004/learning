const { User } = require('../models');
const { validationResult } = require('express-validator');
const { Op } = require('sequelize');

// @route   GET /api/users
// @desc    Get all users (admin only)
// @access  Private/Admin
exports.getAllUsers = async (req, res) => {
  try {
    const users = await User.findAll({
      attributes: ['id', 'username', 'email', 'role', 'createdAt']
    });
    res.json(users);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   GET /api/users/:id
// @desc    Get user by ID
// @access  Private
exports.getUserById = async (req, res) => {
  try {
    const user = await User.findByPk(req.params.id, {
      attributes: ['id', 'username', 'email', 'role', 'createdAt']
    });

    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }

    // Only allow admins or the user themselves to view details
    if (req.user.role !== 'admin' && req.user.id !== user.id) {
      return res.status(403).json({ message: 'Not authorized to view this user' });
    }

    res.json(user);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   PUT /api/users/:id
// @desc    Update user
// @access  Private
exports.updateUser = async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }

  try {
    const user = await User.findByPk(req.params.id);

    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }

    // Only allow admins or the user themselves to update
    if (req.user.role !== 'admin' && req.user.id !== user.id) {
      return res.status(403).json({ message: 'Not authorized to update this user' });
    }

    const { username, email, password } = req.body;

    // Check if username or email is already taken
    if (username && username !== user.username) {
      const usernameExists = await User.findOne({ where: { username } });
      if (usernameExists) {
        return res.status(400).json({ message: 'Username already taken' });
      }
      user.username = username;
    }

    if (email && email !== user.email) {
      const emailExists = await User.findOne({ where: { email } });
      if (emailExists) {
        return res.status(400).json({ message: 'Email already taken' });
      }
      user.email = email;
    }

    if (password) {
      user.password = password;
    }

    // Only admins can change roles
    if (req.body.role && req.user.role === 'admin') {
      user.role = req.body.role;
    }

    await user.save();

    res.json({
      message: 'User updated successfully',
      user: {
        id: user.id,
        username: user.username,
        email: user.email,
        role: user.role
      }
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   DELETE /api/users/:id
// @desc    Delete user
// @access  Private
exports.deleteUser = async (req, res) => {
  try {
    const user = await User.findByPk(req.params.id);

    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }

    // Only allow admins or the user themselves to delete
    if (req.user.role !== 'admin' && req.user.id !== user.id) {
      return res.status(403).json({ message: 'Not authorized to delete this user' });
    }

    await user.destroy();
    res.json({ message: 'User deleted successfully' });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};