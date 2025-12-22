import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

// All transaction routes require authentication
router.use(authenticate)

// Placeholder routes - will be implemented later
router.get('/', (req, res) => res.json({ message: 'Get transactions' }))
router.get('/:id', (req, res) => res.json({ message: 'Get transaction by ID' }))
router.post('/', (req, res) => res.json({ message: 'Create transaction' }))
router.put('/:id', (req, res) => res.json({ message: 'Update transaction' }))
router.delete('/:id', (req, res) => res.json({ message: 'Delete transaction' }))
router.get('/stats/summary', (req, res) => res.json({ message: 'Get transaction stats' }))

export default router
