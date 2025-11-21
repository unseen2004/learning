// src/components/Products/ProductCard.jsx
import { Link } from 'react-router-dom';

const ProductCard = ({ product }) => {
  return (
    <div className="product-card">
      <div className="product-image" style={{ 
        display: 'flex', 
        alignItems: 'center', 
        justifyContent: 'center',
        height: '200px',
        backgroundColor: '#f0f0f0'
      }}>
        <span style={{ color: '#999' }}>Product Image</span>
      </div>
      <div className="product-content">
        <h3 className="product-title">{product.name}</h3>
        <p className="product-price">${product.price}</p>
        <span className="product-category">{product.category}</span>
        <p style={{ 
          marginTop: '0.5rem', 
          color: '#666',
          fontSize: '0.875rem',
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          display: '-webkit-box',
          WebkitLineClamp: 2,
          WebkitBoxOrient: 'vertical'
        }}>
          {product.description}
        </p>
        <div className="mt-2 d-flex gap-1">
          <Link to={`/products/${product.id}`} className="btn btn-primary">
            View Details
          </Link>
          <span style={{ marginLeft: 'auto', color: '#666', fontSize: '0.875rem' }}>
            Stock: {product.stock}
          </span>
        </div>
      </div>
    </div>
  );
};

export default ProductCard;