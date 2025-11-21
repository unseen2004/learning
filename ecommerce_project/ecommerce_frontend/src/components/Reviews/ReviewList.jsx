// src/components/Reviews/ReviewList.jsx
import { useState, useEffect } from 'react';
import { useForm } from 'react-hook-form';
import { productService } from '../../services/productService';
import { isAuthenticated, getUser } from '../../utils/authUtils';
import toast from 'react-hot-toast';

const ReviewList = ({ productId }) => {
  const [reviews, setReviews] = useState([]);
  const [loading, setLoading] = useState(true);
  const [showReviewForm, setShowReviewForm] = useState(false);
  const [editingReview, setEditingReview] = useState(null);
  const { register, handleSubmit, reset, setValue, formState: { errors } } = useForm();
  
  const currentUser = getUser();

  useEffect(() => {
    fetchReviews();
  }, [productId]);

  const fetchReviews = async () => {
    try {
      const data = await productService.getProductReviews(productId);
      setReviews(data);
    } catch (error) {
      console.error('Error fetching reviews:', error);
    } finally {
      setLoading(false);
    }
  };

  const onSubmit = async (data) => {
    try {
      if (editingReview) {
        await productService.updateReview(editingReview.id, data);
        toast.success('Review updated successfully');
        setEditingReview(null);
      } else {
        await productService.createReview(productId, data);
        toast.success('Review added successfully');
        setShowReviewForm(false);
      }
      reset();
      fetchReviews();
    } catch (error) {
      toast.error(error.message || 'Failed to save review');
    }
  };

  const handleEdit = (review) => {
    setEditingReview(review);
    setValue('content', review.content);
    setValue('rating', review.rating);
    setShowReviewForm(true);
  };

  const handleDelete = async (reviewId) => {
    if (window.confirm('Are you sure you want to delete this review?')) {
      try {
        await productService.deleteReview(reviewId);
        toast.success('Review deleted successfully');
        fetchReviews();
      } catch (error) {
        toast.error(error.message || 'Failed to delete review');
      }
    }
  };

  const renderStars = (rating) => {
    return '★'.repeat(rating) + '☆'.repeat(5 - rating);
  };

  return (
    <div className="reviews-section">
      <h2 className="mb-3">Customer Reviews</h2>

      {isAuthenticated() && !showReviewForm && (
        <button 
          className="btn btn-primary mb-3"
          onClick={() => setShowReviewForm(true)}
        >
          Write a Review
        </button>
      )}

      {showReviewForm && isAuthenticated() && (
        <div className="review-form mb-4" style={{ 
          background: 'white', 
          padding: '1.5rem',
          borderRadius: '8px',
          boxShadow: '0 2px 4px rgba(0,0,0,0.1)'
        }}>
          <h3>{editingReview ? 'Edit Review' : 'Write a Review'}</h3>
          <form onSubmit={handleSubmit(onSubmit)}>
            <div className="form-group">
              <label className="form-label">Rating</label>
              <select 
                className="form-select"
                {...register('rating', { 
                  required: 'Rating is required',
                  valueAsNumber: true 
                })}
              >
                <option value="">Select rating</option>
                <option value="5">5 - Excellent</option>
                <option value="4">4 - Good</option>
                <option value="3">3 - Average</option>
                <option value="2">2 - Poor</option>
                <option value="1">1 - Terrible</option>
              </select>
              {errors.rating && (
                <span className="form-error">{errors.rating.message}</span>
              )}
            </div>

            <div className="form-group">
              <label className="form-label">Review</label>
              <textarea
                className="form-textarea"
                rows="4"
                {...register('content', { required: 'Review content is required' })}
              />
              {errors.content && (
                <span className="form-error">{errors.content.message}</span>
              )}
            </div>

            <div className="d-flex gap-1">
              <button type="submit" className="btn btn-primary">
                {editingReview ? 'Update Review' : 'Submit Review'}
              </button>
              <button 
                type="button" 
                className="btn btn-secondary"
                onClick={() => {
                  setShowReviewForm(false);
                  setEditingReview(null);
                  reset();
                }}
              >
                Cancel
              </button>
            </div>
          </form>
        </div>
      )}

      {loading ? (
        <div className="loading">Loading reviews...</div>
      ) : reviews.length === 0 ? (
        <p>No reviews yet. Be the first to review this product!</p>
      ) : (
        reviews.map(review => (
          <div key={review.id} className="review-card">
            <div className="review-header">
              <div>
                <span className="review-author">{review.User?.username}</span>
                <span className="review-rating" style={{ marginLeft: '1rem' }}>
                  {renderStars(review.rating)}
                </span>
              </div>
              <span style={{ color: '#999', fontSize: '0.875rem' }}>
                {new Date(review.createdAt).toLocaleDateString()}
              </span>
            </div>
            <p className="review-content">{review.content}</p>
            
            {currentUser && currentUser.id === review.userId && (
              <div className="mt-2 d-flex gap-1">
                <button 
                  className="btn btn-sm btn-outline"
                  onClick={() => handleEdit(review)}
                >
                  Edit
                </button>
                <button 
                  className="btn btn-sm btn-danger"
                  onClick={() => handleDelete(review.id)}
                >
                  Delete
                </button>
              </div>
            )}
          </div>
        ))
      )}
    </div>
  );
};

export default ReviewList;