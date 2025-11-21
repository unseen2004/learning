# E-Commerce API Test Documentation

## Overview

This document provides a comprehensive overview of the testing strategy and results for the E-Commerce API. The tests ensure that our API functions correctly, handles edge cases properly, and enforces authorization and validation rules.

## Testing Environment

- **Framework**: Jest
- **HTTP Testing**: Supertest
- **Database**: PostgreSQL (test database)
- **Test Database**: A separate test database with clean state for each test run

## Test Categories

The tests are organized into the following categories:

1. **Authentication Tests**: Testing user registration and login endpoints
2. **User Management Tests**: Testing CRUD operations on user resources
3. **Product Tests**: Testing CRUD operations on product resources with filtering, pagination, and sorting
4. **Review Tests**: Testing operations related to product reviews
5. **Authorization Middleware Tests**: Testing the authentication and authorization middleware

## Test Case Summary

### Authentication Tests (auth.test.js)

| Test Case | Description | Expected Result | Status |
|-----------|-------------|-----------------|--------|
| Register user | Register with valid data | Status 201, returns token and user | ✅ |
| Register validation | Register with missing fields | Status 400, returns validation errors | ✅ |
| Register with invalid email | Register with invalid email format | Status 400, returns validation error | ✅ |
| Register with short password | Register with password < 6 chars | Status 400, returns validation error | ✅ |
| Register duplicate | Register with existing username/email | Status 400, returns error message | ✅ |
| Login success | Login with valid credentials | Status 200, returns token and user | ✅ |
| Login wrong password | Login with incorrect password | Status 400, returns error message | ✅ |
| Login non-existent | Login with non-existent email | Status 400, returns error message | ✅ |
| Login missing fields | Login with missing fields | Status 400, returns validation errors | ✅ |

### User Management Tests (user.test.js)

| Test Case | Description | Expected Result | Status |
|-----------|-------------|-----------------|--------|
| Get all users - admin | Admin requests all users | Status 200, returns array of users | ✅ |
| Get all users - non-admin | Regular user requests all users | Status 403, access denied | ✅ |
| Get all users - unauthenticated | No token provided | Status 401, authentication error | ✅ |
| Get user by ID - self | User requests own details | Status 200, returns user data | ✅ |
| Get user by ID - admin | Admin requests other user's details | Status 200, returns user data | ✅ |
| Get user by ID - unauthorized | User requests another user's details | Status 403, access denied | ✅ |
| Get non-existent user | Request user that doesn't exist | Status 404, user not found | ✅ |
| Update user - self | User updates own details | Status 200, returns updated user | ✅ |
| Update user - admin | Admin updates any user | Status 200, returns updated user | ✅ |
| Update user - unauthorized | User updates another user | Status 403, access denied | ✅ |
| Update role - unauthorized | Regular user tries to change role | Role doesn't change | ✅ |
| Update validation | Update with invalid data | Status 400, validation errors | ✅ |
| Delete user - self | User deletes own account | Status 200, account deleted | ✅ |
| Delete user - admin | Admin deletes any account | Status 200, account deleted | ✅ |
| Delete user - unauthorized | User deletes another user | Status 403, access denied | ✅ |

### Product Tests (product.test.js)

| Test Case | Description | Expected Result | Status |
|-----------|-------------|-----------------|--------|
| Get all products | Public access to products | Status 200, returns products and pagination | ✅ |
| Pagination | Request specific page and limit | Status 200, returns correct number and page | ✅ |
| Filter by category | Filter products by category | Status 200, returns only matching category | ✅ |
| Filter by price range | Filter by min/max price | Status 200, returns products in range | ✅ |
| Sort ascending | Sort products by price ascending | Status 200, products sorted correctly | ✅ |
| Sort descending | Sort products by price descending | Status 200, products sorted correctly | ✅ |
| Search by name | Search products by name | Status 200, returns matching products | ✅ |
| Get product by ID | Request specific product | Status 200, returns product details | ✅ |
| Get non-existent product | Request invalid product ID | Status 404, product not found | ✅ |
| Create product | Create with valid data | Status 201, product created | ✅ |
| Create - unauthenticated | Create without token | Status 401, authentication error | ✅ |
| Create validation | Create with invalid data | Status 400, validation errors | ✅ |
| Update product - owner | Owner updates product | Status 200, product updated | ✅ |
| Update product - admin | Admin updates any product | Status 200, product updated | ✅ |
| Update - unauthorized | User updates another's product | Status 403, access denied | ✅ |
| Update validation | Update with invalid data | Status 400, validation errors | ✅ |
| Delete product - owner | Owner deletes product | Status 200, product deleted | ✅ |
| Delete product - admin | Admin deletes any product | Status 200, product deleted | ✅ |
| Delete - unauthorized | User deletes another's product | Status 403, access denied | ✅ |
| Delete non-existent | Delete invalid product ID | Status 404, product not found | ✅ |

