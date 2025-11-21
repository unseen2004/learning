// src/pages/CreateProduct.jsx
import { useNavigate } from 'react-router-dom';
import { useForm } from 'react-hook-form';
import { productService } from '../services/productService';
import toast from 'react-hot-toast';

const CreateProduct = () => {
  const navigate = useNavigate();
  const { register, handleSubmit, formState: { errors } } = useForm();

  const onSubmit = async (data) => {
    try {
      const response = await productService.createProduct(data);
      toast.success('Product created successfully!');
      navigate(`/products/${response.product.id}`);
    } catch (error) {
      toast.error(error.message || 'Failed to create product');
    }
  };

  return (
    <div className="form-container" style={{ maxWidth: '600px' }}>
      <h2 className="text-center mb-4">Create New Product</h2>
      
      <form onSubmit={handleSubmit(onSubmit)}>
        <div className="form-group">
          <label className="form-label">Product Name</label>
          <input
            type="text"
            className="form-input"
            {...register('name', {
              required: 'Product name is required',
              minLength: {
                value: 2,
                message: 'Name must be at least 2 characters'
              },
              maxLength: {
                value: 100,
                message: 'Name must not exceed 100 characters'
              }
            })}
          />
          {errors.name && (
            <span className="form-error">{errors.name.message}</span>
          )}
        </div>

        <div className="form-group">
          <label className="form-label">Description</label>
          <textarea
            className="form-textarea"
            rows="4"
            {...register('description', {
              required: 'Description is required'
            })}
          />
          {errors.description && (
            <span className="form-error">{errors.description.message}</span>
          )}
        </div>

        <div className="form-group">
          <label className="form-label">Price</label>
          <input
            type="number"
            step="0.01"
            className="form-input"
            {...register('price', {
              required: 'Price is required',
              min: {
                value: 0,
                message: 'Price must be a positive number'
              },
              valueAsNumber: true
            })}
          />
          {errors.price && (
            <span className="form-error">{errors.price.message}</span>
          )}
        </div>

        <div className="form-group">
          <label className="form-label">Stock Quantity</label>
          <input
            type="number"
            className="form-input"
            {...register('stock', {
              required: 'Stock quantity is required',
              min: {
                value: 0,
                message: 'Stock must be a positive number'
              },
              valueAsNumber: true
            })}
          />
          {errors.stock && (
            <span className="form-error">{errors.stock.message}</span>
          )}
        </div>

        <div className="form-group">
          <label className="form-label">Category</label>
          <input
            type="text"
            className="form-input"
            placeholder="e.g., Electronics, Clothing, Books"
            {...register('category', {
              required: 'Category is required'
            })}
          />
          {errors.category && (
            <span className="form-error">{errors.category.message}</span>
          )}
        </div>

        <div className="d-flex gap-2">
          <button type="submit" className="btn btn-primary" style={{ flex: 1 }}>
            Create Product
          </button>
          <button 
            type="button" 
            className="btn btn-secondary"
            onClick={() => navigate('/products')}
          >
            Cancel
          </button>
        </div>
      </form>
    </div>
  );
};

export default CreateProduct;