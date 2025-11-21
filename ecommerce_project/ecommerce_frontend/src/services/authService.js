// src/services/authService.js
import api from './api.js';
import { saveToken, saveUser, removeToken } from '../utils/authUtils';

export const authService = {
  async login(email, password) {
    try {
      const response = await api.post('/auth/login', { email, password });
      const { token, user } = response.data;
      
      saveToken(token);
      saveUser(user);
      
      return response.data;
    } catch (error) {
      throw error.response?.data || error;
    }
  },

  async register(username, email, password) {
    try {
      const response = await api.post('/auth/register', {
        username,
        email,
        password
      });
      
      const { token, user } = response.data;
      
      saveToken(token);
      saveUser(user);
      
      return response.data;
    } catch (error) {
      throw error.response?.data || error;
    }
  },

  logout() {
    removeToken();
  },

  async getCurrentUser() {
    try {
      const response = await api.get('/auth/me');
      return response.data;
    } catch (error) {
      throw error.response?.data || error;
    }
  }
};