### Review Tests (review.test.js)

| Test Case | Description | Expected Result | Status |
|-----------|-------------|-----------------|--------|
| Get product reviews | Get all reviews for a product | Status 200, returns array of reviews | ✅ |
| Get reviews - empty | Get reviews for product with none | Status 200, returns empty array | ✅ |
| Get review by ID | Get specific review | Status 200, returns review with user and product | ✅ |
| Get non-existent review | Get invalid review ID | Status 404, review not found | ✅ |
| Create review | Create with valid data | Status 201, review created | ✅ |
| Create duplicate review | Review same product twice | Status 400, already reviewed error | ✅ |
| Review non-existent product | Review invalid product ID | Status 404, product not found | ✅ |
| Create validation | Create with invalid data | Status 400, validation errors | ✅ |
| Update review - owner | User updates own review | Status 200, review updated | ✅ |
| Update - unauthorized | Update another user's review | Status 403, access denied | ✅ |
| Update validation | Update with invalid data | Status 400, validation errors | ✅ |
| Delete review - owner | User deletes own review | Status 200, review deleted | ✅ |
| Delete review - admin | Admin deletes any review | Status 200, review deleted | ✅ |
| Delete - unauthorized | Delete another user's review | Status 403, access denied | ✅ |

### Authorization Middleware Tests (auth.middleware.test.js)

| Test Case | Description | Expected Result | Status |
|-----------|-------------|-----------------|--------|
| No token | Request without token | Status 401, authentication error | ✅ |
| Invalid token | Request with invalid token string | Status 401, invalid token error | ✅ |
| Expired token | Request with expired JWT | Status 401, token expired error | ✅ |
| Non-existent user | Token with invalid user ID | Status 401, invalid token error | ✅ |
| Admin access | Admin accesses restricted route | Status 200, access granted | ✅ |
| Non-admin access | Regular user accesses admin route | Status 403, access denied | ✅ |
| Default pagination | No pagination parameters | Default page=1, limit=10 applied | ✅ |
| Custom pagination | Specific page and limit | Custom pagination parameters applied | ✅ |
| Default sorting | No sort parameter | Default sort by createdAt DESC applied | ✅ |
| Custom sorting | Specific sort parameter | Custom sorting applied | ✅ |

## Edge Cases Tested

1. **Authentication**:
    - Registration with duplicate email/username
    - Login with non-existent user
    - Using expired tokens
    - Using malformed tokens

2. **Authorization**:
    - Regular users attempting to access admin routes
    - Users attempting to modify resources they don't own
    - Requests without authentication tokens

3. **Validation**:
    - Empty fields
    - Invalid email formats
    - Short passwords
    - Negative prices and stock values
    - Out-of-range review ratings

4. **Resource Access**:
    - Accessing non-existent resources
    - Attempting to create duplicate reviews

## Test Results Summary

All 65+ test cases have been executed successfully, covering:

- **Authentication and Authorization**: All security measures work correctly, with proper validation of tokens and permissions.
- **User Operations**: User registration, login, profile management, and deletion work as expected.
- **Product Management**: Creating, retrieving, updating, and deleting products function correctly with proper ownership checks.
- **Review System**: Users can review products, update their reviews, and admins can moderate reviews.
- **Pagination and Filtering**: The API correctly handles pagination, sorting, and filtering parameters.
- **Edge Cases**: The API gracefully handles all tested edge cases and error scenarios.

## Running the Tests

To run the complete test suite:

```bash
npm test