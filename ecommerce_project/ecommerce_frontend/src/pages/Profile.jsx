// src/pages/Profile.jsx
import { useState, useEffect } from 'react';
import { getUser } from '../utils/authUtils';
import { productService } from '../services/productService';
import ProductCard from '../components/Products/ProductCard';

const Profile = () => {
  const user = getUser();
  const [userProducts, setUserProducts] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchUserProducts();
  }, []);

  const fetchUserProducts = async () => {
    try {
      // Fetch all products and filter by user
      const response = await productService.getAllProducts({ limit: 100 });
      const myProducts = response.products.filter(product => product.userId === user.id);
      setUserProducts(myProducts);
    } catch (error) {
      console.error('Error fetching user products:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="profile-page">
      <div className="profile-header" style={{
        background: 'white',
        padding: '2rem',
        borderRadius: '8px',
        marginBottom: '2rem',
        boxShadow: '0 2px 4px rgba(0,0,0,0.1)'
      }}>
        <h1>My Profile</h1>
        <div style={{ marginTop: '1rem' }}>
          <p><strong>Username:</strong> {user?.username}</p>
          <p><strong>Email:</strong> {user?.email}</p>
          <p><strong>Role:</strong> {user?.role}</p>
          <p><strong>Member Since:</strong> {new Date(user?.createdAt).toLocaleDateString()}</p>
        </div>
      </div>

      <div className="user-products">
        <h2 className="mb-3">My Products</h2>
        
        {loading ? (
          <div className="loading">Loading your products...</div>
        ) : userProducts.length === 0 ? (
          <div style={{ 
            textAlign: 'center', 
            padding: '3rem',
            background: 'white',
            borderRadius: '8px'
          }}>
            <p>You haven't created any products yet.</p>
            <a href="/products/create" className="btn btn-primary mt-2">
              Create Your First Product
            </a>
          </div>
        ) : (
          <div className="products-grid">
            {userProducts.map(product => (
              <ProductCard key={product.id} product={product} />
            ))}
          </div>
        )}
      </div>
    </div>
  );
};

export default Profile;