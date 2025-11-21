// src/components/Layout/Navbar.jsx
import { Link, useNavigate, useLocation } from 'react-router-dom';
import { isAuthenticated, getUser } from '../../utils/authUtils';
import { authService } from '../../services/authService';
import toast from 'react-hot-toast';

const Navbar = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const user = getUser();

  const handleLogout = () => {
    authService.logout();
    toast.success('Logged out successfully');
    navigate('/');
  };

  const isActive = (path) => {
    return location.pathname === path ? 'active' : '';
  };

  return (
    <nav className="navbar">
      <div className="container">
        <div className="navbar-content">
          <Link to="/" className="navbar-brand">
            E-Commerce Store
          </Link>
          
          <ul className="navbar-menu">
            <li>
              <Link to="/" className={`navbar-link ${isActive('/')}`}>
                Home
              </Link>
            </li>
            <li>
              <Link to="/products" className={`navbar-link ${isActive('/products')}`}>
                Products
              </Link>
            </li>
            
            {isAuthenticated() ? (
              <>
                <li>
                  <Link to="/products/create" className={`navbar-link ${isActive('/products/create')}`}>
                    Add Product
                  </Link>
                </li>
                <li>
                  <Link to="/profile" className={`navbar-link ${isActive('/profile')}`}>
                    Profile ({user?.username})
                  </Link>
                </li>
                <li>
                  <button onClick={handleLogout} className="btn btn-outline">
                    Logout
                  </button>
                </li>
              </>
            ) : (
              <>
                <li>
                  <Link to="/login" className={`navbar-link ${isActive('/login')}`}>
                    Login
                  </Link>
                </li>
                <li>
                  <Link to="/register" className="btn btn-primary">
                    Register
                  </Link>
                </li>
              </>
            )}
          </ul>
        </div>
      </div>
    </nav>
  );
};

export default Navbar;