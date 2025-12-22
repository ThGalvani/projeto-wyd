import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

router.use(authenticate)

router.get('/summary', (req, res) => res.json({ message: 'Get summary report' }))
router.get('/by-category', (req, res) => res.json({ message: 'Get by category report' }))
router.get('/trends', (req, res) => res.json({ message: 'Get trends report' }))
router.get('/export', (req, res) => res.json({ message: 'Export report' }))

export default router
