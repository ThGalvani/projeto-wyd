import { create } from 'zustand'
import api from '@/lib/api'
import type { User } from '@/types'

interface AuthState {
  user: User | null
  token: string | null
  isAuthenticated: boolean
  isLoading: boolean
  error: string | null

  login: (email: string, password: string) => Promise<void>
  register: (name: string, email: string, password: string) => Promise<void>
  logout: () => void
  fetchMe: () => Promise<void>
  updateProfile: (data: { name?: string; profileImage?: string }) => Promise<void>
  clearError: () => void
}

export const useAuthStore = create<AuthState>((set) => ({
  user: null,
  token: localStorage.getItem('token'),
  isAuthenticated: !!localStorage.getItem('token'),
  isLoading: false,
  error: null,

  login: async (email: string, password: string) => {
    try {
      set({ isLoading: true, error: null })
      const response = await api.post('/auth/login', { email, password })
      const { user, token } = response.data

      localStorage.setItem('token', token)
      set({
        user,
        token,
        isAuthenticated: true,
        isLoading: false,
      })
    } catch (error: any) {
      set({
        error: error.response?.data?.error || 'Login failed',
        isLoading: false,
      })
      throw error
    }
  },

  register: async (name: string, email: string, password: string) => {
    try {
      set({ isLoading: true, error: null })
      const response = await api.post('/auth/register', { name, email, password })
      const { user, token } = response.data

      localStorage.setItem('token', token)
      set({
        user,
        token,
        isAuthenticated: true,
        isLoading: false,
      })
    } catch (error: any) {
      set({
        error: error.response?.data?.error || 'Registration failed',
        isLoading: false,
      })
      throw error
    }
  },

  logout: () => {
    localStorage.removeItem('token')
    set({
      user: null,
      token: null,
      isAuthenticated: false,
    })
  },

  fetchMe: async () => {
    try {
      set({ isLoading: true, error: null })
      const response = await api.get('/auth/me')
      set({
        user: response.data,
        isLoading: false,
      })
    } catch (error: any) {
      set({
        error: error.response?.data?.error || 'Failed to fetch user data',
        isLoading: false,
        isAuthenticated: false,
        token: null,
        user: null,
      })
      localStorage.removeItem('token')
    }
  },

  updateProfile: async (data: { name?: string; profileImage?: string }) => {
    try {
      set({ isLoading: true, error: null })
      const response = await api.put('/auth/profile', data)
      set({
        user: response.data,
        isLoading: false,
      })
    } catch (error: any) {
      set({
        error: error.response?.data?.error || 'Failed to update profile',
        isLoading: false,
      })
      throw error
    }
  },

  clearError: () => set({ error: null }),
}))
