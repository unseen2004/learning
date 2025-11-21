const { Product, User, Review } = require('../models');
const { validationResult } = require('express-validator');
const { Op } = require('sequelize');

// @route   GET /api/products
// @desc    Get all products with pagination, filtering and sorting
// @access  Public
exports.getAllProducts = async (req, res) => {
  try {
    // Extract pagination, sorting and filtering from middleware
    const { limit, offset } = req.pagination;
    const { order } = req.sorting;
    
    // Build where clause for filtering
    const where = {};
    
    // Filter by category if provided
    if (req.query.category) {
      where.category = req.query.category;
    }
    
    // Filter by price range if provided
    if (req.query.minPrice) {
      where.price = { ...where.price, [Op.gte]: parseFloat(req.query.minPrice) };
    }
    if (req.query.maxPrice) {
      where.price = { ...where.price, [Op.lte]: parseFloat(req.query.maxPrice) };
    }
    
    // Search by name if provided
    if (req.query.search) {
      where.name = { [Op.like]: `%${req.query.search}%` };
    }

    // Get products with pagination and filtering
    const { count, rows: products } = await Product.findAndCountAll({
      where,
      limit,
      offset,
      order,
      include: [
        {
          model: User,
          attributes: ['id', 'username']
        }
      ]
    });

    // Calculate pagination info
    const totalPages = Math.ceil(count / limit);
    const currentPage = Math.floor(offset / limit) + 1;

    res.json({
      products,
      pagination: {
        totalItems: count,
        totalPages,
        currentPage,
        itemsPerPage: limit
      }
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   GET /api/products/:id
// @desc    Get product by ID
// @access  Public
exports.getProductById = async (req, res) => {
  try {
    const product = await Product.findByPk(req.params.id, {
      include: [
        {
          model: User,
          attributes: ['id', 'username']
        },
        {
          model: Review,
          include: [{
            model: User,
            attributes: ['id', 'username']
          }]
        }
      ]
    });

    if (!product) {
      return res.status(404).json({ message: 'Product not found' });
    }

    res.json(product);
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   POST /api/products
// @desc    Create a new product
// @access  Private
exports.createProduct = async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }

  try {
    const { name, description, price, stock, category } = req.body;

    const product = await Product.create({
      name,
      description,
      price,
      stock,
      category,
      userId: req.user.id
    });

    res.status(201).json({
      message: 'Product created successfully',
      product
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   PUT /api/products/:id
// @desc    Update product
// @access  Private
exports.updateProduct = async (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }

  try {
    const product = await Product.findByPk(req.params.id);

    if (!product) {
      return res.status(404).json({ message: 'Product not found' });
    }

    // Only allow product owner or admin to update
    if (req.user.role !== 'admin' && req.user.id !== product.userId) {
      return res.status(403).json({ message: 'Not authorized to update this product' });
    }

    const { name, description, price, stock, category } = req.body;

    // Update product fields
    product.name = name || product.name;
    product.description = description || product.description;
    product.price = price || product.price;
    product.stock = stock !== undefined ? stock : product.stock;
    product.category = category || product.category;

    await product.save();

    res.json({
      message: 'Product updated successfully',
      product
    });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};

// @route   DELETE /api/products/:id
// @desc    Delete product
// @access  Private
exports.deleteProduct = async (req, res) => {
  try {
    const product = await Product.findByPk(req.params.id);

    if (!product) {
      return res.status(404).json({ message: 'Product not found' });
    }

    // Only allow product owner or admin to delete
    if (req.user.role !== 'admin' && req.user.id !== product.userId) {
      return res.status(403).json({ message: 'Not authorized to delete this product' });
    }

    await product.destroy();
    res.json({ message: 'Product deleted successfully' });
  } catch (error) {
    console.error(error);
    res.status(500).json({ message: 'Server error' });
  }
};