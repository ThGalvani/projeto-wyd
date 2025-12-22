import express from 'express'
import cors from 'cors'
import dotenv from 'dotenv'
import authRoutes from './routes/auth.routes'
import transactionRoutes from './routes/transaction.routes'
import categoryRoutes from './routes/category.routes'
import subscriptionRoutes from './routes/subscription.routes'
import goalRoutes from './routes/goal.routes'
import reportRoutes from './routes/report.routes'
import partnerRoutes from './routes/partner.routes'

// Load environment variables
dotenv.config()

const app = express()
const PORT = process.env.PORT || 3001

// Check if running in demo mode (no database)
const DEMO_MODE = process.env.DEMO_MODE === 'true' || !process.env.DATABASE_URL

if (DEMO_MODE) {
  console.log('⚠️  Running in DEMO MODE - No database connected')
}

// Middleware
app.use(cors({
  origin: process.env.CORS_ORIGIN || 'http://localhost:5173',
  credentials: true,
}))
app.use(express.json())
app.use(express.urlencoded({ extended: true }))

// Health check
app.get('/health', (req, res) => {
  res.json({
    status: 'ok',
    timestamp: new Date().toISOString(),
    mode: DEMO_MODE ? 'demo' : 'production',
    database: DEMO_MODE ? 'mock' : 'connected'
  })
})

// Demo mode indicator
app.get('/api/status', (req, res) => {
  res.json({
    demoMode: DEMO_MODE,
    message: DEMO_MODE
      ? 'Running in demo mode with mock data. Configure DATABASE_URL to use real database.'
      : 'Connected to database',
  })
})

// Routes
app.use('/api/auth', authRoutes)
app.use('/api/transactions', transactionRoutes)
app.use('/api/categories', categoryRoutes)
app.use('/api/subscriptions', subscriptionRoutes)
app.use('/api/goals', goalRoutes)
app.use('/api/reports', reportRoutes)
app.use('/api/partner', partnerRoutes)

// Error handling middleware
app.use((err: any, req: express.Request, res: express.Response, next: express.NextFunction) => {
  console.error('Error:', err)
  res.status(err.status || 500).json({
    error: err.message || 'Internal Server Error',
    ...(process.env.NODE_ENV === 'development' && { stack: err.stack }),
  })
})

// 404 handler
app.use((req, res) => {
  res.status(404).json({ error: 'Route not found' })
})

// For Vercel serverless
if (process.env.VERCEL) {
  module.exports = app
} else {
  app.listen(PORT, () => {
    console.log(`🚀 Server running on http://localhost:${PORT}`)
    console.log(`📊 Environment: ${process.env.NODE_ENV || 'development'}`)
    if (DEMO_MODE) {
      console.log(`⚠️  Demo Mode: Using mock data`)
    }
  })
}

export default app
