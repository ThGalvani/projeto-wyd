import { Router } from 'express'
import { authenticate } from '../middleware/auth.middleware'

const router = Router()

router.use(authenticate)

router.get('/', (req, res) => res.json({ message: 'Get goals' }))
router.post('/', (req, res) => res.json({ message: 'Create goal' }))
router.put('/:id', (req, res) => res.json({ message: 'Update goal' }))
router.delete('/:id', (req, res) => res.json({ message: 'Delete goal' }))
router.post('/:id/contribute', (req, res) => res.json({ message: 'Contribute to goal' }))

export default router
