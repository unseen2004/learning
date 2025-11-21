// src/components/Layout/Layout.jsx
import Navbar from './Navbar';

const Layout = ({ children }) => {
  return (
    <div className="app">
      <Navbar />
      <main className="main-content">
        <div className="container">
          {children}
        </div>
      </main>
    </div>
  );
};

export default Layout;