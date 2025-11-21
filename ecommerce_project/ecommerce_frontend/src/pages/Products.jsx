// src/pages/Products.jsx
import { useState, useEffect } from 'react';
import { productService } from '../services/productService';
import ProductCard from '../components/Products/ProductCard';
import toast from 'react-hot-toast';

const Products = () => {
  const [products, setProducts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [pagination, setPagination] = useState({
    currentPage: 1,
    totalPages: 1,
    itemsPerPage: 12
  });
  const [filters, setFilters] = useState({
    search: '',
    category: '',
    minPrice: '',
    maxPrice: '',
    sort: '-createdAt'
  });

  useEffect(() => {
    fetchProducts();
  }, [pagination.currentPage, filters.sort]);

  const fetchProducts = async () => {
    setLoading(true);
    try {
      const params = {
        page: pagination.currentPage,
        limit: pagination.itemsPerPage,
        sort: filters.sort,
        ...(filters.search && { search: filters.search }),
        ...(filters.category && { category: filters.category }),
        ...(filters.minPrice && { minPrice: filters.minPrice }),
        ...(filters.maxPrice && { maxPrice: filters.maxPrice })
      };

      const response = await productService.getAllProducts(params);
      setProducts(response.products || []);
      setPagination(prev => ({
        ...prev,
        totalPages: response.pagination?.totalPages || 1,
        currentPage: response.pagination?.currentPage || 1
      }));
    } catch (error) {
      toast.error('Failed to fetch products');
      console.error('Error fetching products:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleSearch = (e) => {
    e.preventDefault();
    setPagination(prev => ({ ...prev, currentPage: 1 }));
    fetchProducts();
  };

  const handlePageChange = (page) => {
    setPagination(prev => ({ ...prev, currentPage: page }));
  };

  const handleSortChange = (e) => {
    setFilters(prev => ({ ...prev, sort: e.target.value }));
    setPagination(prev => ({ ...prev, currentPage: 1 }));
  };

  const renderPagination = () => {
    const pages = [];
    for (let i = 1; i <= pagination.totalPages; i++) {
      pages.push(
        <button
          key={i}
          className={`pagination-btn ${i === pagination.currentPage ? 'active' : ''}`}
          onClick={() => handlePageChange(i)}
        >
          {i}
        </button>
      );
    }
    return pages;
  };

  return (
    <div className="products-page">
      <h1 className="mb-4">Products</h1>

      {/* Filters */}
      <div className="filters-section mb-4" style={{ 
        background: 'white', 
        padding: '1.5rem', 
        borderRadius: '8px',
        boxShadow: '0 2px 4px rgba(0,0,0,0.1)'
      }}>
        <form onSubmit={handleSearch}>
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '1rem' }}>
            <div className="form-group mb-0">
              <input
                type="text"
                className="form-input"
                placeholder="Search products..."
                value={filters.search}
                onChange={(e) => setFilters(prev => ({ ...prev, search: e.target.value }))}
              />
            </div>
            
            <div className="form-group mb-0">
              <input
                type="text"
                className="form-input"
                placeholder="Category"
                value={filters.category}
                onChange={(e) => setFilters(prev => ({ ...prev, category: e.target.value }))}
              />
            </div>
            
            <div className="form-group mb-0">
              <input
                type="number"
                className="form-input"
                placeholder="Min Price"
                value={filters.minPrice}
                onChange={(e) => setFilters(prev => ({ ...prev, minPrice: e.target.value }))}
              />
            </div>
            
            <div className="form-group mb-0">
              <input
                type="number"
                className="form-input"
                placeholder="Max Price"
                value={filters.maxPrice}
                onChange={(e) => setFilters(prev => ({ ...prev, maxPrice: e.target.value }))}
              />
            </div>
            
            <div className="form-group mb-0">
              <select 
                className="form-select" 
                value={filters.sort} 
                onChange={handleSortChange}
              >
                <option value="-createdAt">Newest First</option>
                <option value="createdAt">Oldest First</option>
                <option value="price">Price: Low to High</option>
                <option value="-price">Price: High to Low</option>
                <option value="name">Name: A-Z</option>
                <option value="-name">Name: Z-A</option>
              </select>
            </div>
            
            <div className="form-group mb-0">
              <button type="submit" className="btn btn-primary" style={{ width: '100%' }}>
                Apply Filters
              </button>
            </div>
          </div>
        </form>
      </div>

      {/* Products Grid */}
      {loading ? (
        <div className="loading">Loading products...</div>
      ) : products.length === 0 ? (
        <div className="text-center" style={{ padding: '3rem' }}>
          <p>No products found. Try adjusting your filters.</p>
        </div>
      ) : (
        <>
          <div className="products-grid">
            {products.map(product => (
              <ProductCard key={product.id} product={product} />
            ))}
          </div>

          {/* Pagination */}
          {pagination.totalPages > 1 && (
            <div className="pagination">
              <button
                className="pagination-btn"
                onClick={() => handlePageChange(pagination.currentPage - 1)}
                disabled={pagination.currentPage === 1}
              >
                Previous
              </button>
              
              {renderPagination()}
              
              <button
                className="pagination-btn"
                onClick={() => handlePageChange(pagination.currentPage + 1)}
                disabled={pagination.currentPage === pagination.totalPages}
              >
                Next
              </button>
            </div>
          )}
        </>
      )}
    </div>
  );
};

export default Products;