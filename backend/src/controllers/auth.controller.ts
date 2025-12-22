import { Request, Response } from 'express'
import bcrypt from 'bcrypt'
import { generateToken } from '../utils/jwt'
import { AuthRequest } from '../middleware/auth.middleware'

// Mock data for demo mode
const DEMO_MODE = process.env.DEMO_MODE === 'true' || !process.env.DATABASE_URL

const mockUsers = new Map([
  ['demo@example.com', {
    id: 'demo-user-1',
    name: 'Usuário Demo',
    email: 'demo@example.com',
    password: '$2b$10$demo.hash', // senha: demo123
    profileImage: null,
    partnerId: null,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString(),
  }]
])

// Dynamic import for Prisma (only in non-demo mode)
let prisma: any = null
if (!DEMO_MODE) {
  import('../lib/prisma').then(module => {
    prisma = module.default
  })
}

export const register = async (req: Request, res: Response) => {
  try {
    const { name, email, password } = req.body

    // Validation
    if (!name || !email || !password) {
      return res.status(400).json({ error: 'All fields are required' })
    }

    if (password.length < 8) {
      return res.status(400).json({ error: 'Password must be at least 8 characters' })
    }

    if (DEMO_MODE) {
      // Demo mode - use mock data
      if (mockUsers.has(email)) {
        return res.status(400).json({ error: 'Email already registered' })
      }

      const hashedPassword = await bcrypt.hash(password, 10)
      const newUser = {
        id: `user-${Date.now()}`,
        name,
        email,
        password: hashedPassword,
        profileImage: null,
        partnerId: null,
        createdAt: new Date().toISOString(),
        updatedAt: new Date().toISOString(),
      }

      mockUsers.set(email, newUser)

      const token = generateToken(newUser.id)

      return res.status(201).json({
        user: {
          id: newUser.id,
          name: newUser.name,
          email: newUser.email,
          profileImage: newUser.profileImage,
          partnerId: newUser.partnerId,
          createdAt: newUser.createdAt,
        },
        token,
        demoMode: true,
      })
    }

    // Production mode - use database
    const existingUser = await prisma.user.findUnique({
      where: { email },
    })

    if (existingUser) {
      return res.status(400).json({ error: 'Email already registered' })
    }

    const hashedPassword = await bcrypt.hash(password, 10)

    const user = await prisma.user.create({
      data: {
        name,
        email,
        password: hashedPassword,
      },
      select: {
        id: true,
        name: true,
        email: true,
        profileImage: true,
        partnerId: true,
        createdAt: true,
      },
    })

    const token = generateToken(user.id)

    res.status(201).json({
      user,
      token,
    })
  } catch (error) {
    console.error('Register error:', error)
    res.status(500).json({ error: 'Failed to create account' })
  }
}

export const login = async (req: Request, res: Response) => {
  try {
    const { email, password } = req.body

    // Validation
    if (!email || !password) {
      return res.status(400).json({ error: 'Email and password are required' })
    }

    if (DEMO_MODE) {
      // Demo mode - use mock data
      const user = mockUsers.get(email)

      if (!user) {
        return res.status(401).json({ error: 'Invalid credentials' })
      }

      // For demo, accept "demo123" as password or check hash
      const isPasswordValid = password === 'demo123' || await bcrypt.compare(password, user.password)

      if (!isPasswordValid) {
        return res.status(401).json({ error: 'Invalid credentials' })
      }

      const token = generateToken(user.id)

      return res.json({
        user: {
          id: user.id,
          name: user.name,
          email: user.email,
          profileImage: user.profileImage,
          partnerId: user.partnerId,
        },
        token,
        demoMode: true,
      })
    }

    // Production mode - use database
    const user = await prisma.user.findUnique({
      where: { email },
    })

    if (!user) {
      return res.status(401).json({ error: 'Invalid credentials' })
    }

    const isPasswordValid = await bcrypt.compare(password, user.password)

    if (!isPasswordValid) {
      return res.status(401).json({ error: 'Invalid credentials' })
    }

    const token = generateToken(user.id)

    res.json({
      user: {
        id: user.id,
        name: user.name,
        email: user.email,
        profileImage: user.profileImage,
        partnerId: user.partnerId,
      },
      token,
    })
  } catch (error) {
    console.error('Login error:', error)
    res.status(500).json({ error: 'Failed to login' })
  }
}

export const getMe = async (req: AuthRequest, res: Response) => {
  try {
    if (DEMO_MODE) {
      // Demo mode - find user in mock data
      const user = Array.from(mockUsers.values()).find(u => u.id === req.userId)

      if (!user) {
        return res.status(404).json({ error: 'User not found' })
      }

      return res.json({
        id: user.id,
        name: user.name,
        email: user.email,
        profileImage: user.profileImage,
        partnerId: user.partnerId,
        partner: null,
        createdAt: user.createdAt,
        updatedAt: user.updatedAt,
        demoMode: true,
      })
    }

    // Production mode - use database
    const user = await prisma.user.findUnique({
      where: { id: req.userId },
      select: {
        id: true,
        name: true,
        email: true,
        profileImage: true,
        partnerId: true,
        partner: {
          select: {
            id: true,
            name: true,
            email: true,
            profileImage: true,
          },
        },
        createdAt: true,
        updatedAt: true,
      },
    })

    if (!user) {
      return res.status(404).json({ error: 'User not found' })
    }

    res.json(user)
  } catch (error) {
    console.error('Get me error:', error)
    res.status(500).json({ error: 'Failed to get user data' })
  }
}

export const updateProfile = async (req: AuthRequest, res: Response) => {
  try {
    const { name, profileImage } = req.body

    if (DEMO_MODE) {
      // Demo mode - update mock data
      const user = Array.from(mockUsers.values()).find(u => u.id === req.userId)

      if (!user) {
        return res.status(404).json({ error: 'User not found' })
      }

      if (name) user.name = name
      if (profileImage !== undefined) user.profileImage = profileImage
      user.updatedAt = new Date().toISOString()

      return res.json({
        id: user.id,
        name: user.name,
        email: user.email,
        profileImage: user.profileImage,
        partnerId: user.partnerId,
        updatedAt: user.updatedAt,
        demoMode: true,
      })
    }

    // Production mode - use database
    const user = await prisma.user.update({
      where: { id: req.userId },
      data: {
        ...(name && { name }),
        ...(profileImage !== undefined && { profileImage }),
      },
      select: {
        id: true,
        name: true,
        email: true,
        profileImage: true,
        partnerId: true,
        updatedAt: true,
      },
    })

    res.json(user)
  } catch (error) {
    console.error('Update profile error:', error)
    res.status(500).json({ error: 'Failed to update profile' })
  }
}
