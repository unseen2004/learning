const jwt = require('jsonwebtoken');
const { User } = require('../models');

// Middleware to authenticate JWT tokens
const authenticate = async (req, res, next) => {
  try {
    // Get token from header
    const token = req.header('Authorization')?.replace('Bearer ', '');
    
    if (!token) {
      return res.status(401).json({ message: 'No authentication token, access denied' });
    }

    // Verify token
    const decoded = jwt.verify(token, process.env.JWT_SECRET);
    
    // Find user by id
    const user = await User.findByPk(decoded.id);
    
    if (!user) {
      return res.status(401).json({ message: 'Invalid token' });
    }

    // Attach user to request object
    req.user = user;
    next();
  } catch (error) {
    if (error.name === 'JsonWebTokenError') {
      return res.status(401).json({ message: 'Invalid token' });
    }
    if (error.name === 'TokenExpiredError') {
      return res.status(401).json({ message: 'Token expired' });
    }
    res.status(500).json({ message: 'Server error' });
  }
};

// Middleware to check if user is admin
const isAdmin = (req, res, next) => {
  if (req.user && req.user.role === 'admin') {
    next();
  } else {
    res.status(403).json({ message: 'Access denied: Admin role required' });
  }
};

// Middleware for pagination, filtering and sorting
const paginationFilter = (req, res, next) => {
  // Pagination
  const page = parseInt(req.query.page) || 1;
  const limit = parseInt(req.query.limit) || 10;
  const offset = (page - 1) * limit;

  // Sorting
  let order = [];
  if (req.query.sort) {
    const sortField = req.query.sort.startsWith('-') 
      ? req.query.sort.substring(1)
      : req.query.sort;
    const sortOrder = req.query.sort.startsWith('-') ? 'DESC' : 'ASC';
    order = [[sortField, sortOrder]];
  } else {
    order = [['createdAt', 'DESC']];
  }
  
  // Attach to request object
  req.pagination = { limit, offset };
  req.sorting = { order };
  
  // Basic filtering - will be handled in controllers
  req.filtering = req.query;
  
  next();
};

module.exports = {
  authenticate,
  isAdmin,
  paginationFilter
};