import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

router.use(authenticate)

router.get('/', (req, res) => res.json({ message: 'Get subscriptions' }))
router.post('/', (req, res) => res.json({ message: 'Create subscription' }))
router.put('/:id', (req, res) => res.json({ message: 'Update subscription' }))
router.delete('/:id', (req, res) => res.json({ message: 'Delete subscription' }))
router.get('/summary', (req, res) => res.json({ message: 'Get subscription summary' }))

export default router
