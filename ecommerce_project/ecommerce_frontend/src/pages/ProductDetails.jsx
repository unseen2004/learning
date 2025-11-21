// src/pages/ProductDetails.jsx
import { useState, useEffect } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { productService } from '../services/productService';
import { isAuthenticated, getUser } from '../utils/authUtils';
import ReviewList from '../components/Reviews/ReviewList';
import toast from 'react-hot-toast';

const ProductDetails = () => {
  const { id } = useParams();
  const navigate = useNavigate();
  const [product, setProduct] = useState(null);
  const [loading, setLoading] = useState(true);
  const [isEditing, setIsEditing] = useState(false);
  const [editForm, setEditForm] = useState({
    name: '',
    description: '',
    price: '',
    stock: '',
    category: ''
  });

  const currentUser = getUser();
  const canEdit = currentUser && (currentUser.id === product?.userId || currentUser.role === 'admin');

  useEffect(() => {
    fetchProductDetails();
  }, [id]);

  const fetchProductDetails = async () => {
    try {
      const data = await productService.getProductById(id);
      setProduct(data);
      setEditForm({
        name: data.name,
        description: data.description,
        price: data.price,
        stock: data.stock,
        category: data.category
      });
    } catch (error) {
      toast.error('Failed to fetch product details');
      console.error('Error:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleUpdate = async (e) => {
    e.preventDefault();
    try {
      await productService.updateProduct(id, editForm);
      toast.success('Product updated successfully');
      setIsEditing(false);
      fetchProductDetails();
    } catch (error) {
      toast.error(error.message || 'Failed to update product');
    }
  };

  const handleDelete = async () => {
    if (window.confirm('Are you sure you want to delete this product?')) {
      try {
        await productService.deleteProduct(id);
        toast.success('Product deleted successfully');
        navigate('/products');
      } catch (error) {
        toast.error(error.message || 'Failed to delete product');
      }
    }
  };

  if (loading) {
    return <div className="loading">Loading product details...</div>;
  }

  if (!product) {
    return <div className="error">Product not found</div>;
  }

  return (
    <div className="product-details-page">
      <div className="product-detail">
        {isEditing ? (
          <form onSubmit={handleUpdate}>
            <div className="form-group">
              <label className="form-label">Name</label>
              <input
                type="text"
                className="form-input"
                value={editForm.name}
                onChange={(e) => setEditForm({ ...editForm, name: e.target.value })}
                required
              />
            </div>
            
            <div className="form-group">
              <label className="form-label">Description</label>
              <textarea
                className="form-textarea"
                rows="4"
                value={editForm.description}
                onChange={(e) => setEditForm({ ...editForm, description: e.target.value })}
                required
              />
            </div>
            
            <div className="form-group">
              <label className="form-label">Price</label>
              <input
                type="number"
                step="0.01"
                className="form-input"
                value={editForm.price}
                onChange={(e) => setEditForm({ ...editForm, price: e.target.value })}
                required
              />
            </div>
            
            <div className="form-group">
              <label className="form-label">Stock</label>
              <input
                type="number"
                className="form-input"
                value={editForm.stock}
                onChange={(e) => setEditForm({ ...editForm, stock: e.target.value })}
                required
              />
            </div>
            
            <div className="form-group">
              <label className="form-label">Category</label>
              <input
                type="text"
                className="form-input"
                value={editForm.category}
                onChange={(e) => setEditForm({ ...editForm, category: e.target.value })}
                required
              />
            </div>
            
            <div className="d-flex gap-1">
              <button type="submit" className="btn btn-primary">Save Changes</button>
              <button 
                type="button" 
                className="btn btn-secondary"
                onClick={() => setIsEditing(false)}
              >
                Cancel
              </button>
            </div>
          </form>
        ) : (
          <>
            <div className="product-detail-header">
              <div>
                <h1 className="product-detail-title">{product.name}</h1>
                <span className="product-category">{product.category}</span>
                <p style={{ marginTop: '0.5rem', color: '#666' }}>
                  By {product.User?.username || 'Unknown'}
                </p>
              </div>
              <div className="text-right">
                <p className="product-detail-price">${product.price}</p>
                <p style={{ color: '#666' }}>Stock: {product.stock}</p>
              </div>
            </div>

            <div className="product-detail-body">
              <p style={{ fontSize: '1.1rem', lineHeight: 1.6, color: '#444' }}>
                {product.description}
              </p>
            </div>

            {canEdit && (
              <div className="product-actions mt-3 d-flex gap-1">
                <button 
                  className="btn btn-primary"
                  onClick={() => setIsEditing(true)}
                >
                  Edit Product
                </button>
                <button 
                  className="btn btn-danger"
                  onClick={handleDelete}
                >
                  Delete Product
                </button>
              </div>
            )}
          </>
        )}
      </div>

      {!isEditing && (
        <ReviewList productId={id} />
      )}
    </div>
  );
};

export default ProductDetails;