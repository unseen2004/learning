// src/pages/Home.jsx
import { Link } from 'react-router-dom';
import { useEffect, useState } from 'react';
import { productService } from '../services/productService';
import ProductCard from '../components/Products/ProductCard';

const Home = () => {
  const [featuredProducts, setFeaturedProducts] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchFeaturedProducts();
  }, []);

  const fetchFeaturedProducts = async () => {
    try {
      const response = await productService.getAllProducts({ limit: 4 });
      setFeaturedProducts(response.products || []);
    } catch (error) {
      console.error('Error fetching featured products:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="home-page">
      <section className="hero-section" style={{ textAlign: 'center', padding: '4rem 0' }}>
        <h1 style={{ fontSize: '3rem', marginBottom: '1rem' }}>Welcome to E-Commerce Store</h1>
        <p style={{ fontSize: '1.25rem', color: '#666', marginBottom: '2rem' }}>
          Discover amazing products at great prices
        </p>
        <Link to="/products" className="btn btn-primary">
          Browse Products
        </Link>
      </section>

      <section className="featured-section mt-4">
        <h2 className="text-center mb-3">Featured Products</h2>
        
        {loading ? (
          <div className="loading">Loading featured products...</div>
        ) : (
          <div className="products-grid">
            {featuredProducts.map(product => (
              <ProductCard key={product.id} product={product} />
            ))}
          </div>
        )}
        
        <div className="text-center mt-4">
          <Link to="/products" className="btn btn-outline">
            View All Products
          </Link>
        </div>
      </section>

      <section className="info-section" style={{ 
        marginTop: '4rem', 
        padding: '3rem 0',
        backgroundColor: '#f8f9fa',
        borderRadius: '8px'
      }}>
        <div className="container">
          <h2 className="text-center mb-3">Why Choose Us?</h2>
          <div style={{ 
            display: 'grid', 
            gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))',
            gap: '2rem',
            marginTop: '2rem'
          }}>
            <div className="text-center">
              <h3 style={{ color: '#007bff', marginBottom: '1rem' }}>Quality Products</h3>
              <p>We offer only the best quality products from trusted sellers</p>
            </div>
            <div className="text-center">
              <h3 style={{ color: '#007bff', marginBottom: '1rem' }}>Secure Shopping</h3>
              <p>Your data is safe with our secure authentication system</p>
            </div>
            <div className="text-center">
              <h3 style={{ color: '#007bff', marginBottom: '1rem' }}>Customer Reviews</h3>
              <p>Read authentic reviews from real customers</p>
            </div>
          </div>
        </div>
      </section>
    </div>
  );
};

export default Home;